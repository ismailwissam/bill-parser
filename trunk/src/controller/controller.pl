use strict;
use Time::Local;
use Getopt::Long;
use FileHandle;
use POSIX qw(strftime);

# ����ȫ�ֳ���
my $USAGE =<<"EOF";
Usage:$0 -r <run_path>
Options:
       -r run path, default is ./
EOF

my $LOG_DIR = "./log";
my $CONFIG_DIR = "./conf";

my $ERR_LOG_FILE = "$LOG_DIR/controller_err";
my $CONFIG_FILE = "$CONFIG_DIR/controller.conf";

use constant
{
    FALSE => 0,
    TRUE =>1
};

# ����ȫ�ֱ���
my $g_RunPath = './';
my $g_TermFlag = FALSE;

# ע���źŴ�����
$SIG{SIGINT} = sub { $g_TermFlag = TRUE; };

# �������
main();

sub main
{
    # ��ȡ����
    my %opts;
    if(!GetOptions(\%opts, "r=s"))
    {
        err_log("main: error parameters!\n");
        die "error parameters!\n$USAGE\n";
    }
    $g_RunPath = $opts{"r"};

    # ��֤�������
    if((!defined $g_RunPath) || (! -d $g_RunPath))
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

    # ���LOGĿ¼
    if((!defined $LOG_DIR) || (! -d $LOG_DIR))
    {
        err_log("main: invalid log dir: $LOG_DIR\n");
        die "invalid log dir: $LOG_DIR\n";
    }

    # ���CONFĿ¼
    if((!defined $CONFIG_DIR) || (! -d $CONFIG_DIR))
    {
        err_log("main: invalid config dir: $CONFIG_DIR\n");
        die "invalid config dir: $CONFIG_DIR\n";
    }

    # ����������Ϣ
    my $ref_config = load_config($CONFIG_FILE);
    if(!defined $ref_config)
    {
        err_log("main: load config fail\n");
        die "load config fail\n";
    }

    # �����ػ�״̬
    daemon_start();

    # ��ؽ���״̬
    while(1)
    {
        # ��زɼ�����
        if( !detect_collect_process() )
        {
            start_collect_process($ref_config->{'Path'}->{'collect_run_path'});
        }

        # ��ؽ�������
        if( !detect_pretreat_process() )
        {
            start_pretreat_process($ref_config->{'Path'}->{'pretreat_run_path'});
        }

        # ���������
        if( !detect_insert_process() )
        {
            start_insert_process($ref_config->{'Path'}->{'insert_run_path'});
        }

        # ����������жϱ�־�����˳���ͬʱ�ս����м�ؽ���
        if($g_TermFlag)
        {
            if( detect_collect_process() )
            {
                stop_collect_process($ref_config->{'Path'}->{'collect_run_path'});
            }

            if( detect_pretreat_process() )
            {
                stop_pretreat_process($ref_config->{'Path'}->{'pretreat_run_path'});
            }

            if( detect_insert_process() )
            {
                stop_insert_process($ref_config->{'Path'}->{'insert_run_path'});
            }

            # �˳�
            last;
        }

        # �ӳ��趨�ķ��Ӻ��ټ��
        sleep $ref_config->{'Generic'}->{'delay_minute'} * 60;
    }
}

#
# ��������
#
sub load_config
{
    my $config_file = shift;
    my $ref_config = {};

	if ( !(-e $config_file) )
	{
		err_log("load_config: can not find $config_file\n");
        return undef;
	}

	my $FH = new FileHandle("$config_file") || die "can not open $config_file\n";
	my ($section, $line) ;
	while( $line = <$FH> )
	{
		if( $line =~ /^\s*\[(\S+)\]\s*$/ )
		{
			$section = $1;
			next ;
		}

	   	if( $line =~ /^\s*(\w+)\s*=\s*(\S+)\s*$/ )
	   	{
	   		$ref_config->{$section}->{$1} = $2;
	   		next;
	   	}
	}
	close($FH);

    return $ref_config;
}

#
# ���ɼ������Ƿ����
#
sub detect_collect_process
{
    my $inst_num = 0;
    my @results = `ps -ef | grep "collect " | grep -v grep`;
    for my $line (@results) {
        if ($line =~ /zj-bill-parser/gi) {
            $inst_num++;
        }
    }

    if ($inst_num > 0) {
        return TRUE;
    }

    return FALSE;
}

# 
# �����ɼ�����
#
sub start_collect_process
{
    my $collect_run_path = shift;

    system "$collect_run_path/start_collect";
}

# 
# ֹͣ�ɼ�����
#
sub stop_collect_process
{
    my $collect_run_path = shift;
    system "$collect_run_path/stop_collect";
}

#
# �����������Ƿ����
#
sub detect_pretreat_process
{
    my $inst_num = 0;
    my @results = `ps -ef | grep "pretreat " | grep -v grep`;
    for my $line (@results) {
        if ($line =~ /zj-bill-parser/gi) {
            $inst_num++;
        }
    }

    if ($inst_num > 0) {
        return TRUE;
    }

    return FALSE;
}

# 
# ������������
#
sub start_pretreat_process
{
    my $pretreat_run_path = shift;
    system "$pretreat_run_path/start_pretreat";
}

# 
# ֹͣ��������
#
sub stop_pretreat_process
{
    my $pretreat_run_path = shift;
    system "$pretreat_run_path/stop_pretreat";
}

#
# ����������Ƿ����
#
sub detect_insert_process
{
    my $inst_num = 0;
    my @results = `ps -ef | grep "insert " | grep -v grep`;
    for my $line (@results) {
        if ($line =~ /zj-bill-parser/gi) {
            $inst_num++;
        }
    }

    if ($inst_num > 0) {
        return TRUE;
    }

    return FALSE;
}

# 
# ����������
#
sub start_insert_process
{
    my $insert_run_path = shift;
    system "$insert_run_path/start_insert";
}

# 
# ֹͣ������
#
sub stop_insert_process
{
    my $insert_run_path = shift;
    system "$insert_run_path/stop_insert";
}

# 
# �����ػ�����
#
sub daemon_start
{
    umask 022;

    return if(getppid() == 1);

    $SIG{TTOU} = 'IGNORE';
    $SIG{TTIN} = 'IGNORE';
    $SIG{TSTP} = 'IGNORE';
    $SIG{USR1} = 'IGNORE';
    $SIG{USR2} = 'IGNORE';

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
# ��¼������־
#
sub err_log
{
    my $err_msg = shift;

    open ERR_LOG, ">>$ERR_LOG_FILE";
    print ERR_LOG "------------------------------------------------------------------\n";
    print ERR_LOG (strftime "%a %b %e %H:%M:%S %Y", localtime);
    print ERR_LOG $err_msg;
    close ERR_LOG;
}

