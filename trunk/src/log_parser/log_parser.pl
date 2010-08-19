##############################################################################
# log_parser.pl
#
# ��־��������
#
# create by wangxiaohui at 2010.8
##############################################################################
use strict;
use Time::Local;
use Getopt::Long;
use FileHandle;
use POSIX qw(strftime);
use POSIX ":sys_wait_h";

# ����ȫ�ֳ���
my $USAGE =<<"EOF";
Usage:$0 -r<run_path> -c<collect_log_path> -p<pretreat_log_path> -i<insert_log_path> -d<parse_date>
Options:
       -r run path, default is ./
       -c collect program log path, must be set
       -p pretreat program log path, must be set
       -i insert program log path, must be set
       -d specified date(YYYYMMDD) the log to be parsed
EOF

my $LOG_DIR = "./log";
my $TEMPLATE_DIR = "./template";
my $WORK_DIR = "./work";

my $ERR_LOG_FILE = "$LOG_DIR/log_parser_err";
my $PREFIX_RUN_LOG_FILE = "$LOG_DIR/log_parser_run";

my $DB_SERVER = 'obidb3_132';
my $DB_USER = 'obirawdb';
my $DB_PASSWORD = 'obirawdb\$zj2010';

my $COLLECT_LOG_TABLE = "sys_bill_collect_log";
my $PRETREAT_LOG_TABLE = "sys_bill_pretreat_log";
my $INSERT_LOG_TABLE = "sys_bill_insert_log";
my $LOG_TO_CSV_TABLE = "sys_bill_log_to_csv";

my $COLLECT_LOG_TEMPLATE = "$TEMPLATE_DIR/$COLLECT_LOG_TABLE.template";
my $PRETREAT_LOG_TEMPLATE = "$TEMPLATE_DIR/$PRETREAT_LOG_TABLE.template";
my $INSERT_LOG_TEMPLATE = "$TEMPLATE_DIR/$INSERT_LOG_TABLE.template";
my $LOG_TO_CSV_TEMPLATE = "$TEMPLATE_DIR/$LOG_TO_CSV_TABLE.template";

my $COLLECT_LOG_CSV = "$WORK_DIR/$COLLECT_LOG_TABLE.csv";
my $PRETREAT_LOG_CSV = "$WORK_DIR/$PRETREAT_LOG_TABLE.csv";
my $INSERT_LOG_CSV = "$WORK_DIR/$INSERT_LOG_TABLE.csv";
my $LOG_TO_CSV_CSV = "$WORK_DIR/$LOG_TO_CSV_TABLE.csv";

use constant
{
    FALSE => 0,
    TRUE =>1
};

use constant MAX_TASK_NUM => 4;

# ����ȫ�ֱ���
my $g_RunPath;
my $g_CollectLogPath;
my $g_PretreatLogPath;
my $g_InsertLogPath;
my $g_ParseLogDate;
my $g_CurrentProcessNo = 0;
my $g_TermFlag = 0;

# ע���źŴ�����
$SIG{INT} = sub { $g_TermFlag = 1; };

main();

sub main
{
    # ��ȡ�������
    my %opts;
    if(!GetOptions(\%opts, "r=s", "c=s", "p=s", "i=s", "d=s"))
    {
        err_log("main: error parameters!\n");
        die "error parameters!\n$USAGE\n";
    }
    $g_RunPath = $opts{"r"};
    $g_CollectLogPath = $opts{"c"};
    $g_PretreatLogPath = $opts{"p"};
    $g_InsertLogPath = $opts{"i"};
    $g_ParseLogDate = $opts{"d"};

    # ��֤�������
    if(!defined $g_RunPath)
    {
        $g_RunPath = "./";
    }
    elsif(! -d $g_RunPath)
    {
        err_log("main: invalid run dir\n");
        die "invalid run dir\n$USAGE\n";
    }

    # �л�������Ŀ¼
    if(!chdir($g_RunPath))
    {
        err_log("main: chdir to $g_RunPath fail\n");
        die "chdir to $g_RunPath fail\n";
    }

    # ��֤�ɼ���־���Ŀ¼
    if((!defined $g_CollectLogPath) || (! -d $g_CollectLogPath))
    {
        err_log("main: invalid collect log dir\n");
        die "invalid collect log dir\n$USAGE\n";
    }

    # ��֤������־���Ŀ¼ 
    if((!defined $g_PretreatLogPath) || (! -d $g_PretreatLogPath))
    {
        err_log("main: invalid pretreat log dir\n");
        die "invalid pretreat log dir\n$USAGE\n";
    }

    # ��֤�����־���Ŀ¼
    if((!defined $g_InsertLogPath) || (! -d $g_InsertLogPath))
    {
        err_log("main: invalid insert log dir\n");
        die "invalid insert log dir\n$USAGE\n";
    }

    # ���LOGĿ¼
    if((!defined $LOG_DIR) || (! -d $LOG_DIR))
    {
        err_log("main: invalid log dir: $LOG_DIR\n");
        die "invalid log dir: $LOG_DIR\n";
    }

    # ���WORKĿ¼
    if((!defined $WORK_DIR) || (! -d $WORK_DIR))
    {
        err_log("main: invalid work dir: $WORK_DIR\n");
        die "invalid work dir: $WORK_DIR\n";
    }

    # ������ģ��Ŀ¼
    if((!defined $TEMPLATE_DIR) || (! -d $TEMPLATE_DIR))
    {
        err_log("main: invalid template  dir: $TEMPLATE_DIR\n");
        die "invalid template  dir: $TEMPLATE_DIR\n";
    }

    # ���÷�����־������
    if(!defined $g_ParseLogDate)
    {
        $g_ParseLogDate = get_last_day();
    }

    # ����WORKĿ¼
    unlink glob("$WORK_DIR/*");

    # ���ȷ�������
    my @task_pid;
    for(my $i = 0; $i < MAX_TASK_NUM; $i++)
    {
        $task_pid[$i] = dispatch_parse_task($i + 1);
        if($task_pid[$i] < 0)
        {
            err_log("main: dispatch task fail\n");
            die "dispatch task fail\n";
        }
    }

    # �ȴ���������˳�
    my $wait_pid;
    my $all_task_finish;
    while(TRUE)
    {
        $all_task_finish = TRUE;

        # �ж��������״̬
        for(my $i = 0; $i < MAX_TASK_NUM; $i++)
        {
            if($task_pid[$i] > 0)
            {
                $wait_pid = waitpid($task_pid[$i], WNOHANG);
                if($wait_pid < 0)
                {
                    err_log("main: waitpid fail\n");
                    die "waitpid fail\n";
                }
                elsif($wait_pid > 0)
                {
                    $task_pid[$i] = 0;
                }
                else
                {
                    $all_task_finish = FALSE;
                }
            }
        }

        # �������������̶��ѽ��������˳�������
        if($all_task_finish)
        {
            last;
        }

        # ����ⲿ�������ж��źţ���ɱ�����л��������̣�Ȼ���˳�������
        if($g_TermFlag)
        {
            for(my $i = 0; $i < MAX_TASK_NUM; $i++)
            {
                if($task_pid[$i] > 0)
                {
                    kill 2, $task_pid[$i];  #send SIGINT to task process
                }
            }
            last;
        }

        sleep 60;
    }
}

#
# ���ȷ�������
#
sub dispatch_parse_task
{
    my $process_no = shift;
    $g_CurrentProcessNo = $process_no;

    my $pid = fork();

    if($pid == 0)
    {
        if($g_CurrentProcessNo == 1)
        {
            parse_collect_run_log($g_ParseLogDate);
        }
        elsif($g_CurrentProcessNo == 2)
        {
            parse_pretreat_run_log($g_ParseLogDate);
        }
        elsif($g_CurrentProcessNo == 3)
        {
            parse_insert_run_log($g_ParseLogDate);
        }
        elsif($g_CurrentProcessNo == 4)
        {
            parse_insert_sqlldr_log($g_ParseLogDate);
        }
        exit(0);
    }

    return $pid;
}

# 
# �����ɼ�������־
#
sub parse_collect_run_log
{
    my $parse_date = shift;
    my $collect_log_data;

    print "===>begin collect run log parsing task...\n\n";

    # 
    # ��ȡ����Ҫ�������־�ļ�����ѭ������
    #
    my @log_files = glob "$g_CollectLogPath/collect_run.*.$parse_date";
    foreach my $log_file (@log_files)
    {
        # ����־�ļ�
        my $FH = new FileHandle("$log_file", "r");
        if(!defined $FH)
        {
            err_log("parse_collect_log: file open fail: $log_file\n");
            next;
        }

        # ��ȡ��־����
        my $count = 0;
        while(my $line = <$FH>)
        {
            chomp $line;
            my ($collect_point, 
                $ne_name, 
                $ip,
                $collect_path, 
                $ori_file_name, 
                $cur_file_name,
                $file_size, 
                $file_create_time,
                $collect_time,
                $is_backup,
                $is_parse) = split /\s+/, $line;

            $collect_log_data->[$count]->{'collect_point'} = substr $collect_point, 1, -1;
            $collect_log_data->[$count]->{'ne_name'} = $ne_name;
            $collect_log_data->[$count]->{'collect_path'} = $collect_path;
            $collect_log_data->[$count]->{'ori_file_name'} = $ori_file_name;
            $collect_log_data->[$count]->{'cur_file_name'} = $cur_file_name;
            $collect_log_data->[$count]->{'file_size'} = $file_size;
            $collect_log_data->[$count]->{'file_create_time'} = to_date($file_create_time);
            $collect_log_data->[$count]->{'collect_time'} = to_date($collect_time);
            $collect_log_data->[$count]->{'is_backup'} = $is_backup;
            $collect_log_data->[$count]->{'is_parse'} = $is_parse;
            $collect_log_data->[$count]->{'log_create_time'} = to_date("$parse_date"."000000");

            $count++;
        }

        # �ر���־�ļ�
        $FH->close;

        # д��CSV�ļ�
        if(!write_collect_log_csv($collect_log_data))
        {
            err_log("parse_collect_log: write collect log csv file fail\n");
            return FALSE;
        }

        # �ͷŵ�ǰ����־����
        undef $collect_log_data;
    }

    # ����CSV�ļ����
    if(!load_collect_log_csv())
    {
        err_log("parse_collect_log: load collect log csv fail\n");
        return FALSE;
    }

    print "===>finish collect run log parsing task successfully!\n\n";

    return TRUE;
}

# 
# �����Ĳɼ���־д��CSV�ļ�
#
sub write_collect_log_csv
{
    my $collect_log_data = shift;

    # ��׷�ӷ�ʽ�򿪲ɼ���־CSV�ļ�
    my $fh_csv = new FileHandle($COLLECT_LOG_CSV, "a+");
    if(!defined $fh_csv)
    {
        err_log("write_collect_log_csv: open $COLLECT_LOG_CSV fail\n");
        return FALSE;
    }

    # ������־����
    my ($collect_point, $ne_name, $collect_path, $ori_file_name, $cur_file_name, $file_size, 
        $file_create_time, $collect_time, $is_backup, $is_parse, $log_create_time);
    foreach my $log_entry (@$collect_log_data)
    {
        $collect_point = $log_entry->{'collect_point'};
        $ne_name = $log_entry->{'ne_name'};
        $collect_path = $log_entry->{'collect_path'};
        $ori_file_name = $log_entry->{'ori_file_name'};
        $cur_file_name = $log_entry->{'cur_file_name'};
        $file_size = $log_entry->{'file_size'};
        $file_create_time = $log_entry->{'file_create_time'};
        $collect_time = $log_entry->{'collect_time'};
        $is_backup = $log_entry->{'is_backup'};
        $is_parse = $log_entry->{'is_parse'};
        $log_create_time = $log_entry->{'log_create_time'};

        print $fh_csv "$collect_point,$ne_name,$collect_path,$ori_file_name,$cur_file_name,$file_size,'$file_create_time','$collect_time',$is_backup,$is_parse,'$log_create_time'\n";
    }

    # �ر��ļ�
    $fh_csv->close;

    return TRUE;
}

# 
# ���زɼ���־CSV�ļ����
#
sub load_collect_log_csv
{
    my $load_time = get_time();

    # ����sqlldr����������
    my @results = `sqlldr $DB_USER/$DB_PASSWORD\@$DB_SERVER control=$COLLECT_LOG_TEMPLATE log=$LOG_DIR/$COLLECT_LOG_TABLE\_$load_time\_sqlldr.log bad=$LOG_DIR/$COLLECT_LOG_TABLE\_$load_time\_sqlldr.bad`;

    if(@results == 0)
    {
	    return FALSE;
    }

    return TRUE;
}

# 
# ��������������־
#
sub parse_pretreat_run_log
{
    my $parse_date = shift;
    my $pretreat_log_data;

    print "===>begin pretreat run log parsing task...\n\n";

    # 
    # ��ȡ����Ҫ�������־�ļ�����ѭ������
    #
    my @log_files = glob "$g_PretreatLogPath/pretreat_run.*.$parse_date";
    foreach my $log_file (@log_files)
    {
        # ����־�ļ�
        my $FH = new FileHandle("$log_file", "r");
        if(!defined $FH)
        {
            err_log("parse_pretreat_log: file open fail: $log_file\n");
            next;
        }

        # ��ȡ��־����
        my $count = 0;
        while(my $line = <$FH>)
        {
            chomp $line;
            my ($file_name,
                $file_size,
                $parse_start_time,
                $parse_end_time,
                $parse_bill_num) = split /\s+/, $line;

            my $ne_name = get_ne_name_from_dat_file_name($file_name);

            $pretreat_log_data->[$count]->{'ne_name'} = $ne_name;
            $pretreat_log_data->[$count]->{'file_name'} = $file_name;
            $pretreat_log_data->[$count]->{'file_size'} = $file_size;
            $pretreat_log_data->[$count]->{'parse_start_time'} = to_date($parse_start_time);
            $pretreat_log_data->[$count]->{'parse_end_time'} = to_date($parse_end_time);
            $pretreat_log_data->[$count]->{'parse_bill_num'} = $parse_bill_num;
            $pretreat_log_data->[$count]->{'log_create_time'} = to_date("$parse_date"."000000");

            $count++;
        }

        # �ر���־�ļ�
        $FH->close;

        # ��־�������
        if(!write_pretreat_log_csv($pretreat_log_data))
        {
            err_log("parse_pretreat_log: write pretreat log csv file fail\n");
            return FALSE;
        }

        # �ͷŵ�ǰ����־����
        undef $pretreat_log_data;
    }

    # ����CSV�ļ����
    if(!load_pretreat_log_csv())
    {
        err_log("parse_pretreat_log: load pretreat log csv fail\n");
        return FALSE;
    }

    print "===>finish pretreat run log parsing task successfaully!\n\n";

    return TRUE;
}

# 
# �ӻ����ļ���������ȡ��Ԫ����
# �ļ������ʽ���ɼ�����_������д_��Ԫ����_��������ʱ��_ԭʼ��������
#
sub get_ne_name_from_dat_file_name
{
    my $file_name = shift;
    my @elems = split /_/, $file_name;
    return $elems[2];
}

# 
# ������־д��CSV�ļ� 
#
sub write_pretreat_log_csv
{
    my $pretreat_log_data = shift;

    # �򿪽�����־CSV�ļ�
    my $fh_csv = new FileHandle($PRETREAT_LOG_CSV, "a+");
    if(!defined $fh_csv)
    {
        err_log("write_pretreat_log_csv: open $PRETREAT_LOG_CSV fail\n");
        return FALSE;
    }

    # ������־����
    my ($ne_name, $file_name, $file_size, $parse_start_time,
        $parse_end_time, $parse_bill_num, $log_create_time);
    foreach my $log_entry (@$pretreat_log_data)
    {
        $ne_name = $log_entry->{'ne_name'};
        $file_name = $log_entry->{'file_name'};
        $file_size = $log_entry->{'file_size'};
        $parse_start_time = $log_entry->{'parse_start_time'};
        $parse_end_time = $log_entry->{'parse_end_time'};
        $parse_bill_num = $log_entry->{'parse_bill_num'};
        $log_create_time = $log_entry->{'log_create_time'};

        print $fh_csv "$ne_name,$file_name,$file_size,'$parse_start_time','$parse_end_time',$parse_bill_num, '$log_create_time'\n";
    }

    # �ر��ļ�
    $fh_csv->close;

    return TRUE;
}

# 
# ���ؽ�����־CSV�ļ����
#
sub load_pretreat_log_csv
{
    my $load_time = get_time();

    # ����sqlldr����������
    my @results = `sqlldr $DB_USER/$DB_PASSWORD\@$DB_SERVER control=$PRETREAT_LOG_TEMPLATE log=$LOG_DIR/$PRETREAT_LOG_TABLE\_$load_time\_sqlldr.log bad=$LOG_DIR/$PRETREAT_LOG_TABLE\_$load_time\_sqlldr.bad`;

    if(@results == 0)
    {
	    return FALSE;
    }

    return TRUE;
}

# 
# �������������־
#
sub parse_insert_run_log
{
    my $parse_date = shift;
    my $insert_run_log_data;
 
    print "===>begin the insert run log parsing task...\n\n";

    # 
    # ��ȡ����Ҫ�������־�ļ�����ѭ������
    #
    my @log_files = glob "$g_InsertLogPath/insert_run.*.$parse_date";
    foreach my $log_file (@log_files)
    {
        # ����־�ļ�
        my $FH = new FileHandle("$log_file", "r");
        if(!defined $FH)
        {
            err_log("parse_insert_run_log: file open fail: $log_file\n");
            next;
        }

        # ��ȡ��־����
        my $count = 0;
        while(my $line = <$FH>)
        {
            chomp $line;
            my ($load_csv_name,
                $table_name,
                $log_file_name,
                $load_start_time,
                $load_end_time) = split /\s+/, $line;

            $insert_run_log_data->[$count]->{'load_csv_name'} = $load_csv_name;
            $insert_run_log_data->[$count]->{'log_file_name'} = $log_file_name;

            $count++;
        }

        # �ر���־�ļ�
        $FH->close;

        # ������־д��CSV�ļ�
        if(!write_log_to_csv_csv($insert_run_log_data))
        {
            err_log("parse_insert_run_log: write insert run log csv fail\n");
            return FALSE;
        }

        # �ͷŵ�ǰ����־����
        undef $insert_run_log_data;
    }

    #����������־CSV�ļ����
    if(!load_log_to_csv_csv())
    {
        err_log("parse_insert_run_log: load insert run log csv fail\n");
        return FALSE;
    }

    print "===>finish the insert run log parsing task successfully!\n\n";

    return TRUE;
}

# 
# ����sqlldr������־
#
sub parse_insert_sqlldr_log
{
    my $parse_date = shift;
    my $insert_sqlldr_log_data;

    print "===>begin the insert sqlldr log parsing task...\n\n";

    # 
    # ��ȡ����Ҫ�������־�ļ�����ѭ������
    #
    my @log_files = glob "$g_InsertLogPath/*_$parse_date*.log";
    my $count = 0;
    foreach my $log_file (@log_files)
    {
        my ($table_name, 
            $load_csv_num, 
            $load_csv_list, 
            $load_csv_time, 
            $succ_bill_num, 
            $fail_bill_num) = get_sqlldr_stat_info($log_file);

        my $last_slash = rindex $log_file, "/";
        my $log_file_name;
        if($last_slash >= 0)
        {
            $log_file_name = substr $log_file, $last_slash + 1;
        }
        else
        {
            $log_file_name = $log_file;
        }
        my $ne_name = get_ne_name_from_sqlldr_log_file_name($log_file_name);

        $insert_sqlldr_log_data->[$count]->{'ne_name'} = $ne_name;
        $insert_sqlldr_log_data->[$count]->{'log_file_name'} = $log_file_name;
        $insert_sqlldr_log_data->[$count]->{'table_name'} = $table_name;
        $insert_sqlldr_log_data->[$count]->{'load_csv_num'} = $load_csv_num;
        $insert_sqlldr_log_data->[$count]->{'load_csv_list'} = $load_csv_list;
        $insert_sqlldr_log_data->[$count]->{'load_csv_time'} = to_date($load_csv_time);
        $insert_sqlldr_log_data->[$count]->{'succ_bill_num'} = $succ_bill_num;
        $insert_sqlldr_log_data->[$count]->{'fail_bill_num'} = $fail_bill_num;
        $insert_sqlldr_log_data->[$count]->{'log_create_time'} = to_date("$parse_date"."000000");

        $count++;
    }

    # sqlldr��־д��CSV�ļ�
    if(!write_insert_log_csv($insert_sqlldr_log_data))
    {
        err_log("parse_insert_log: write insert log csv file fail\n");
        return FALSE;
    }

    # ����CSV�ļ����
    if(!load_insert_log_csv())
    {
        err_log("parse_insert_log: load insert log csv fail\n");
        return FALSE;
    }

    # �ͷŵ�ǰ����־����
    undef $insert_sqlldr_log_data;

    print "===>finish the insert sqlldr log parsing task successfully!\n\n";

    return TRUE;
}

# 
# ���ļ���������ȡ��Ԫ����
# �ļ������ʽ1��op_bill_������д_����������д_��Ԫ����_��������(MMDD)_����ʱ��(YYYYMMDDhhmmss)_sqlldr.log
# �ļ������ʽ2��op_bill_������д_��Ԫ����_��������(MMDD)_����ʱ��(YYYYMMDDhhmmss)_sqlldr.log
#
sub get_ne_name_from_sqlldr_log_file_name
{
    my $file_name = shift;
    my @elems = split /_/, $file_name;
    if(@elems == 8)
    {
        return $elems[4];
    }
    elsif(@elems == 7)
    {
        return $elems[3];
    }
}

# 
# ����sqlldr��������־�ļ�����ȡ������Ϣ
#
sub get_sqlldr_stat_info
{
    my $log_file = shift;
    my ($table_name, 
        $load_csv_num, 
        $load_csv_list, 
        $load_csv_time, 
        $succ_bill_num, 
        $fail_bill_num);

    my $last_slash = rindex $log_file, "/";
    my $log_file_name;
    if($last_slash >= 0)
    {
        $log_file_name = substr $log_file, $last_slash + 1;
    }
    else
    {
        $log_file_name = $log_file;
    }

    $table_name = get_table_name_from_sqlldr_log_file_name($log_file_name);
    $load_csv_time = get_load_csv_time_from_sqlldr_log_file_name($log_file_name);

    # ����־�ļ�
    my $FH = new FileHandle("$log_file", "r");
    if(!defined $FH)
    {
        err_log("get_sqlldr_stat_info: file open fail: $log_file\n");
        return undef;
    }

    # ��ȡ��־����
    $load_csv_num = 0;
    $load_csv_list = "";
    $succ_bill_num = 0;
    $fail_bill_num = 0;
    my $is_multi_data_file = FALSE;
    while(my $line = <$FH>)
    {
        chomp $line;

        if($line =~ /^\s*There are (\d+) data files:\s*$/)
        {
            $load_csv_num = $1;
            $is_multi_data_file = TRUE;
            next;
        }

        if($line =~ /^\s*Data Files:\s*(\S+)\s*$/)
        {
            if($load_csv_list eq "")
            {
                $load_csv_list = $1;
            }
            else
            {
                $load_csv_list .= ":$1";
            }
            next;
        }

        if($line =~ /^\s*(\d+)\s+Rows successfully loaded.*$/)
        {
            $succ_bill_num += $1;
            next;
        }

        if($line =~ /^\s*(\d+)\s+Rows not loaded.*$/)
        {
            $fail_bill_num += $1;
            next;
        }
    }
    if(!$is_multi_data_file)
    {
        $load_csv_num = 1;
    }

    # �ر���־�ļ�
    $FH->close;

    return ($table_name, $load_csv_num, $load_csv_list, $load_csv_time, $succ_bill_num, $fail_bill_num);
}

# 
# ��sqlldr��־�ļ���������ȡ��������
# �ļ������ʽ1��op_bill_������д_����������д_��Ԫ����_��������(MMDD)_����ʱ��(YYYYMMDDhhmmss)_sqlldr.log
# �ļ������ʽ2��op_bill_������д_��Ԫ����_��������(MMDD)_����ʱ��(YYYYMMDDhhmmss)_sqlldr.log
#
sub get_table_name_from_sqlldr_log_file_name
{
    my $log_file_name = shift;
    my @elems = split /_/, $log_file_name;
    if(@elems == 8)
    {
        return $elems[0] . "_" . $elems[1] . "_" . $elems[2] . "_" . $elems[3] . "_" . $elems[4] . "_" . $elems[5];
    }
    elsif(@elems == 7)
    {
        return $elems[0] . "_" . $elems[1] . "_" . $elems[2] . "_" . $elems[3] . "_" . $elems[4];
    }
}

# 
# ��sqlldr��־�ļ���������ȡ����ʱ��
# �ļ������ʽ1��op_bill_������д_����������д_��Ԫ����_��������(MMDD)_����ʱ��(YYYYMMDDhhmmss)_sqlldr.log
# �ļ������ʽ2��op_bill_������д_��Ԫ����_��������(MMDD)_����ʱ��(YYYYMMDDhhmmss)_sqlldr.log
#
sub get_load_csv_time_from_sqlldr_log_file_name
{
    my $log_file_name = shift;
    my @elems = split /_/, $log_file_name;
    if(@elems == 8)
    {
        return $elems[6];
    }
    elsif(@elems == 7)
    {
        return $elems[5];
    }
}

# 
# sqlldr��־д��CSV�ļ�
#
sub write_insert_log_csv
{
    my $insert_sqlldr_log_data = shift;

    # ��CSV�ļ�
    my $fh_csv = new FileHandle($INSERT_LOG_CSV, "a+");
    if(!defined $fh_csv)
    {
        err_log("write_insert_log_csv: open $INSERT_LOG_CSV fail\n");
        return FALSE;
    }

    # ������־����
    my ($ne_name, $log_file_name, $table_name, $load_csv_num, $load_csv_time, 
        $succ_bill_num, $fail_bill_num, $log_create_time);
    foreach my $log_entry (@$insert_sqlldr_log_data)
    {
        $ne_name = $log_entry->{'ne_name'};
        $log_file_name = $log_entry->{'log_file_name'};
        $table_name = $log_entry->{'table_name'};
        $load_csv_num = $log_entry->{'load_csv_num'};
        $load_csv_time = $log_entry->{'load_csv_time'};
        $succ_bill_num = $log_entry->{'succ_bill_num'};
        $fail_bill_num = $log_entry->{'fail_bill_num'};
        $log_create_time = $log_entry->{'log_create_time'};

        print $fh_csv "$ne_name,$log_file_name,$table_name,'$load_csv_time',$load_csv_num,$succ_bill_num,$fail_bill_num,'$log_create_time'\n";
    }

    # �ر�CSV�ļ�
    $fh_csv->close;

    return TRUE;
}

# 
# ����sqlldr��־CSV�ļ����
#
sub load_insert_log_csv
{
    my $load_time = get_time();

    # ����sqlldr����������
    my @results = `sqlldr $DB_USER/$DB_PASSWORD\@$DB_SERVER control=$INSERT_LOG_TEMPLATE log=$LOG_DIR/$INSERT_LOG_TABLE\_$load_time\_sqlldr.log bad=$LOG_DIR/$INSERT_LOG_TABLE\_$load_time\_sqlldr.bad`;

    if(@results == 0)
    {
	    return FALSE;
    }

    return TRUE;
}

# 
# write insert run log to csv file
#
sub write_log_to_csv_csv
{
    my $insert_run_log_data = shift;

    # ��CSV�ļ�
    my $fh_csv = new FileHandle($LOG_TO_CSV_CSV, "a+");
    if(!defined $fh_csv)
    {
        err_log("write_log_to_csv_csv: open $LOG_TO_CSV_CSV fail\n");
        return FALSE;
    }

    # ������־����
    my ($log_file_name, $load_csv_name);
    foreach my $log_entry (@$insert_run_log_data)
    {
        $log_file_name = $log_entry->{'log_file_name'};
        $load_csv_name = $log_entry->{'load_csv_name'};

        print $fh_csv "$log_file_name,$load_csv_name\n";
    }

    # �ر�CSV�ļ�
    $fh_csv->close;

    return TRUE;
}

# 
# �������������־CSV���
#
sub load_log_to_csv_csv
{
    my $load_time = get_time();

    # ����sqlldr����������
    my @results = `sqlldr $DB_USER/$DB_PASSWORD\@$DB_SERVER control=$LOG_TO_CSV_TEMPLATE log=$LOG_DIR/$LOG_TO_CSV_TABLE\_$load_time\_sqlldr.log bad=$LOG_DIR/$LOG_TO_CSV_TABLE\_$load_time\_sqlldr.bad`;

    if(@results == 0)
    {
	    return FALSE;
    }

    return TRUE;
}

#
# ��ȡ��ǰ����ʱ��
#
sub get_time
{
    my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime;
    return sprintf("%04d%02d%02d%02d%02d%02d", $year + 1900, $mon + 1, $mday, $hour, $min, $sec);
}

#
# ���ǰһ�������
#
sub get_last_day
{
    my $now_time = time;
    my $yesterday_time = $now_time - 24 * 60 * 60;
    my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime $yesterday_time;
    return sprintf("%04d%02d%02d", $year + 1900, $mon + 1, $mday);
}

# 
# ת������ʱ���ַ���: YYYYMMDDhhmmss��ʽ
# ����׼ʱ���ʽ��YYYY-MM-DD hh24:mi:ss
# 
sub to_date
{
    my $date_str = shift;
    if(length($date_str) != 14)
    {
        return undef;
    }

    my $year = substr $date_str, 0, 4;
    my $mon = substr $date_str, 4, 2;
    my $day = substr $date_str, 6, 2;
    my $hour = substr $date_str, 8, 2;
    my $min = substr $date_str, 10, 2;
    my $sec = substr $date_str, 12, 2;

    return sprintf("%04d-%02d-%02d %02d:%02d:%02d", $year, $mon, $day, $hour, $min, $sec);
}

#
# ��¼������־
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
    print ERR_LOG (strftime "%a %b %e %H:%M:%S %Y\n", localtime);
    print ERR_LOG $err_msg;
    close ERR_LOG;
}

