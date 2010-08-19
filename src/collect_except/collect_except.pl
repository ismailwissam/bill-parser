############################################################################
# collect_except.pl                                                         
#
# 特殊网元采集程序                                                          
#
# create by wangxiaohui at 2010.7
############################################################################
use strict;
use Time::Local;
use Net::FTP;
use Getopt::Long;
use FileHandle;
use POSIX ":sys_wait_h";
use POSIX qw(strftime);

# 定义全局常量
my $USAGE =<<"EOF";
Usage:$0 -c <commit_path> -r <run_path> -p <process_number> -m <adjust_minute>
Options:
       -c file commit path, default is ./data
       -r run path, default is ./
       -p max collect process number, default is 1
       -m adjust minute, default is 0
EOF
my $LOG_DIR = './log';
my $WORK_DIR = './work';
my $CONFIG_DIR = './conf';
my $ERR_LOG_FILE = './log/collect_except_err';
my $PREFIX_RUN_LOG_FILE = './log/collect_except_run';
my $MAX_CHILD_PROCESS = 128;
my $SLEEP_TIME = 60;

use constant
{
    FALSE => 0,
    TRUE =>1
};

# 定义全局变量
my $g_ChildProcessStatus;
my $g_ChildProcessNum = 1;
my $g_CollectConfig;
my $g_CollectPointNum = 0;
my $g_CollectStartTime;
my $g_CommitPath = './data/collect';
my $g_RunPath = './';
my $g_CurrentProcessNo = 0;
my $g_TermFlag = 0;

# 注册信号处理函数
$SIG{SIGINT} = sub { $g_TermFlag = 1; }

main();

sub main
{
    # 读取配置参数
    my %opts;
    my $adjust_minute = 0;
    if(!GetOptions(\%opts, "c=s", "r=s", "p=s", "m=s"))
    {
        err_log("main: error parameters!\n");
        die "error parameters!\n$USAGE\n";
    }
    $g_CommitPath = $opts{"c"};
    $g_RunPath = $opts{"r"};
    $g_ChildProcessNum = $opts{"p"};
    $adjust_minute = $opts{"m"};

    # 验证输入参数
    if((!defined $g_RunPath) || (! -d $g_RunPath))
    {
        err_log("main: invalid run dir\n");
        die "invalid run dir\n$USAGE\n";
    }

    if(!chdir($g_RunPath))
    {
        err_log("main: chdir to $g_RunPath fail\n");
        die "chdir to $g_RunPath fail\n";
    }

    if((!defined $g_CommitPath) || (! -d $g_CommitPath))
    {
        err_log("main: invalid commit dir\n");
        die "invalid commit dir\n$USAGE\n";
    }

    # 检查Work目录
    if((!defined $WORK_DIR) || (! -d $WORK_DIR))
    {
        err_log("main: invalid work dir: $WORK_DIR\n");
        die "invalid work dir: $WORK_DIR\n";
    }

    # 检查LOG目录
    if((!defined $LOG_DIR) || (! -d $LOG_DIR))
    {
        err_log("main: invalid log dir: $LOG_DIR\n");
        die "invalid log dir: $LOG_DIR\n";
    }

    # 检查配置目录
    if((!defined $CONFIG_DIR) || (! -d $CONFIG_DIR))
    {
        err_log("main: invalid conf dir: $CONFIG_DIR\n");
        die "invalid conf dir: $CONFIG_DIR\n";
    }

    # 加载采集配置
    if(!load_collect_config())
    {
        err_log("main: verify collect conf fail\n");
        die "verify collect conf fail\n";
    }

    if($g_CollectPointNum <= 0)
    {
        err_log("main: collect config incorrect\n");
        die "collect config incorrect\n";
    }

    if($g_ChildProcessNum < 0) 
    {
        $g_ChildProcessNum = 1;
    }
    if($g_ChildProcessNum > $MAX_CHILD_PROCESS) 
    {
        $g_ChildProcessNum = $MAX_CHILD_PROCESS;
    }
    if($g_ChildProcessNum > $g_CollectPointNum) 
    {
        $g_ChildProcessNum = $g_CollectPointNum;
    }

    # 清理WORK目录下临时文件
    unlink glob("$WORK_DIR/*");

    # 获取采集开始时间
    $g_CollectStartTime = get_collect_time($adjust_minute);

    # 初始化采集进程组
    for(my $i = 0; $i < $g_ChildProcessNum; $i++)
    {
        $g_ChildProcessStatus->[$i]->{'pid'} = 0;
        $g_ChildProcessStatus->[$i]->{'sleep_time'} = 0;
    }

    # 进入守护状态
    daemon_start();

    # 开启采集进程
    while(1)
    {
        # 创建子进程
        for(my $i = 0; $i < $g_ChildProcessNum; $i++)
        {
            if( ($g_ChildProcessStatus->[$i]->{'pid'} == 0) && ($g_ChildProcessStatus->[$i]->{'sleep_time'} <= 0) )
            {
                my $child_pid = fork();
                if($child_pid < 0)
                {
                    err_log("main: fork fail\n");
                    exit(1);
                }
                elsif($child_pid > 0)
                {
                    $g_ChildProcessStatus->[$i]->{'pid'} = $child_pid;
                    $g_ChildProcessStatus->[$i]->{'sleep_time'} = $SLEEP_TIME;
                }
                elsif($child_pid == 0)
                {
                    process_collect($i + 1, $g_ChildProcessNum);
                    exit(0);
                }
            }
        }

        # 回收子进程
        for(my $i = 0; $i < $g_ChildProcessNum; $i++)
        {
            if($g_ChildProcessStatus->[$i]->{'pid'} > 0)
            {
                my $ret_pid = waitpid($g_ChildProcessStatus->[$i]->{'pid'}, WNOHANG);
                if($ret_pid < 0)
                {
                    err_log("main: waitpid fail\n");
                    exit(1);
                }
                elsif($ret_pid > 0)
                {
                    $g_ChildProcessStatus->[$i]->{'pid'} = 0;
                }
            }
        }

        # 如果设置了终止标志，则退出处理循环
        if($g_TermFlag)
        {
            # 退出之前杀死所有的采集进程
            for(my $i = 0; $i < $g_ChildProcessNum; $i++)
            {
                if($g_ChildProcessStatus->[$i]->{'pid'} > 0)
                {
                    kill 2, $g_ChildProcessStatus->[$i]->{'pid'};   #send SIGINT to child
                }
            }
            last;
        }

        sleep 1;

        # 每秒递减已释放进程的sleep time
        for(my $i = 0; $i < $g_ChildProcessNum; $i++)
        {
            if($g_ChildProcessStatus->[$i]->{'pid'} == 0 && $g_ChildProcessStatus->[$i]->{'sleep_time'} > 0)
            {
                $g_ChildProcessStatus->[$i]->{'sleep_time'}--;
            }
        }
    }
}

sub process_collect
{
    my ($current_process_num, $max_process_num) = @_;
    my $factor;
    my $collect_point_no;

    $g_CurrentProcessNo = $current_process_num;

    $factor = 0;
    while(1)
    {
        $collect_point_no = $factor * $max_process_num + $current_process_num - 1;
        last if($collect_point_no >= $g_CollectPointNum);
        if(!point_collect($collect_point_no, $current_process_num))
        {
            last;
        }
        $factor++;
    }
}

# 
# 单个采集点采集任务
#
sub point_collect
{
    my ($collect_point_no, $current_process_no) = @_;
    my ($collect_point,$vendor,$device,$ip,$port,$user,$passwd,$backup_path,$is_backup) = (
        $g_CollectConfig->[$collect_point_no]->{'collect_point'},
        $g_CollectConfig->[$collect_point_no]->{'vendor'},
        $g_CollectConfig->[$collect_point_no]->{'device'},
        $g_CollectConfig->[$collect_point_no]->{'ip'},
        $g_CollectConfig->[$collect_point_no]->{'port'},
        $g_CollectConfig->[$collect_point_no]->{'user'},
        $g_CollectConfig->[$collect_point_no]->{'passwd'},
        $g_CollectConfig->[$collect_point_no]->{'backup_path'},
        $g_CollectConfig->[$collect_point_no]->{'is_backup'});

    # 获取本采集点的采集开始时间
    my $time_point_file = "$CONFIG_DIR/$device\[$ip\]_TimePoint";
    my ($time_point, $tmp_time_point, $tmp_time_point_date);
    my $fh_TimePoint = new FileHandle("$time_point_file", "r");
    if(!defined $fh_TimePoint)
    {
        $time_point = $g_CollectStartTime;
    }
    elsif(!defined ($time_point = <$fh_TimePoint>))
    {
        $time_point = $g_CollectStartTime;
    }
    elsif($time_point lt $g_CollectStartTime)
    {
        $time_point = $g_CollectStartTime;
    }
    $tmp_time_point = $time_point;
    $tmp_time_point_date = substr($time_point, 0, 8);
    if(defined $fh_TimePoint)
    {
        $fh_TimePoint->close();
    }

    # 连接FTP
    my $ftp = connect_ftp($ip, $port, $user, $passwd, undef, 30);
    if(!defined $ftp)
    {
        err_log("point_collect: ftp connect $ip fail\n");
        return FALSE;
    }

    # 获取目录列表
    my @dir_list = $ftp->dir();
    foreach my $dir_line (@dir_list)
    {
        my @dir_line_elems = split(/\s+/, $dir_line);

        if(@dir_line_elems == 4)
        {
            if($dir_line_elems[2] !~ /^<DIR>$/)
            {
                next;
            }

            if(length($dir_line_elems[3]) != 8)
            {
                next;
            }

            if($dir_line_elems[3] lt $tmp_time_point_date)
            {
                next;
            }

            my $cur_dir = $dir_line_elems[3];

            # 进入目录
            if(!$ftp->cwd($cur_dir))
            {
                next;
            }

            my @file_list = $ftp->dir("*.dat");
            foreach my $file_line (@file_list)
            {
                my @file_line_elems = split(/\s+/, $file_line);
                if(@file_line_elems == 4)
                {
                    my $file_time_stamp = gen_time_stamp_4($file_line_elems[0], $file_line_elems[1]);
                    if($file_time_stamp lt $tmp_time_point)
                    {
                        next;
                    }

                    my $file_size = $file_line_elems[2];
                    my $cur_file = $file_line_elems[3];

                    # 下载文件
                    if(!get_remote_file($ftp, $collect_point_no, $cur_dir, $cur_file))
                    {
                        err_log("point_collect: get remote file $cur_file fail\n");
                        return FALSE;
                    }  

                    # 备份文件
                    if(!backup_file($collect_point_no, $cur_file, $file_time_stamp))
                    {
                        err_log("point_collect: backup file $cur_file fail\n");
                        return FALSE;
                    }

                    # 提交文件
                    if(!commit_file($collect_point_no, $cur_dir, $cur_file, $file_size, $file_time_stamp))
                    {
                        err_log("point_collect: commit file $cur_file fail\n");
                        return FALSE;
                    }

                    # 更新时间戳
                    if($tmp_time_point lt $file_time_stamp)
                    {
                        $tmp_time_point = $file_time_stamp;
                    }
                }
                elsif(@file_line_elems == 9)
                {
                    my $file_time_stamp = gen_time_stamp_9($cur_dir, $file_line_elems[7]);
                    if($file_time_stamp lt $tmp_time_point)
                    {
                        next;
                    }

                    my $file_size = $file_line_elems[4];
                    my $cur_file = $file_line_elems[8];

                    # 下载文件
                    if(!get_remote_file($ftp, $collect_point_no, $cur_dir, $cur_file))
                    {
                        err_log("point_collect: get remote file $cur_file fail\n");
                        return FALSE;
                    }  

                    # 备份文件
                    if(!backup_file($collect_point_no, $cur_file, $file_time_stamp))
                    {
                        err_log("point_collect: backup file $cur_file fail\n");
                        return FALSE;
                    }

                    # 提交文件
                    if(!commit_file($collect_point_no, $cur_dir, $cur_file, $file_size, $file_time_stamp))
                    {
                        err_log("point_collect: commit file $cur_file fail\n");
                        return FALSE;
                    }

                    # 更新时间戳
                    if($tmp_time_point lt $file_time_stamp)
                    {
                        $tmp_time_point = $file_time_stamp;
                    }
                }
                else
                {
                    next;
                }
            }
        }
        elsif(@dir_line_elems == 9)
        {
            if($dir_line_elems[0] !~ /^d[rwx-]{9}$/)
            {
                next;
            }

            if(length($dir_line_elems[8]) != 8)
            {
                next;
            }

            if($dir_line_elems[8] lt $tmp_time_point_date)
            {
                next;
            }

            my $cur_dir = $dir_line_elems[8];

            # 进入目录
            if(!$ftp->cwd($cur_dir))
            {
                err_log("point_collect: cwd $cur_dir fail\n");
                next;
            }

            my @file_list = $ftp->dir("*.dat");

            foreach my $file_line (@file_list)
            {
                my @file_line_elems = split(/\s+/, $file_line);

                if(@file_line_elems == 4)
                {
                    my $file_time_stamp = gen_time_stamp_4($file_line_elems[0], $file_line_elems[1]);
                    if($file_time_stamp lt $tmp_time_point)
                    {
                        next;
                    }

                    my $file_size = $file_line_elems[2];
                    my $cur_file = $file_line_elems[3];

                    # 下载文件
                    if(!get_remote_file($ftp, $collect_point_no, $cur_dir, $cur_file))
                    {
                        err_log("point_collect: get remote file $cur_file fail\n");
                        return FALSE;
                    }  

                    # 备份文件
                    if(!backup_file($collect_point_no, $cur_file, $file_time_stamp))
                    {
                        err_log("point_collect: backup file $cur_file fail\n");
                        return FALSE;
                    }

                    # 提交文件
                    if(!commit_file($collect_point_no, $cur_dir, $cur_file, $file_size, $file_time_stamp))
                    {
                        err_log("point_collect: commit file $cur_file fail\n");
                        return FALSE;
                    }

                    # 更新时间戳
                    if($tmp_time_point lt $file_time_stamp)
                    {
                        $tmp_time_point = $file_time_stamp;
                    }
                }
                elsif(@file_line_elems == 9)
                {
                    my $file_time_stamp = gen_time_stamp_9($cur_dir, $file_line_elems[7]);
                    if($file_time_stamp lt $tmp_time_point)
                    {
                        next;
                    }

                    my $file_size = $file_line_elems[4];
                    my $cur_file = $file_line_elems[8];

                    # 下载文件
                    if(!get_remote_file($ftp, $collect_point_no, $cur_dir, $cur_file))
                    {
                        err_log("point_collect: get remote file $cur_file fail\n");
                        return FALSE;
                    }  

                    # 备份文件
                    if(!backup_file($collect_point_no, $cur_file, $file_time_stamp))
                    {
                        err_log("point_collect: backup file $cur_file fail\n");
                        return FALSE;
                    }

                    # 提交文件
                    if(!commit_file($collect_point_no, $cur_dir, $cur_file, $file_size, $file_time_stamp))
                    {
                        err_log("point_collect: commit file $cur_file fail\n");
                        return FALSE;
                    }

                    # 更新时间戳
                    if($tmp_time_point lt $file_time_stamp)
                    {
                        $tmp_time_point = $file_time_stamp;
                    }
                }
                else
                {
                    next;
                }
            }
        }
        else
        {
            err_log("point_collect: ftp server files saving changed\n");
            next;
        }
    }

    # 断开FTP
    $ftp->quit;


    # 清理工作
    if($time_point lt $tmp_time_point)
    {
        $fh_TimePoint = new FileHandle("$time_point_file", "w");
        if(!defined $fh_TimePoint)
        {
            err_log("point_collect: update time point file $time_point_file fail\n");
            return FALSE;
        }
        print $fh_TimePoint "$tmp_time_point";
        $fh_TimePoint->close();
    }

    # 清理WORK目录下的临时文件
    my $cur_collect_point = sprintf("%06d", $collect_point);
    unlink glob("$WORK_DIR/$cur_collect_point*");

    return TRUE;
}

sub connect_ftp
{
    my $Host = shift;
    my $Port = shift;
    my $User = shift;
    my $Passwd = shift;
    my $Path = shift;
    my $Timeout = shift;
    
    # 链接FTP
    my $ftp = Net::FTP->new($Host, Port => $Port, Timeout =>$Timeout ,Passive => 1);
    if(!$ftp)
    {
        err_log("connect_ftp: ftp connect $Port fail\n");
        return undef;
    }

    if(!$ftp->login($User, $Passwd))
    {
        err_log("connect_ftp: ftp login fail\n");
        return undef;
    }

    if(defined($Path) and !$ftp->cwd($Path))
    {
        err_log("connect_ftp: ftp cd $Path fail\n");
        return undef;
    }

    if(!$ftp->binary())
    {
        err_log("connect_ftp: ftp binary fail\n");
        return undef;
    }

    return $ftp;
}

#
# 加载话单采集配置
#
sub load_collect_config
{
    my $config_file = "$CONFIG_DIR/collect_except.conf";
    my $fh_Config = new FileHandle("$config_file", "r");
    if(!$fh_Config)
    {
        return FALSE;
    }

    my $count = 0;
    while(my $line = <$fh_Config>)
    {
        chomp $line;
        my @elems = split /,/, $line;
        if(@elems != 9)
        {
            err_log("load_collect_config: collect config error!\n");
            return FALSE;
        }
        
        $g_CollectConfig->[$count]->{'collect_point'} = $elems[0];
        $g_CollectConfig->[$count]->{'vendor'} = $elems[1];
        $g_CollectConfig->[$count]->{'device'} = $elems[2];
        $g_CollectConfig->[$count]->{'ip'} = $elems[3];
        $g_CollectConfig->[$count]->{'port'} = $elems[4];
        $g_CollectConfig->[$count]->{'user'} = $elems[5];
        $g_CollectConfig->[$count]->{'passwd'} = $elems[6];
        $g_CollectConfig->[$count]->{'backup_path'} = $elems[7];
        $g_CollectConfig->[$count]->{'is_backup'} = $elems[8];

        $count++;
    }
    $fh_Config->close;

    $g_CollectPointNum = $count;

    return TRUE;
}

#
# 获取当前日期时间
#
sub get_time
{
    my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime;
    return sprintf("%04d%02d%02d%02d%02d%02d", $year + 1900, $mon + 1, $mday, $hour, $min, $sec);
}

#
# 获得话单采集开始时间
#
sub get_collect_time
{
    my $adjust_minute = shift;
    my $now_time = time;
    my $start_time = $now_time + $adjust_minute * 60;
    my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime $start_time;
    return sprintf("%04d%02d%02d%02d%02d%02d", $year + 1900, $mon + 1, $mday, $hour, $min, $sec);
}

# 
# 开启守护进程
#
sub daemon_start
{
    umask 022;

    return if(getppid() == 1);

    $SIG{TTOU} = 'IGNORE';
    $SIG{TTIN} = 'IGNORE';
    $SIG{TSTP} = 'IGNORE';
    $SIG{USR1} = 'IGNORE';

    setpgrp;

    $SIG{HUP} = 'IGNORE';

    my $child_pid = fork;
    if($child_pid < 0)
    {
        err_log("daemon_start: fork fail\n");
        exit 1;
    }
    elsif($child_pid > 0)
    {
        exit 0;
    }
}

#
# 记录错误日志
#
sub err_log
{
    my $err_msg = shift;

    my $err_log_file;
    if($g_CurrentProcessNo != 0)
    {
        $err_log_file = sprintf("%s.%03d", $ERR_LOG_FILE, $g_CurrentProcessNo);
    }
    else
    {
        $err_log_file = $ERR_LOG_FILE;
    }

    open ERR_LOG, ">>$err_log_file";
    print ERR_LOG "------------------------------------------------------------------\n";
    print ERR_LOG (strftime "%a %b %e %H:%M:%S %Y", localtime);
    print ERR_LOG $err_msg;
    close ERR_LOG;
}

#
# 下载远程文件
# 
sub get_remote_file
{
    my ($ftp, $collect_point_no, $remote_path, $remote_file) = @_;
    my $local_file = sprintf("%s/%06d_%s", $WORK_DIR, $g_CollectConfig->[$collect_point_no]->{'collect_point'}, $remote_file);
    if(-e $local_file)
    {
        unlink $local_file;
    }

    my $ret_status;
    eval
    {
        $ret_status = $ftp->get($remote_file, $local_file);
    };

    if(!$ret_status or ! -e $local_file)
    {
        err_log("get_remote_file: get file $remote_file fail\n");
        return FALSE;
    }

    return TRUE;
}

sub backup_file
{
    my ($collect_point_no, $remote_file, $file_stamp) = @_;
    if($g_CollectConfig->[$collect_point_no]->{'is_backup'} eq 'yes')
    {
        my $tmp_file = sprintf("%s/%06d_%s", $WORK_DIR, $g_CollectConfig->[$collect_point_no]->{'collect_point'}, $remote_file);
        my $backup_dir = sprintf("%s/%s/%s",
            $g_CollectConfig->[$collect_point_no]->{'backup_path'},
            $g_CollectConfig->[$collect_point_no]->{'vendor'},
            $g_CollectConfig->[$collect_point_no]->{'device'});
        my $backup_file = sprintf("%s/%06d_%s_%s_%s_%s",
            $backup_dir,
            $g_CollectConfig->[$collect_point_no]->{'collect_point'},
            $g_CollectConfig->[$collect_point_no]->{'vendor'},
            $g_CollectConfig->[$collect_point_no]->{'device'},
            $file_stamp,
            $remote_file);            

        if(! -d $backup_dir)
        {
            if(!mkdir($backup_dir, 0755))
            {
                err_log("backup_file: mkdir $backup_dir fail\n");
                return FALSE;
            }
        }

        if(system("cp $tmp_file $backup_file") != 0)
        {
            err_log("backup_file: backup file $tmp_file fail\n");
            return FALSE;
        }
    }

    return TRUE;
}

sub commit_file
{
    my ($collect_point_no, $remote_dir, $remote_file, $file_size, $file_stamp) = @_;
    my $tmp_file = sprintf("%s/%06d_%s", $WORK_DIR, $g_CollectConfig->[$collect_point_no]->{'collect_point'}, $remote_file);
    my $commit_file = sprintf("%s/@%06d_%s_%s_%s_%s", $g_CommitPath,
            $g_CollectConfig->[$collect_point_no]->{'collect_point'},
            $g_CollectConfig->[$collect_point_no]->{'vendor'},
            $g_CollectConfig->[$collect_point_no]->{'device'},
            $file_stamp,
            $remote_file);  
   if(! -e $commit_file)
   {
       if($remote_file =~ /^gz.+/)
       {
           if(system("gzip -S .dat -d $tmp_file") != 0) {
               err_log("commit_file: uncompress file $tmp_file fail\n");
               return FALSE;
           }
           $tmp_file =~ /(.+?)\.dat/;
           $tmp_file = $1;
       }
       if(!link($tmp_file, $commit_file))
       {
           err_log("commit_file: commit file $tmp_file fail\n");
           return FALSE;
       }
   }
   else
   {
       err_log("commit_file: target file $commit_file already exist\n");
   }

   # 记录运行日志
   my $log_time = get_time();
   my $commit_time = get_time();
   $log_time = substr($log_time, 0, 8);

   my $log_file = sprintf("%s.%s.%s", $PREFIX_RUN_LOG_FILE,
                         $g_CollectConfig->[$collect_point_no]->{'device'},
                         $log_time);
   open LOG_FILE, ">>$log_file";
   my $commit_msg = sprintf("<%06d> %s %s %s %s %ld %s %s\n", 
                            $g_CollectConfig->[$collect_point_no]->{'collect_point'},
                            $g_CollectConfig->[$collect_point_no]->{'device'},
                            $g_CollectConfig->[$collect_point_no]->{'ip'},
                            $remote_dir,
                            $remote_file,
                            $file_size,
                            $commit_time,
                            $g_CollectConfig->[$collect_point_no]->{'is_backup'});
   print LOG_FILE $commit_msg;
   close LOG_FILE;

   return TRUE;
}

sub gen_time_stamp_4
{
    my ($date_str, $time_str) = @_;
    my ($year, $mon, $day, $hour, $min, $sec);
    my $time_flag;

    if($date_str =~ /(\d{2})-(\d{2})-(\d{2})/)
    {
        $mon = $1;
        $day = $2;
        $year = "20$3";
    }

    if($time_str =~ /(\d{2}):(\d{2})(AM|PM)/)
    {
        $hour = $1;
        $min = $2;
        $time_flag = $3;

        if($time_flag eq 'AM')
        {
            if($hour eq '12')
            {
                $hour -= 12;
            }
        }
        else
        {
            if($hour < 12)
            {
                $hour += 12;
            }
        }
    }

    $sec = '00';

    return sprintf("%04d%02d%02d%02d%02d%02d", $year, $mon, $day, $hour, $min, $sec);
}

sub gen_time_stamp_9
{
    my ($date_str, $time_str) = @_;
    my ($hour, $min, $sec);

    if($time_str =~ /(\d{2}):(\d{2})/)
    {
        $hour = $1;
        $min = $2;
    }

    $sec = '00';

    return sprintf("%s%02d%02d%02d", $date_str, $hour, $min, $sec);
}

