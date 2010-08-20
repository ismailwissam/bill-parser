/***************************************************************************/
/* insert.c                                                                */
/*                                                                         */ 
/* ���������� v1.0                                                       */
/*                                                                         */
/* created by wangxiaohui at 2010.5.11                                     */
/*                                                                         */
/* modify by wangxiaohui at 2010.5.23                                      */
/*           �����޸ļ����˶������csv�ļ���������ʽ����֤��               */
/*           �����csv�ļ�������ʽ�����ǣ�                                 */
/*           �ɼ�����_����_�˾�_��������_��������ʱ��_ԭ�ļ���.csv       */
/*                                                                         */
/* modify by wangxiaohui at 2010.6.11                                      */
/*           �����޸���Ҫ��Ϊ����������ִ��Ч�ʣ�֮ǰ���ÿһ��csv�ļ��� */
/*           ����һ��sqlldr�������޸�Ϊ���һ�����ݿ���Ӧ������csv�ļ��� */
/*           ����һ��sqlldr��ǰ�õ�pretreatģ��Ҫ��֤�Ѳ�����csv�ļ����õ� */
/*           ��ȷ�ı�����Ŀ¼��                                            */
/*                                                                         */
/* modify by liuyang at 2010.6.11                                          */
/*           ��ɵ����޸�                                                  */
/*                                                                         */
/* modify by wangxiaohui at 2010.7.2                                       */
/*           ����������ǰ׺�б�Ӵ����з���������õ�����table.conf����  */
/*           �ļ�������������ĳЩ��Ԫ���Ʒ����ı䣬����������Ԫʱ�Ͳ���  */
/*           �޸Ĵ��룬ֻ���޸������ļ�����                                */
/***************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>

/* micor definition */
#define MAX_CHILD_PROCESS       128
#define MAX_BUFFER              2048
#define MAX_FILENAME            256
#define MAX_TABLENAME           64
#define MAX_TIME                15
#define SLEEP_TIME              60
#define DEFAULT_IN_DIR          "./in"
#define DEFAULT_RUN_DIR         "./"
#define DEFAULT_DB_USER         "obirawdb"
#define DEFAULT_DB_PASSWORD     "obirawdb\\$zj2010"
#define DEFAULT_DB_SERVER       "obidb3_132"
#define LOG_DIR                 "./log"
#define WORK_DIR                "./work"
#define CONFIG_DIR              "./conf"
#define CONTROL_TEMPLATE_DIR    "./template"
#define ERR_LOG_FILE            "./log/insert_err"
#define PREFIX_RUN_LOG_FILE     "./log/insert_run"
#define TABLE_LIST_CONFIG_FILE  "./conf/table.conf"
#define MAX_PARALLEL_TABLE      10
#define MAX_LDR_FILE_NUM        1000

/* data type definition */
typedef struct {
	int  pid;
	int  sleep_time;
} t_child_process_status;

typedef enum parse_code {
	PARSE_FAIL = -1,
	PARSE_MATCH	= 1,
	PARSE_UNMATCH = 2,
} parse_code_e;

/* global variant defintion */
char *       progname               = NULL;
char *       file_in_dir            = NULL;
char *       run_dir                = NULL;
char *       db_user                = NULL;
char *       db_password            = NULL;
char *       db_server              = NULL;
int          curr_process_number    = 0;
int          debug                  = 0;
int          parallel_child_process = 1;
int          sleep_time             = 5;
int          table_list_num         = 0;
int          g_term_flag            = 0;

/* static functions definitions */
static void         usage(int status);
static int          get_time(char * par);
static int          err_log(const char * format, ...);
static void         daemon_start(void);
static parse_code_e pre_suf_check(const char * name, const char * prefix,const char * suffix);
static int          commit_file(const char * in_file_name,  const char* in_dir_name, const char * begin_time, const char * end_time);
static int          insert(const char * in_dir_name);
static parse_code_e get_in_dir_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_dir_list);
static parse_code_e get_orig_file_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_file_name);
static int          process_insert(int current_number, int parallel_number);
static int          clear_dir_file(const char * dir_name, const char * prefix, const char * suffix);
static int          get_ne_name(const char * original_file_name, char * ret_ne_name);
static int          get_table_num(int * ret_table_num);
static int          get_table_name(int index, char * ret_table_name);
static void         sigint_handler(int signum);

/*
 *  Display the syntax for starting this program.
 */
static void usage(int status)
{
    FILE *output = status?stderr:stdout;
    
    fprintf(output,"Usage: %s [-i csv_input_path] [-r run_path] [-p parallel_child_number] \
                              [-t interval_time] [-u db_user] [-w db_password] [-s db_server] [-d]\n",progname);
    fprintf(output,"\nOptions:\n");
    fprintf(output,"        -i csv_input_path: csv input path, default is ./in\n");
    fprintf(output,"        -r run_path: insert program run path, default is ./\n");
    fprintf(output,"        -p parallel_child_number: the parallel child max number, default is 1\n");
    fprintf(output,"        -t interval_time: child process poll interval time");
    fprintf(output,"        -u db_user: specify db user\n");
    fprintf(output,"        -w db_password: specify db password\n");
    fprintf(output,"        -s db_server: specify db server\n");
    fprintf(output,"        -d debug flag\n");

    exit(status);
}

static int get_time(char * par)
{
	time_t t;
	struct tm *systime;

	t = time(NULL);
	if(t == -1)
	{
	   strcpy(par, "yyyymmddhhmiss");
	   return 1;
	}
	
	systime = localtime(&t);
	if(systime == NULL)
	{
	   strcpy(par, "yyyymmddhhmiss");
	   return 1;
	}
	
	sprintf(par, "%04d%02d%02d%02d%02d%02d", systime->tm_year+1900, systime->tm_mon+1, \
	        systime->tm_mday, systime->tm_hour, systime->tm_min, systime->tm_sec);
		
	return 0;
}

static int err_log(const char * format, ...)
{
	time_t t;
	struct tm *systime;
	FILE *fp;
	char tmp[MAX_FILENAME];
	va_list ap;

	if(debug)
	{
  		va_start(ap,  format);
  		vprintf(format, ap);
  		va_end(ap);
  		printf("\n");
	}

    if(curr_process_number != 0)
    {
        sprintf(tmp, "%s.%03d", ERR_LOG_FILE, curr_process_number);
        fp = fopen(tmp, "a+");
    }
    else
    {
        fp = fopen(ERR_LOG_FILE, "a+");
    }

	if(fp == NULL)
	{
		return 1;
	}
	
	t = time(NULL);
	systime = localtime(&t);
	fprintf(fp,  "------------------------------------------------------------\n");
	fprintf(fp,  "%s",  asctime(systime));

	va_start(ap,  format);
	vfprintf(fp, format, ap);
	va_end(ap);
	fprintf(fp, "\n");
  
	fclose(fp);
	
	return 0;
}

static void daemon_start(void)
{
     int childpid;

     umask(022);

     if(getppid() == 1) {  return; }

     signal(SIGTTOU, SIG_IGN);
     signal(SIGTTIN, SIG_IGN);
     signal(SIGTSTP, SIG_IGN);
     signal(SIGUSR1, SIG_IGN);

     if(setpgrp() == -1)
            err_log("daemon_start: can't change process group\n");
     signal(SIGHUP, SIG_IGN);

     
     if((childpid = fork()) < 0) {
            err_log("daemon_start: fork error\n");
            exit(1);
     }
     else if(childpid > 0)
             exit(0);
}

int main(int argc, char * argv[])
{
    int     ret = 0;
	int     argval;
 	int		circle_second = 300;
	struct  stat stat_buff;
    t_child_process_status child_process_status[MAX_CHILD_PROCESS];
	pid_t   childpid;
	pid_t   r_waitpid;
	int     i;

    /* register the signal SIGINT handler */
    if(signal(SIGINT, &sigint_handler) == SIG_ERR)
    {
        err_log("main: signal register fail\n");
        ret = 1;
        goto Exit_Pro;
    }
	
    /* ��ȡ�������� */
    if((progname = strrchr(argv[0],  '/')) == NULL)
    {
        progname = argv[0];
    }
    else
    {
        progname++;
    }
	
 	/*  �������  */
    while((argval = getopt(argc,  argv,  "i:r:p:t:u:w:s:d")) != EOF) 
    {
        switch(argval) 
        {
            case 'i':
                file_in_dir = strdup(optarg);
                    break;	
            case 'r':
                run_dir = strdup(optarg);
                break;	
            case 'p':
                parallel_child_process = atoi(optarg);
                break;
            case 't':
                circle_second = atoi(optarg);
                break;
            case 'u':
                db_user = strdup(optarg);
                break;
            case 'w':
                db_password = strdup(optarg);
                break;
            case 's':
                db_server = strdup(optarg);
                break;
            case 'd':
                debug = 1;
                break;	
            default:
                usage(1);
                break;
        }
	}

    /* ����Ĭ��ֵ */
    if(run_dir == NULL)
    {
        run_dir = (char *)malloc(MAX_FILENAME);
        if(run_dir == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(run_dir, DEFAULT_RUN_DIR);
    }

    if(file_in_dir == NULL)
    {
        file_in_dir = (char *)malloc(MAX_FILENAME);
        if(file_in_dir == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(file_in_dir, DEFAULT_IN_DIR);
    }

    if(db_user == NULL)
    {
        db_user = (char *)malloc(MAX_TABLENAME);
        if(db_user == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(db_user, DEFAULT_DB_USER);
    }

    if(db_password == NULL)
    {
        db_password = (char *)malloc(MAX_TABLENAME);
        if(db_password == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(db_password, DEFAULT_DB_PASSWORD);
    }

    if(db_server == NULL)
    {
        db_server = (char *)malloc(MAX_TABLENAME);
        if(db_server == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(db_server, DEFAULT_DB_SERVER);
    }

	/* ���ָ��������Ŀ¼���л�������Ŀ¼;������ǵ�ǰĿ¼ */
    if(chdir(run_dir) == -1)
    {
        err_log("main: chdir to %s fail\n", run_dir);
        ret = 1;
        goto Exit_Pro;
    }

	/* �����־Ŀ¼ */
	if(stat(LOG_DIR, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", LOG_DIR);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", LOG_DIR);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* ��鹤��Ŀ¼ */
	if(stat(WORK_DIR, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", WORK_DIR);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", WORK_DIR);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* �������Ŀ¼ */
	if(stat(CONFIG_DIR, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", CONFIG_DIR);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", CONFIG_DIR);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* ��������ļ�Ŀ¼ */
	if(stat(file_in_dir, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", file_in_dir);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", file_in_dir);
            ret = 1;
            goto Exit_Pro;
        }
	}

    /* ����WORKĿ¼���ļ� */
    if(clear_dir_file(WORK_DIR, NULL, NULL) != 0)
    {
		err_log("main: clear %s fail\n", WORK_DIR);
        ret = 1;
        goto Exit_Pro;
    }

    /* ��ȡ���б���Ŀ */
    if(get_table_num(&table_list_num) != 0)
    {
		err_log("main: get table num fail\n");
        ret = 1;
        goto Exit_Pro;
    }
    if(table_list_num == 0)
    {
		err_log("main: table config incorrect\n");
        ret = 1;
        goto Exit_Pro;
    }

	/* У���ӽ��̵�������Ŀ */
	if(parallel_child_process <= 0)
    {
        parallel_child_process = 1;
    }
	if(parallel_child_process > MAX_CHILD_PROCESS) 
    {
        parallel_child_process = MAX_CHILD_PROCESS;
    }
    if(parallel_child_process > table_list_num)
    {
        parallel_child_process = table_list_num;
    }
	
	// ����sleep_time
 	sleep_time = circle_second * parallel_child_process / table_list_num;
 	if (sleep_time < 5)
 	{
		sleep_time = 5;
 	}
        
	/* ��ʼ���ӽ��̽ṹ���� */
    for(i = 0; i < MAX_CHILD_PROCESS; i++)
    {
        memset(&child_process_status[i], 0, sizeof(t_child_process_status));
    }
    
    /* ������ǵ���ģʽ�����ػ��������� */
    if(debug == 0)
    {
        daemon_start();
    }

	/* ѭ����������̽�����⴦�� */
	while(1)
	{
		/*�����ӽ���*/
		for(i = 0; i < parallel_child_process; i++)
		{
			if(child_process_status[i].pid == 0 && child_process_status[i].sleep_time <= 0)
			{
				if((childpid = fork()) < 0) /*fork error*/
				{
                    err_log("main: fork error\n");
                    exit(1);
                }
                else if(childpid > 0)     /*parent process*/ 
                {
                    child_process_status[i].pid        = childpid;
                    child_process_status[i].sleep_time = SLEEP_TIME;
                }
                else if(childpid == 0)    /*child process*/
                {
                     process_insert(i + 1, parallel_child_process);
                    exit(0);
                }
            }
        }
	
		/*�����ӽ���*/
		for(i = 0; i < parallel_child_process; i++)
		{
			if(child_process_status[i].pid > 0)
			{
				r_waitpid = waitpid(child_process_status[i].pid, NULL, WNOHANG);
				if(r_waitpid > 0)
				{
					child_process_status[i].pid = 0;
				}
				if(r_waitpid < 0)
				{
					err_log("main: waitpid fail\n");
					exit(1);
				}
			}
		}
			
        /* �����������ֹ��־�����˳�����ѭ�� */
        if(g_term_flag)
        {
            /* ���˳�֮ǰ�����еĲɼ��ӽ���ɱ�� */
            for(i = 0; i < parallel_child_process; i++)
            {
                if(child_process_status[i].pid > 0)
                {
                    kill(child_process_status[i].pid, SIGKILL);
                }
            }
            break;
        }
	
		sleep(1);
		
		/*ÿ��ݼ�sleep_time*/
		for(i = 0; i < parallel_child_process; i++)
		{
            if(child_process_status[i].pid == 0 && child_process_status[i].sleep_time > 0)
            {
                child_process_status[i].sleep_time--;
            }
		}
	}
	
Exit_Pro:
    if(run_dir != NULL)
    {
        free(run_dir);
        run_dir = NULL;
    }
    if(file_in_dir != NULL)
    {
        free(file_in_dir);
        file_in_dir = NULL;
    }
    if(db_user != NULL)
    {
        free(db_user);
        db_user = NULL;
    }
    if(db_password != NULL)
    {
        free(db_password);
        db_password = NULL;
    }
    if(db_server != NULL)
    {
        free(db_server);
        db_server = NULL;
	}
   	return ret;
}

/*
 *  pre_suf_check()
 */
static parse_code_e pre_suf_check(const char * name,const char * prefix,const char * suffix)
{
	if(name == NULL)
		return PARSE_UNMATCH;
		
	/*ǰ׺ƥ����*/
	if(prefix !=  NULL)
	{
		if(strstr(name, prefix) != name)
			return PARSE_UNMATCH;
	}
	/*��׺ƥ����*/
	if(suffix != NULL)
	{
		if(strlen(name) < strlen(suffix))
			return PARSE_UNMATCH;
		if(strcmp((name+strlen(name)-strlen(suffix)), suffix) != 0)
			return PARSE_UNMATCH;
	}
	
	return PARSE_MATCH;
}


/*
 *commit_file()
 *              return 0 success,  1 fail
 *
 */
static int commit_file(const char * in_file_name, const char* in_dir_name, const char * begin_time, const char * end_time)
{
	int   ret;
	FILE  * fp = NULL;
	char  run_log_time[MAX_TIME];
	char  tmp_file_name[MAX_FILENAME];
    char  ne_name[MAX_TABLENAME];

	ret = 0;

    /* ȡ����Ԫ���� */
    memset(ne_name, 0, sizeof(ne_name));
    if(get_ne_name(in_file_name, ne_name) != 0)
    {
        err_log("commit_file: get ne name fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    /* ����ɲ�����ļ���¼insert_run.YYYYMMDD�ļ��� */
	get_time(run_log_time);
	run_log_time[8] = '\0';
	
	sprintf(tmp_file_name, "%s.%s.%s", PREFIX_RUN_LOG_FILE, ne_name, run_log_time);

	fp = fopen(tmp_file_name, "a+");
	if(fp == NULL)
	{
		err_log("commit_file: fopen %s fail\n", tmp_file_name);
		ret = 1;
		goto Exit_Pro;
	}

    sprintf(tmp_file_name, "%s_%s_sqlldr.log", in_dir_name, begin_time);
    //���ص�csv�ļ� ���� ����ʱ���ɵ�log�ļ� ��ʼ����ʱ�� ��������ʱ��
	fprintf(fp, "%s %s %s %s %s\n", in_file_name, in_dir_name, tmp_file_name, begin_time, end_time);

	fclose(fp);
	fp = NULL;	

    /* ɾ�������ļ�in_file_name */
    sprintf(tmp_file_name, "%s/%s/%s", file_in_dir, in_dir_name, in_file_name);
	if(unlink(tmp_file_name) != 0)
	{
		err_log("commit_file: unlink file %s fail\n", in_file_name);
		ret = 1;
		goto Exit_Pro;
	}
	
Exit_Pro:
	if(fp != NULL)
    {
        fclose(fp);
    }
	return ret;
}

/*
 *insert()
 *           return 0 success,  1 fail
 *
 */
static int insert(const char * in_dir_name)
{
	int             ret = 0;
	char			ldr_file_list[MAX_LDR_FILE_NUM][MAX_FILENAME];
	DIR *           pdir = NULL;
	struct dirent *	pdirent;
	int             index = 0;
	char            cmd[MAX_BUFFER];
	char            control_template[MAX_FILENAME];
	char            control_file[MAX_FILENAME];
	char			table_prefix[MAX_FILENAME];
	char			infile_one_line[MAX_FILENAME];
	int				i;
    char            *ptr_front = NULL, *ptr_back = NULL;
 	char            begin_time[MAX_TIME];
 	char            end_time[MAX_TIME];
	char            tmp_db_password[MAX_TABLENAME];
	char			tmp[MAX_FILENAME];
	FILE			*ctl_template = NULL;
	FILE			*ctl = NULL;
	char			read_ch;			

    /* �������Ŀ¼����Ч�� */
	if( NULL == in_dir_name )
	{
		err_log("insert: dir_name is NULL\n");
		ret = 1;
		goto Exit_Pro;	
	}

	/* ��Ŀ¼ */
	sprintf(tmp,"%s/%s",file_in_dir,in_dir_name);
	pdir = opendir(tmp);
	if(pdir == NULL)
	{
		err_log("insert: opendir fail: %s\n",tmp);
		ret = 1;
		goto Exit_Pro;	
	}

	/* ɨ���ļ����������ļ� */
	memset(ldr_file_list,0,sizeof(ldr_file_list));
	while((pdirent = readdir(pdir)) != NULL)
	{
		/* �򵥼��,����.csv��β���� */
		if(pre_suf_check(pdirent->d_name, NULL, ".csv") != PARSE_MATCH)
        {
			continue;
        }

		/* �ѷ���Ҫ����ļ����������� */
		strcpy(ldr_file_list[index++], pdirent->d_name);
		if(index == MAX_LDR_FILE_NUM)
        {
            break;
        }
	}

    /* �ر�Ŀ¼ */
    closedir(pdir);
    pdir = NULL;

    /* ���û���ҵ�����Ҫ����ļ������˳� */
	if (index == 0)
	{
		goto Exit_Pro;	
	}

	/* ��ȡģ���ļ��� */
	strcpy(table_prefix, in_dir_name);            //in_dir_name :  OP_BILL_NSN_MOC_SH9_0611
    ptr_front = table_prefix;
    i = 0;
    while(i < 2)
    {
        ptr_back = strrchr(ptr_front, '_');
        if(ptr_back != NULL)
        {
            *ptr_back = '\0';
        }
        else
        {
            err_log("insert: invalid input dir: %s\n", in_dir_name);
            ret = 1;
            goto Exit_Pro;
        }
        i++;
    }
	sprintf(control_template, "%s/%s.ctl.template", CONTROL_TEMPLATE_DIR, table_prefix);

	/* ��ȡctl�ļ��� */
    sprintf(control_file, "%s/%s.ctl", WORK_DIR, in_dir_name);

	/* ����ctl�ļ� */
	if (NULL == (ctl_template = fopen(control_template,"r")))
	{
		err_log("insert: open file fail: %s\n",control_template);
		ret = 1;
		goto Exit_Pro;	
	}
	if (NULL == (ctl = fopen(control_file,"w")))
	{
		err_log("insert: open file fail: %s\n",control_file);
		ret = 1;
		goto Exit_Pro;	
	}

	memset(tmp,0,sizeof(tmp));
	while( EOF != (read_ch = fgetc(ctl_template)) )
	{
		if ('$' == read_ch )
		{
			fread(tmp,9,1,ctl_template);
			tmp[9] = '\0';
			if (0 == strcmp("DATA_FILE", tmp))
			{
				for(i = 0; (strlen(ldr_file_list[i]) > 0) && (i < MAX_LDR_FILE_NUM) ; ++i)
				{
					sprintf(infile_one_line,"infile \"%s/%s/%s\"\n",file_in_dir,in_dir_name,ldr_file_list[i]);
					fwrite(infile_one_line,strlen(infile_one_line),1,ctl);
				}
			}
			else if (0 == strcmp("TABLE_NAM", tmp))
			{
				read_ch = fgetc(ctl_template);
				fwrite(in_dir_name,strlen(in_dir_name),1,ctl);
			}
			else
			{
				err_log("insert: ctl_template file error: %s\n",control_template);
				ret = 1;
				goto Exit_Pro;	
			}
		}
		else
		{
			fputc(read_ch,ctl);
		}
	}

	if (ctl != NULL)
	{
		fclose(ctl);
		ctl = NULL;
	}
	if (ctl_template != NULL)
	{
		fclose(ctl_template);
		ctl_template = NULL;
	}

    /* Ԥ���������ַ��� 
     * ��Ϊ���봮����'$'�ַ���������ַ���sqlldrʹ��ʱ����
     * ֱ�ӳ��֣�������ǰ����Ϸ����ַ�'\' */
    memset(tmp_db_password, 0, sizeof(tmp_db_password));
    ptr_front = db_password;
    while((ptr_back = strchr(ptr_front, '$')) != NULL)
    {
        strncat(tmp_db_password, ptr_front, ptr_back - ptr_front);
        strcat(tmp_db_password, "\\$");
        ptr_front = ++ptr_back;
    }
    if(strlen(ptr_front) > 0)
    {
        strcat(tmp_db_password, ptr_front);
    }

    /* ��ȡ��⿪ʼʱ�� */
    get_time(begin_time);

	/* ����sqlldr��������� */
	memset(cmd, 0, sizeof(cmd));
 	sprintf(cmd, "sqlldr %s/%s@%s control=%s log=%s/%s_%s_sqlldr.log bad=%s/%s_%s_sqlldr.bad > /dev/null", 
 		db_user, tmp_db_password, db_server, control_file, LOG_DIR, in_dir_name, begin_time, LOG_DIR, in_dir_name, begin_time);
	if(system(cmd) == -1)
	{
		err_log("insert: call sqlldr fail\n");
		ret = 1;
		goto Exit_Pro;
	}
	
    /* ��ȡ������ʱ�� */
    get_time(end_time);
	
    /* ɾ���������Ŀ����ļ� */
	if(unlink(control_file) != 0)
	{
		err_log("insert: unlink control file %s fail\n", control_file);
		ret = 1;
		goto Exit_Pro;
	}
	
    /*�ύ�ļ�*/
	for(i = 0; (strlen(ldr_file_list[i]) > 0) && (i < MAX_LDR_FILE_NUM) ; ++i)
	{
		if(commit_file(ldr_file_list[i], in_dir_name, begin_time, end_time) != 0)
		{
			err_log("insert: commit_file %s fail\n", ldr_file_list[i]);
			ret = 1;
			goto Exit_Pro;
		}
	}

Exit_Pro:
    if(pdir != NULL)
    {
        closedir(pdir);
    }
	if (ctl != NULL)
	{
		fclose(ctl);
		ctl = NULL;
	}
	if (ctl_template != NULL)
	{
		fclose(ctl_template);
		ctl_template = NULL;
	}
	
	return ret;
}


/*
 *  get_in_dir_name()
 */
static parse_code_e get_in_dir_name(const char * dir_name, const char * prefix,const char * suffix, char * ret_dir_list)
{
	int              ret;
	DIR *            pdir = NULL;
	struct dirent *	 pdirent;
	struct stat stat_buff;
	char             tmp_file_name[MAX_FILENAME];
    int              index;
	
	ret = PARSE_UNMATCH;

    /* ���ָ��Ŀ¼����Ч�� */
	if(dir_name == NULL)
	{
		err_log("get_in_dir_name: dir_name is NULL\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
	}
	
    /* ��ָ����Ŀ¼ */
	pdir = opendir(dir_name);
	if(pdir == NULL)
	{
		err_log("get_in_dir_name: opendir fail\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
	}

    index = 0;
	while((pdirent = readdir(pdir)) != NULL)
	{
		/*ǰ��׺ƥ����*/
		if(pre_suf_check(pdirent->d_name, prefix, suffix) != PARSE_MATCH)
        {
			continue;
        }

        /*�ļ����*/
        strcpy(tmp_file_name, dir_name);
        strcat(tmp_file_name, "/");
        strcat(tmp_file_name, pdirent->d_name);
            
		if(stat(tmp_file_name, &stat_buff) == -1)
		{
			err_log("get_in_dir_name: stat %s fail\n", pdirent->d_name);
			ret = PARSE_FAIL;
			goto Exit_Pro;
		}

        if(S_ISDIR(stat_buff.st_mode))
        {
			strcat(ret_dir_list, pdirent->d_name);
			strcat(ret_dir_list, "|");
            ret = PARSE_MATCH;

            index++;
        }	
        
        /* �����ȡMAX_PARALLEL_TABLE��Ŀ¼ */
        if(index == MAX_PARALLEL_TABLE)
        {
            break;
        }
	}

Exit_Pro:
	if(pdir != NULL)
    {
        closedir(pdir);
    }
	return ret;
}

/* 
 *  get_orig_file_name()
 */
static parse_code_e get_orig_file_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_file_name)
{
	int              ret;
	DIR *            pdir = NULL;
	struct dirent *	 pdirent;
	struct stat stat_buff;
	char             tmp_file_name[MAX_FILENAME];
	
	ret = PARSE_UNMATCH;

    /* ���ָ��Ŀ¼����Ч�� */
	if(dir_name == NULL)
	{
		err_log("get_orig_file_name: dir_name is NULL\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
	}
	
    /* ��ָ����Ŀ¼ */
	pdir = opendir(dir_name);
	if(pdir == NULL)
	{
		err_log("get_orig_file_name: opendir fail\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
	}

	while( (pdirent = readdir(pdir)) != NULL )
	{
		/* ǰ��׺ƥ���� */
		if( pre_suf_check(pdirent->d_name, prefix, suffix) != PARSE_MATCH )
        {
			continue;
        }

        /* �ļ���� */
        strcpy(tmp_file_name, dir_name);
        strcat(tmp_file_name, "/");
        strcat(tmp_file_name, pdirent->d_name);
            
		if(stat(tmp_file_name, &stat_buff) == -1)
		{
			err_log("get_orig_file_name: stat %s fail\n", pdirent->d_name);
			ret = PARSE_FAIL;
            goto Exit_Pro;	
		}

        if(S_ISREG(stat_buff.st_mode))
        {
            strcpy(ret_file_name, pdirent->d_name);
            ret = PARSE_MATCH;
            break;	
        }	
	}

Exit_Pro:
	if(pdir != NULL)
        	closedir(pdir);
	return ret;
}

/*
 *  process_insert()
 */
static int process_insert(int current_number, int parallel_number)
{
	char    dir_list[MAX_PARALLEL_TABLE][MAX_FILENAME];
	char	dir_list_string[MAX_BUFFER];
	char*	ptr_hea = NULL;
	char*	ptr_tra = NULL;
    char    table_name[MAX_TABLENAME];
	char    temp_table_pre[MAX_TABLENAME];
    int     skip_num, begin_no, end_no;
    int     i, j, count;

	/*����ȫ���ӽ��̱��*/
	curr_process_number = current_number;

    /* �����ӽ��̱�ż����Ӧ�Ĵ���Ŀ¼���� */
    skip_num = (table_list_num % parallel_number) ? 
        (table_list_num / parallel_number + 1) : (table_list_num / parallel_number);
    begin_no = (current_number - 1) * skip_num;
    end_no = begin_no + skip_num;
    if(end_no > table_list_num)
    {
        end_no = table_list_num;
    }
	
    /* ��ѯ�����Ӧ��Ŀ¼���� */
    for(i = begin_no; i < end_no; ++i)
    {
        memset(dir_list, 0, sizeof(dir_list));
        memset(dir_list_string, 0, sizeof(dir_list_string));
        memset(table_name, 0, sizeof(table_name));
        if(get_table_name(i, table_name) != 0)
        {
            err_log("process_insert: get table name fail\n");
            continue;
        }
		sprintf(temp_table_pre,"%s_",table_name);
		// ��ȡĿ¼�Ĵ������ļ����б�, ����dir_listĿ¼�б�
        if(get_in_dir_name(file_in_dir, temp_table_pre, NULL, dir_list_string) == PARSE_MATCH)
        {
			count = 0;
			ptr_hea = dir_list_string;
			while((ptr_tra = strchr(ptr_hea, '|')) != NULL)
			{
				strncpy(dir_list[count++], ptr_hea, ptr_tra - ptr_hea);
				ptr_hea = ++ptr_tra;
			}
            if(strlen(ptr_hea) > 0)
            {
                strcpy(dir_list[count++], ptr_hea);
            }
			//�ļ��д���ʱ����
            for(j = 0; (j < count) && (strlen(dir_list[j]) > 0); ++j)
            {
                /*�ļ�Ԥ����*/
 				if(insert(dir_list[j]) != 0)
 				{
 					err_log("process_insert: insert fail\n");
 					break;
 				}
            }
        }

        /* �ȴ�һ����ʱ���� */
        sleep(sleep_time);
    }
    return 0;
}

/*
 * clear_work_dir()
 *
 * return 0 success, 1 fail.
 */
static int clear_dir_file(const char * dir_name, const char * prefix, const char * suffix)
{
    char    file_name[MAX_FILENAME];
    char    tmp_file_name[MAX_FILENAME];

	memset(file_name, 0, sizeof(file_name));
	while(get_orig_file_name(dir_name, prefix, suffix, file_name) == PARSE_MATCH)
	{
        sprintf(tmp_file_name, "%s/%s", dir_name, file_name);
		if(unlink(tmp_file_name) != 0)
		{
			err_log("clear_dir: unlink file %s fail\n", tmp_file_name);
			return 1;
		}
		memset(file_name, 0, sizeof(file_name));
    }

    return 0;
}

static int get_ne_name(const char * original_file_name, char * ret_ne_name)
{
    const char *ptr_front = NULL, *ptr_back = NULL;

    ptr_front = strrchr(original_file_name, '/');
    if(ptr_front == NULL)
    {
        ptr_front = original_file_name;
    }
    else 
    {
        ptr_front++;
    }

    /* �ɼ����� */
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }

    /* ���� */
    ptr_front = ++ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }

    /* ��Ԫ���� */
    ptr_front = ++ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }

    /* ��ȡ��Ԫ���� */
    strncpy(ret_ne_name, ptr_front, ptr_back - ptr_front);

    return 0;
}

static int get_table_num(int * ret_table_num)
{
    int  ret = 0;
    FILE *fp = NULL;
    char buff[MAX_TABLENAME];
    int  len;
    int  count = 0;

    fp = fopen(TABLE_LIST_CONFIG_FILE, "r");
    if(fp == NULL)
    {
        err_log("get_table_num: fopen %s fail\n", TABLE_LIST_CONFIG_FILE);
        ret = 1;
        goto Exit_Pro;
    }
    
    memset(buff, 0, sizeof(buff));
    while(fgets(buff, MAX_TABLENAME, fp) != NULL)
    {
        len = strlen(buff);
        while(buff[len-1] == '\n' || 
              buff[len-1] == '\r' || 
              buff[len-1] == '\t' ||
              buff[len-1] == ' ')
        {
            buff[len-1] = '\0';
            len = strlen(buff);
            if(len == 0)
                break;
        }

        if(strlen(buff) == 0)
            continue;

        count++;
        memset(buff, 0, sizeof(buff));
    }

    (*ret_table_num) = count;

Exit_Pro:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return ret;
}

static int get_table_name(int index, char * ret_table_name)
{
    int  ret = 0;
    FILE *fp = NULL;
    char buff[MAX_TABLENAME];
    int  len;
    int  count = 0;

    fp = fopen(TABLE_LIST_CONFIG_FILE, "r");
    if(fp == NULL)
    {
        err_log("get_table_name: fopen %s fail\n", TABLE_LIST_CONFIG_FILE);
        ret = 1;
        goto Exit_Pro;
    }
    
    memset(buff, 0, sizeof(buff));
    while(fgets(buff, MAX_TABLENAME, fp) != NULL)
    {
        len = strlen(buff);
        while(buff[len-1] == '\n' || 
              buff[len-1] == '\r' || 
              buff[len-1] == '\t' ||
              buff[len-1] == ' ')
        {
            buff[len-1] = '\0';
            len = strlen(buff);
            if(len == 0)
                break;
        }

        if(strlen(buff) == 0)
            continue;

        if(count == index)
        {
            strcpy(ret_table_name, buff);
            break;
        }

        count++;

        memset(buff, 0, sizeof(buff));
    }


Exit_Pro:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return ret;
}

static void sigint_handler(int signum)
{
    if(signum == SIGINT)
    {
        g_term_flag = 1;
    }
}

