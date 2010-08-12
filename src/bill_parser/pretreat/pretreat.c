/****************************************************************************/
/* pretreat.c                                                               */
/*                                                                          */ 
/* 话单预处理程序                                                           */
/*                                                                          */
/* create by wangxiaohui at 2010.5.13                                       */
/*                                                                          */
/* modify by wangxioahui at 2010.5.15                                       */
/*        本次修改主要是调整接口参数，接口的第二个参数原本是输入参数，      */
/*        提供解析后的CSV文件名；现在变更为输入输出参数，输入时提供保存     */
/*        CSV文件的路径，输出时把所有生成的CSV文件名追加在路径后面返回，    */
/*        装配格式为：CSV文件保存路径:CSV文件名1;CSV文件名2[;...]           */
/*                                                                          */
/* modify by wangxiaohui at 2010.5.19                                       */
/*        本次修改主要是调整多进程的调度算法，不再针对采集点分配进程，而是  */
/*         每个进程可以处理任意话单文件，但话单文件前面必须有@标志，否则不予*/
/*        处理。                                                            */
/*                                                                          */
/* modify by wangxiaohui at 2010.5.23                                       */
/*        加入对输入文件命名格式的验证： valid_in_file_name                 */
/*        输入的话单文件命名格式必须是：                                    */
/*               采集点编号_厂家_端局_话单生成时间(YYYYMMDDhhmmss)_原文件名 */
/*                                                                          */
/* modify by 刘洋 at 2010.6.11                                              */
/*        新的commitfile:获取表名,判断文件夹存在否,建文件夹,link过去.       */
/*        定时清空旧的文件夹(守护进程处理):									*/
/*        凌晨2点(延时2小时是不允许的)的时候扫描所有文件夹后缀,             */
/*        昨天的空文件夹通通删除,											*/
/*        不为空的扔到error文件夹并记录日志.                                */
/*                                                                          */
/* modify by wangxiaohui at 2010.6.22                                       */
/*        把对data/pretreat目录下中间数据的清理功能提取出来，开发专门的清理 */
/*        程序clean_data_pretreat来负责该清理工作                           */
/****************************************************************************/
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
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <dlfcn.h>
#include <link.h>

/* micro definition */
#define      SEMKEY                 878
#define      MAX_MODULE             128
#define      MAX_CHILD_PROCESS      128
#define      MAX_FILENAME           256
#define      MAX_BUFFER             1024
#define      MAX_DATE               9
#define      MAX_TIME               15
#define      SLEEP_TIME             60
#define      MAX_OUTFILE_NUM        10
#define      MAX_FILE_ELEM_NUM      10
#define      MAX_FILE_ELEM          64
#define      DEFAULT_IN_DIR         "./in"
#define      DEFAULT_OUT_DIR        "./out"
#define      DEFAULT_RUN_DIR        "./"
#define      WORK                   "./work"
#define      LOG                    "./log"
#define      CONF                   "./conf"
#define      ERR_LOG_FILE           "./log/pretreat_err"
#define      PREFIX_RUN_LOG_FILE    "./log/pretreat_run"

/* type definition */
typedef int funHandler(char *, char *, int *);  //in_file_name, out_file_name, rec_num

typedef struct {
	int  pid;
	int  sleep_time;
} t_child_process_status;

typedef enum parse_code {
	PARSE_FAIL 	= -1,
	PARSE_MATCH	= 1,
	PARSE_UNMATCH	= 2,
} parse_code_e;

typedef struct {
        int            isload;         //       0 no     , 1 yes
        funHandler   * module_fun;     //return 0 success, 1 fail
        void         * handler_dlopen;
} t_module;

/* global variant definition */
int          curr_process_number    = 0;
char *       progname               = NULL;
char *       file_in_dir            = NULL;
char *       file_out_dir           = NULL;
char *       run_dir                = NULL;
int          debug                  = 0;
int          parallel_child_process = 1;
t_module     module[MAX_MODULE];
int          g_term_flag            = 0;

/* funtion definition */
static void         usage(int status);
static int          get_time(char * par);
static int          err_log(const char * format, ...);
static void         daemon_start(void);
static void         P(void);
static void         V(void);
static parse_code_e pre_suf_check(const char * name, const char * prefix, const char * suffix);
static parse_code_e get_orig_file_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_file_name);
static int          commit_file(const char * in_file_name, const char * out_file_name, const char * begin_time, const char * end_time, long in_file_size, long rec_num);
static int          pretreat(const char * in_file_name);
static parse_code_e get_in_file_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_file_name);
static void         process_pretreat(int current_number);
static int          clear_dir_file(const char * dir_name, const char * prefix, const char * suffix);
static int          load_module(void);
static int          get_relative_module_no(const char * in_file_name, int * module_no);
static int          valid_in_file_name(const char * in_file_name);
static int          valid_out_file_name_and_get_folder_name(const char * out_file_name, char * ret_folder_name);
static int          get_ne_name(const char * original_file_name, char * ret_ne_name);
static void         sigint_handler(int signum);

/*
 *  Display the syntax for starting this program.
 */
static void usage(int status)
{
    FILE *output = status?stderr:stdout;
    
    fprintf(output, "Usage: %s [-i file from path] [-o file commit path] [-r run path] [-p parallel child number] [-d]\n", progname);
    fprintf(output, "\nOptions:\n");
    fprintf(output, "        -i changes the file from directory to that specified in path, default is ./in\n");
    fprintf(output, "        -o changes the file commit directory to that specified in path, default is ./out\n");
    fprintf(output, "        -r changes the run directory to that specified in path, default is ./\n");
    fprintf(output, "        -p changes the parallel child number, default is 1\n");
    fprintf(output, "        -d         debug flag\n");

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
	
	char   tmp[MAX_FILENAME];

	va_list ap;

	if(debug)
	{
  		va_start(ap, format);
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
	fprintf(fp, "------------------------------------------------------------\n");
	fprintf(fp, "%s", asctime(systime));

	va_start(ap, format);
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

     if(getppid() == 1 ) {  return; }

     signal(SIGTTOU, SIG_IGN);
     signal(SIGTTIN, SIG_IGN);
     signal(SIGTSTP, SIG_IGN);
     signal(SIGUSR1, SIG_IGN);

     if(setpgrp() == -1)
            err_log("daemon_start: can't change process group\n");
     signal(SIGHUP, SIG_IGN);

     
     if((childpid = fork())<0) {
            err_log("daemon_start: fork error\n");
            exit(1);
     }
     else if(childpid > 0)
             exit(0);
}

static void P(void)
{
	int    sem_id;
    struct sembuf sem_buff;
	
	sem_id = semget(SEMKEY, 1, 0);
	if(sem_id == -1)
	{
		err_log("P: semget SEMKEY %d fail\n", SEMKEY);
		exit(1);
	}
    /*-----------------------------------------------*/
    /*  semphore p oprate                            */
    /*-----------------------------------------------*/
	sem_buff.sem_num = 0;
	sem_buff.sem_op  = -1;
	sem_buff.sem_flg = SEM_UNDO;
	
	while(semop(sem_id, &sem_buff, 1) == -1)
	{
		if(errno == EINTR || errno == EAGAIN)
		{
			err_log("P: semop SEMKEY %d EINTR or EAGAIN errno=%d\n", SEMKEY, errno);
		}
		else
		{
			err_log("P: semop SEMKEY %d fail errno=%d\n", SEMKEY, errno);
			exit(1);
		}
	}
}

static void V(void)
{
	int    sem_id;
    struct sembuf sem_buff;
	
	sem_id = semget(SEMKEY, 1, 0);
	if(sem_id == -1)
	{
		err_log("V: semget SEMKEY %d fail\n", SEMKEY);
		exit(1);
	}
    /*-----------------------------------------------*/
    /*  semphore v oprate                            */
    /*-----------------------------------------------*/
	sem_buff.sem_num = 0;
	sem_buff.sem_op  = 1;
	sem_buff.sem_flg = SEM_UNDO;

	while(semop(sem_id, &sem_buff, 1) == -1)
	{
		if(errno == EINTR || errno == EAGAIN)
		{
			err_log("V: semop SEMKEY %d EINTR or EAGAIN errno=%d\n", SEMKEY, errno);
		}
		else
		{
			err_log("V: semop SEMKEY %d fail errno=%d\n", SEMKEY, errno);
			exit(1);
		}
	}
}

int main(int argc, char * argv[])
{
    int        ret = 0;
	int        argval;
	struct     stat stat_buff;
	int        i;
	pid_t      childpid;
	pid_t      r_waitpid;
	int        sem_id;
	int        sem_in;
    t_child_process_status child_process_status[MAX_CHILD_PROCESS];

    /* register the signal SIGINT handler */
    if(signal(SIGINT, &sigint_handler) == SIG_ERR)
    {
        err_log("main: signal register fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    /* 获取程序名称 */
    if ((progname = strrchr(argv[0], '/')) == NULL)
    {
        progname = argv[0];
    }
    else
    {
        progname++;
    }

 	/* 处理参数 */
    while ((argval = getopt(argc, argv, "o:i:r:p:d")) != EOF) 
    {
        switch(argval) {
            case 'i':
                file_in_dir = strdup(optarg);
                break;	
            case 'o':
                file_out_dir = strdup(optarg);
                break;	
            case 'r':
                run_dir = strdup(optarg);
                break;	
            case 'p':
                parallel_child_process = atoi(optarg);
                break;
            case 'd':
                debug = 1;
                break;	
            default:
                usage(1);
                break;
        }
	}

    /* 处理默认值 */
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

    if(file_out_dir == NULL)
    {
        file_out_dir = (char *)malloc(MAX_FILENAME);
        if(file_out_dir == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(file_out_dir, DEFAULT_OUT_DIR);
    }

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

	/* 切换到运行目录 */
	if(chdir(run_dir) == -1)
	{
		err_log("main: chdir to %s fail\n", run_dir);
        ret = 1;
        goto Exit_Pro;
	}

	/* 检查临时工作目录 */
	if(stat(WORK, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", WORK);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", WORK);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* 检查日志目录 */
	if(stat(LOG, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", LOG);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", LOG);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* 检查配置目录 */
	if(stat(CONF, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", CONF);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", CONF);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* 检查输出文件目录 */
	if(stat(file_out_dir, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", file_out_dir);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", file_out_dir);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* 检查输入文件目录 */
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
	
	/* 校验子进程设置数目 */
	if(parallel_child_process <= 0)
    {
        parallel_child_process = 1;
    }
	if(parallel_child_process > MAX_CHILD_PROCESS) 
    {
        parallel_child_process = MAX_CHILD_PROCESS;
    }
	
	if(debug)
	{
		fprintf(stdout, "main: file_out_dir:%s#\n", file_out_dir);
		fprintf(stdout, "main: file_in_dir:%s#\n", file_in_dir);
		fprintf(stdout, "main: run_dir:%s#\n", run_dir);
		fprintf(stdout, "main: parallel_child_process:%d#\n", parallel_child_process);
	}

	/* 清理work目录下文件 */
    if(clear_dir_file(WORK, NULL, NULL) != 0)
    {
		err_log("main: clear %s fail\n", WORK);
        ret = 1;
        goto Exit_Pro;
    }

	/* 初始化信号量对象 */
	sem_id = semget(SEMKEY, 1, 0666|IPC_CREAT);
	if(sem_id == -1) {
		err_log("main: semget SEMKEY %d fail\n", SEMKEY);
        ret = 1;
        goto Exit_Pro;
	}
	sem_in = 1;

	if(semctl(sem_id, 0, SETVAL, &sem_in) == -1) {
		err_log("main: semctl SEMKEY %d fail\n", SEMKEY);
        ret = 1;
        goto Exit_Pro;
	}

	/* 装载预处理模块 */
    if(load_module() != 0)
    {
		err_log("main: load module fail\n");
        ret = 1;
        goto Exit_Pro;
    }
	        
	/* 初始化 子进程结构数组 */
    for(i = 0; i<MAX_CHILD_PROCESS; i++)
    {
        memset(&child_process_status[i], 0, sizeof(t_child_process_status));
    }
    
    if(debug == 0)
    {
        daemon_start();
    }
    
	/* 循环开启多进程进行解析处理 */
	while(1)
	{
		/* 创建子进程 */
		for(i = 0; i<parallel_child_process; i++)
		{
			if(child_process_status[i].pid == 0 && child_process_status[i].sleep_time <= 0)
			{
				if((childpid = fork())<0) /* fork error */
				{
                    err_log("main: fork error\n");
                    ret = 1;
                    goto Exit_Pro;
                }
                else if(childpid > 0)     /* parent process  */ 
                {
                    child_process_status[i].pid        = childpid;
                    child_process_status[i].sleep_time = SLEEP_TIME;
                }
                else if(childpid == 0)    /* child process */
                {
                    process_pretreat(i+1);
                    exit(0);
                }
            }
		}
	
		/* 回收子进程 */
		for(i = 0; i<parallel_child_process; i++)
		{
			if(child_process_status[i].pid > 0)
			{
				r_waitpid = waitpid(child_process_status[i].pid, NULL, WNOHANG);
				if(r_waitpid > 0)
				{
					child_process_status[i].pid = 0;
				}
				if(r_waitpid<0)
				{
					err_log("main: waitpid fail\n");
                    ret = 1;
                    goto Exit_Pro;
				}
			}
		}
				
        /* 如果设置了终止标志，则退出处理循环 */
        if(g_term_flag)
        {
            /* 在退出之前把所有的采集子进程杀死 */
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
		
		/* 每秒递减sleep_time */
		for(i = 0; i<parallel_child_process; i++)
		{
            if(child_process_status[i].pid == 0 && child_process_status[i].sleep_time > 0)
            {
                child_process_status[i].sleep_time--;
            }
		}
	}
    	
Exit_Pro:
    if(file_in_dir != NULL)
    {
        free(file_in_dir);
        file_in_dir = NULL;
    }

    if(file_out_dir != NULL)
    {
        free(file_out_dir);
        file_out_dir = NULL;
    }

    if(run_dir != NULL)
    {
        free(run_dir);
        run_dir = NULL;
    }

	for( i=0; i<MAX_MODULE; i++ )
	{
        if( module[i].handler_dlopen != NULL )
        {
            dlclose(module[i].handler_dlopen);
        }
	}

	return ret;
}

/*
 *  pre_suf_check()
 */
static parse_code_e pre_suf_check(const char * name, const char * prefix, const char * suffix)
{
	if(name == NULL)
		return PARSE_UNMATCH;
		
	/*前缀匹配检查 */
	if(prefix != NULL)
	{
		if(strstr(name, prefix) != name)
			return PARSE_UNMATCH;
	}
	/* 后缀匹配检查 */
	if(suffix != NULL)
	{
		if(strlen(name)<strlen(suffix))
			return PARSE_UNMATCH;
		if(strcmp( (name+strlen(name)-strlen(suffix)), suffix) != 0 )
			return PARSE_UNMATCH;
	}
	
	return PARSE_MATCH;
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

	if(dir_name == NULL)
    {
		err_log("get_orig_file_name: dir name is null\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
    }

    pdir = opendir(dir_name);
	if(pdir == NULL)
	{
		err_log("get_orig_file_name: opendir fail\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
	}

	while( (pdirent = readdir(pdir)) != NULL )
	{
		/* 前后缀匹配检查 */
		if( pre_suf_check(pdirent->d_name, prefix, suffix) != PARSE_MATCH )
			continue;

        /* 文件检查 */
        strcpy(tmp_file_name, dir_name);
        strcat(tmp_file_name, "/");
        strcat(tmp_file_name, pdirent->d_name);

		if(stat(tmp_file_name, &stat_buff) == -1)
		{
			err_log("get_orig_file_name: stat %s fail\n", pdirent->d_name);
			ret = PARSE_FAIL;
			break;
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
 *commit_file()
 *              return 0 success, 1 fail
 *
 */
static int commit_file(const char * in_file_name, const char * out_file_name, const char * begin_time, const char * end_time, long in_file_size, long rec_num)
{
	int   ret = 0;
	FILE  * fp = NULL;
	char  run_log_time[MAX_TIME];
	char  tmp_file_name[MAX_FILENAME];
	char  tmp_file_name_2[MAX_FILENAME];
	char  tmp_file_name_3[MAX_FILENAME];
    char  work_dir[MAX_FILENAME];
    char  out_files[MAX_OUTFILE_NUM][MAX_FILENAME];
    const char  *ptr_front = NULL, *ptr_back = NULL;
    char  ne_name[MAX_FILE_ELEM];
    int   count, i;

    /* 初始化 */
    memset(work_dir, 0, sizeof(work_dir));
    memset(tmp_file_name, 0, sizeof(tmp_file_name));
    memset(tmp_file_name_2, 0, sizeof(tmp_file_name_2));
    memset(out_files, 0, sizeof(out_files));

    /* 获取工作目录 */
    ptr_front = out_file_name;
    ptr_back = strchr(ptr_front, ':');
    if(ptr_back == NULL)
    {
        err_log("commit_file: out file string %s incorrect\n", out_file_name);
        ret = 1;
        goto Exit_Pro;
    }
    strncpy(work_dir, ptr_front, ptr_back - ptr_front);

    /* 解析所有CSV文件名 */
    ptr_front = ++ptr_back;
    count = 0;
    while((ptr_back = strchr(ptr_front, ';')) != NULL) 
    {
        strncpy(out_files[count++], ptr_front, ptr_back - ptr_front);
        ptr_front = ++ptr_back;
    }

    if(strlen(ptr_front) > 0)
    {
        strcpy(out_files[count++], ptr_front);
    }

    /* 循环处理每一个输出的CSV文件 */
    for(i = 0; i < count; ++i)
    {
		// 获取提交目录
		if (0 != valid_out_file_name_and_get_folder_name(out_files[i],tmp_file_name_3))
		{
			err_log("commit_file: valid_out_file_name_and_get_folder_name: %s to %s\n", out_files[i],tmp_file_name_3);
			ret = 1;
			goto Exit_Pro;
		}
        /* 复制工作目录文件至提交目录 */
        sprintf(tmp_file_name, "%s/%s", work_dir, out_files[i]);
        sprintf(tmp_file_name_2, "%s/%s", tmp_file_name_3, out_files[i]);

        if(access(tmp_file_name_2, F_OK) == -1)
        {
            if(link(tmp_file_name, tmp_file_name_2) != 0)
            {
                err_log("commit_file: link %s to %s fail\n", tmp_file_name, tmp_file_name_2);
                ret = 1;
                goto Exit_Pro;
            }
        }
        else
        {
            err_log("commit_file: target file %s exist\n", tmp_file_name_2);
        }

        /* 删除工作目录下相应的临时文件 */
		if ( unlink(tmp_file_name) != 0 )
		{
			err_log("commit_file: unlink file %s fail\n", tmp_file_name);
			ret = 1;
			goto Exit_Pro;
		}
    }

    /* 删除输入文件in_file_name */
    sprintf(tmp_file_name, "%s/%s", file_in_dir, in_file_name);
    if ( unlink(tmp_file_name) != 0 )
    {
        err_log("commit_file: unlink file %s fail\n", tmp_file_name);
        ret = 1;
        goto Exit_Pro;
    }

    /* 取得网元名称 */
    memset(ne_name, 0, sizeof(ne_name));
    if(get_ne_name(in_file_name, ne_name) != 0)
    {
        err_log("commit_file: get ne name fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    /* 记录处理信息到pretreat_run.YYYYMMDD文件 */
	get_time(run_log_time);
	run_log_time[8] = '\0';
	
	sprintf(tmp_file_name, "%s.%s.%s", PREFIX_RUN_LOG_FILE, ne_name, run_log_time);

    P();		
	fp = fopen(tmp_file_name, "a+");
	if(fp == NULL)
	{
        V();
		err_log("commit_file: fopen %s fail\n", tmp_file_name);
		ret = 1;
		goto Exit_Pro;
	}
	fprintf(fp, "%s %ld %s %s %ld\n", in_file_name, in_file_size, begin_time, end_time, rec_num);
	fclose(fp);
	fp = NULL;	
    V();  

Exit_Pro:
	return ret;
}

/*
 *pretreat()
 *           return 0 success, 1 fail
 *
 */
static int pretreat(const char * in_file_name)
{
	int    ret = 0;
	struct stat stat_buff;
	char   tmp_in_file_name[MAX_FILENAME];
	char   tmp_out_file_name[MAX_BUFFER];
	long   in_file_size;
	int    rec_num;
	char   begin_time[MAX_TIME];
	char   end_time[MAX_TIME];
	int    num_module;

    /* 验证输入文件是否符合命名规范 */
    if(valid_in_file_name(in_file_name) != 0)
    {
		err_log("pretreat: bill file name format incorrect\n", in_file_name);
		ret = 1;
		goto Exit_Pro;
    }

	/* 取文件对应解析模块号 */
    if( get_relative_module_no(in_file_name, &num_module) != 0 )
    {
		err_log("pretreat: get %s relative module no fail\n", in_file_name);
		ret = 1;
		goto Exit_Pro;
    }

	if( num_module < 1 )
	{
		err_log("pretreat: file %s, pretreat.conf no this point\n", in_file_name);
		ret = 1;
		goto Exit_Pro;
	}
	
    /* 判断对应的解析模块是否加载 */
	if( module[num_module-1].isload != 1 )
	{
		err_log("pretreat: file %s, module %d not load\n", in_file_name, num_module);
		ret = 1;
		goto Exit_Pro;
	}

	
    /* 设置解析模块的调用参数：输入文件,输出文件路径,返回话单数目 */
	sprintf(tmp_in_file_name, "%s/%s", file_in_dir, in_file_name);
	strcpy(tmp_out_file_name, WORK);
	rec_num = 0;

	get_time(begin_time);

	/* 调用对应模块的处理函数 */
	if( module[num_module-1].module_fun(tmp_in_file_name, tmp_out_file_name, &rec_num) != 0 )
	{
		err_log("pretreat: file %s, module %d module_fun fail\n", in_file_name, num_module);
		ret = 1;
		goto Exit_Pro;
	}

	get_time(end_time);

	/* 取输入文件大小 */
	if(stat(tmp_in_file_name, &stat_buff) == -1)
	{
		err_log("pretreat: stat %s fail\n", tmp_in_file_name);
		ret = 1;
		goto Exit_Pro;
	}
	in_file_size = stat_buff.st_size;

    /* 提交文件 */
	if( commit_file(in_file_name, tmp_out_file_name, begin_time, end_time, in_file_size, rec_num) != 0 )
	{
		err_log("pretreat: commit_file %s fail\n", in_file_name);
		ret = 1;
		goto Exit_Pro;
	}
	
Exit_Pro:
	return ret;
}

/*
 *  get_in_file_name()
 */
static parse_code_e get_in_file_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_file_name)
{
	int              ret;
	DIR *            pdir = NULL;
	struct dirent *	 pdirent;
	struct stat stat_buff;
	char             tmp_file_name[MAX_FILENAME];
    char             *ptr = NULL;
    char             tmp_file_name_2[MAX_FILENAME];
	
	ret = PARSE_UNMATCH;

	if(dir_name == NULL)
	{
		err_log("get_in_file_name: dir_name is NULL\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
	}
	
    /* 
     * 因为打开目录后不只是进行了查询操作，还有更改文件名称的操作，
     * 所以为了防止多个进程同时对同一个文件进行竞争处理，特设置
     * 了多进程间互斥 
     */
    P();

	pdir = opendir(dir_name);
	if(pdir == NULL)
	{
		err_log("get_in_file_name: opendir fail\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
	}

	while( (pdirent=readdir(pdir)) != NULL )
	{
		/* 前后缀匹配检查 */
		if( pre_suf_check(pdirent->d_name, prefix, suffix) != PARSE_MATCH )
			continue;

        ptr = pdirent->d_name;
        ptr++;

		/* 除去前面特殊标志@，要求文件名前6个字节是数字 */
		if(strlen(ptr)<6)
			continue;
		if(!isdigit(ptr[0]))
			continue;
		if(!isdigit(ptr[1]))
			continue;
		if(!isdigit(ptr[2]))
			continue;
		if(!isdigit(ptr[3]))
			continue;
		if(!isdigit(ptr[4]))
			continue;
		if(!isdigit(ptr[5]))
			continue;

        /* 文件检查 */
        if(dir_name != NULL)
        {
            strcpy(tmp_file_name, dir_name);
            strcat(tmp_file_name, "/");
            strcat(tmp_file_name, pdirent->d_name);
        }
        else
            strcpy(tmp_file_name, pdirent->d_name);
            
		if(stat(tmp_file_name, &stat_buff) == -1)
		{
			err_log("get_in_file_name: stat %s fail\n", pdirent->d_name);
			ret = PARSE_FAIL;
			break;
		}
        if(S_ISREG(stat_buff.st_mode))
        {
            sprintf(tmp_file_name_2, "%s/%s", dir_name, ptr);
            if(rename(tmp_file_name, tmp_file_name_2) != 0)
            {
                err_log("get_in_file_name: rename fail\n", pdirent->d_name);
                ret = PARSE_FAIL;
                break;
            }

            strcpy(ret_file_name, ptr);

            ret = PARSE_MATCH;
            break;	
        }	
	}
    closedir(pdir);
    pdir = NULL;
    V();

Exit_Pro:
	if(pdir != NULL)
    {
        closedir(pdir);
    }
	return ret;
}

/*
 *process_pretreat()
 *
 *
 */
static void process_pretreat(int current_number)
{
	char    file_name[MAX_FILENAME];

	/* 设置全局子进程编号 */
	curr_process_number = current_number;

	memset(file_name, 0, sizeof(file_name));
	while(get_in_file_name(file_in_dir, "@", NULL, file_name) == PARSE_MATCH)
	{
		/* 文件预处理 */
		if(pretreat(file_name) != 0)
		{
			err_log("process_pretreat: pretreat fail\n");
			break;
		}
		
		memset(file_name, 0, sizeof(file_name));
    }
	return;
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

static int load_module(void)
{
    int        ret = 0;
    FILE       *fp = NULL;
	char       file_name[MAX_FILENAME];
	char	   buff[MAX_BUFFER];
	char       content[3][MAX_FILENAME];
    void       *handler_dlopen = NULL;
    char       *dlError        = NULL;
	int        r;

	memset(module, 0, sizeof(module));

	sprintf(file_name, "%s/%s", CONF, "module.conf");

	fp = fopen(file_name, "r");
	if(fp == NULL)
	{
		err_log("load_module: fopen %s fail\n", file_name);
        ret = 1;
        goto Exit_Pro;
	}

	r = 0;

	while(1)
	{
	    memset(buff, 0, sizeof(buff));
	    memset(content, 0, sizeof(content));

        if(fgets(buff, sizeof(buff), fp) == NULL)
        {
            break;
        }
       		
        r++;
	    
	    if( r > MAX_MODULE )
	    {
	    	err_log("load_module: module.conf line>%d\n", MAX_MODULE);
            ret = 1;
            goto Exit_Pro;
	    }
	    
        if(buff[0] != '#')
        {     	
            if( sscanf(buff, "%s%s%s", content[0], content[1], content[2]) != 3 )
            {
                err_log("load_module: module.conf line %d incorrect\n", r);
                ret = 1;
                goto Exit_Pro;
            }
            
            if( atoi(content[0]) != r )
            {
                err_log("load_module: module.conf line %d incorrect\n", r);
                ret = 1;
                goto Exit_Pro;
            }
            
            /* 装载动态库 */
            handler_dlopen = dlopen(content[1], RTLD_LAZY);
            if( handler_dlopen == NULL)
            {
                dlError = dlerror();
                err_log("load_module: dlopen %s fail, %s\n", content[1], dlError);
                ret = 1;
                goto Exit_Pro;
            }
        
            module[r-1].handler_dlopen = handler_dlopen;
            
            module[r-1].module_fun = dlsym( handler_dlopen, content[2]);
            if( module[r-1].module_fun == NULL )
            {
                dlError = dlerror();
                err_log("load_module: dlsym module:%s, fun:%s fail, %s\n", content[1], content[2], dlError);
                ret = 1;
                goto Exit_Pro;
            }
            
            module[r-1].isload = 1;
        }
	}
	
Exit_Pro:
    if(fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
    return ret;
}

static int get_relative_module_no(const char * in_file_name, int * module_no)
{
    int    ret = 0;
	int    point;
	char   tmp[7];
	int    r, len;
	char   buff[MAX_BUFFER];
	char   content[2][MAX_FILENAME];
    char   tmp_file_name[MAX_FILENAME];
	FILE   *fp = NULL;

    (*module_no) = 0;
	r = 0;
	
	memset(tmp, 0, sizeof(tmp));
	strncpy(tmp, in_file_name, 6);
	point = atoi(tmp);
	
	sprintf(tmp_file_name, "%s/pretreat.conf", CONF);
	fp = fopen(tmp_file_name, "r");
	if(fp == NULL)
	{
		err_log("pretreat: fopen %s fail\n", tmp_file_name);
		ret = 1;
		goto Exit_Pro;
	}
	
	while(1)
	{
	    memset(buff, 0, sizeof(buff));
	    memset(content, 0, sizeof(content));

        if(fgets(buff, sizeof(buff), fp) == NULL)
        {
            break;
        }
        
        r++;

        if(buff[0] != '#')
        {     	
            if( sscanf(buff, "%s%s", content[0], content[1] ) != 2 )
            {
                err_log("pretreat: pretreat.conf line %d incorrect\n", r);
                ret = 1;
                goto Exit_Pro;
            }
            
            len = strlen(content[0]);
            if(len != 8 || content[0][0] != '<' || content[0][len-1] != '>')
            {
                err_log("pretreat: pretreat.conf line %d incorrect\n", r);
                ret = 1;
                goto Exit_Pro;
            }
                
            content[0][len-1] = '\0';
            if(atoi(&content[0][1]) != r)
            {
                err_log("pretreat: pretreat.conf line %d incorrect\n", r);
                ret = 1;
                goto Exit_Pro;
            }
            
            if( r == point )
            {
                (*module_no) = atoi(content[1]);
                break;
            }
        }
	}
	
Exit_Pro:
	if(fp != NULL)
    {
		fclose(fp);
        fp = NULL;
    }
    return ret;
}

static int valid_in_file_name(const char * in_file_name)
{
    char            elems[MAX_FILE_ELEM_NUM][MAX_FILE_ELEM];
    const char      *ptr_front = NULL, *ptr_back = NULL;
    int             count, len, i;

    /* 初始化缓存数组 */
    for(i = 0; i < MAX_FILE_ELEM_NUM; ++i)
    {
        memset(elems[i], 0, sizeof(elems[i]));
    }

    /* 分解输入文件的组成元素 */
    ptr_front = strrchr(in_file_name, '/');
    if(ptr_front == NULL)
    {
        ptr_front = in_file_name;
    }
    else 
    {
        ptr_front++;
    }

    count = 0;
    while((ptr_back = strchr(ptr_front, '_')) != NULL)
    {
        strncpy(elems[count++], ptr_front, ptr_back - ptr_front);
        ptr_front = ++ptr_back;
    }
    
    if(strlen(ptr_front) > 0)
    {
        strcpy(elems[count++], ptr_front);
    }
 
    /* 话单文件以'_'间隔的元素最少有5个 */
    if(count < 5)
    {
		err_log("valid_in_file_name: in file name format incorrect\n");
        return 1;
    }

    /* 
     * 验证分解出来的前四个各个元素 
     * elems[0]: 采集点编号 
     * elems[1]: 厂商编号(hw,nsn)
     * elems[2]: 网元名称
     * elems[3]: 话单生成时间(YYYYMMDDhhmmss)
     * */
    if(strlen(elems[0]) == 0 
       || strlen(elems[1]) == 0
       || strlen(elems[2]) == 0 
       || strlen(elems[3]) == 0
       || strlen(elems[4]) == 0)
    {
		err_log("valid_in_file_name: in file name format incorrect\n");
        return 1;
    } 

    /* 第一个元素elems[0]是采集点编号：6位数字编号
     * 因为在之前定位要解析文件时已经经过验证，所以
     * 这里不需要再验证
     */

    /* 第二个元素elems[1]是厂家缩写，目前只可能是hw和nsn */
    if((strcmp(elems[1], "hw") != 0) && (strcmp(elems[1], "nsn") != 0))
    {
		err_log("valid_in_file_name: in file name format incorrect\n");
        return 1;
    }
    
    /* 
     * 第三个元素elems[2]是网元名称，所有的网元名称也是确定的，但为了
     * 执行效率，目前暂不验证网元名称 
     */

    /* 第四个元素elems[3]是日期格式：YYYYMMDDhhsssmm */
    len = strlen(elems[3]);
    if(len != 14)
    {
		err_log("valid_in_file_name: in file name format incorrect\n");
        return 1;
    }

    for(i = 0; i < len; ++i)
    {
        if(!isdigit(elems[3][i]))
        {
            err_log("valid_in_file_name: in file name format incorrect\n");
            return 1;
        }
    }

    return 0;
}

static int valid_out_file_name_and_get_folder_name(const char * out_file_name, char * ret_folder_name)
{ 
    char            elems[MAX_FILE_ELEM_NUM][MAX_FILE_ELEM];
    const char      *ptr_front = NULL, *ptr_back = NULL;
    char            table_date[MAX_DATE];
	char			szFolder[MAX_FILENAME];
    int             count, len, i;

    /* 初始化缓存数组 */
    for(i = 0; i < MAX_FILE_ELEM_NUM; ++i)
    {
        memset(elems[i], 0, sizeof(elems[i]));
    }

    /* 分解输入文件的标题元素 */
    ptr_front = strrchr(out_file_name, '/');
    if(ptr_front == NULL)
    {
        ptr_front = out_file_name;
    }
    else 
    {
        ptr_front++;
    }

    count = 0;
    while((ptr_back = strchr(ptr_front, '_')) != NULL)
    {
        strncpy(elems[count++], ptr_front, ptr_back - ptr_front);
        ptr_front = ++ptr_back;
    }
    
    if(strlen(ptr_front) > 0)
    {
        strcpy(elems[count++], ptr_front);
    }
 
    /* CSV文件以'_'间隔的元素最少有6个 */
    if(count < 6)
    {
		err_log("valid_out_file_name_and_get_folder_name: file name format incorrect: %s\n", out_file_name);
        return 1;
    }
 
    /* 
     * 验证分解出来的前五个元素 
     * elems[0]: 采集点编号 
     * elems[1]: 厂商编号(hw,nsn)
     * elems[2]: 网元名称
     * elems[3]: 话单类型
     * elems[4]: 话单生成时间(YYYYMMDDhhmmss)
     * */
    if(strlen(elems[0]) == 0 
       || strlen(elems[1]) == 0
       || strlen(elems[2]) == 0 
       || strlen(elems[3]) == 0
       || strlen(elems[4]) == 0
       || strlen(elems[5]) == 0)
    {
		err_log("valid_out_file_name_and_get_folder_name: file name format incorrect: %s\n", out_file_name);
        return 1;
    } 

    /* 第一个元素elems[0]是采集点编号：6位数字编号     */

    /* 第二个元素elems[1]是厂家缩写，目前只可能是hw和nsn */
    if((strcmp(elems[1], "hw") != 0) && (strcmp(elems[1], "nsn") != 0))
    {
		err_log("valid_out_file_name_and_get_folder_name: file name format incorrect: %s\n", out_file_name);
        return 1;
    }
    
    /* 
     * 第三个元素elems[2]是网元名称，所有的网元名称也是确定的，但为了
     * 执行效率，目前暂不验证网元名称 
     */

    /* 
     * 第四个元素elems[3]是话单类型，要解析的话单类型也是确定的，但为了
     * 执行效率，目前暂不验证话单类型 
     */

    /* 第五个元素elems[4]是日期格式：YYYYMMDDhhsssmm */
    len = strlen(elems[4]);
    if(len != 14)
    {
		err_log("valid_out_file_name_and_get_folder_name: file name format incorrect: %s\n", out_file_name);
        return 1;
    }

    for(i = 0; i < len; ++i)
    {
        if(!isdigit(elems[4][i]))
        {
            err_log("valid_out_file_name_and_get_folder_name: file name format incorrect: %s\n", out_file_name);
            return 1;
        }
    }

    /* 把需要的时间提取出来 
     * 现在elems[4]中存放的时间格式是：YYYYMMDDhhmmss
     * 把其中的MMDD提取出来作为表名的一部分
     */
    memset(table_date, 0, sizeof(table_date));
    strncpy(table_date, &(elems[4][4]), 4);

    /* 生成需要的文件夹名称 */
    if(strcmp(elems[1], "hw") == 0)
    {
        if(strcmp(elems[3], "bi") == 0)
        {
            /* 表名： op_bill_厂家_端局_时间(MMDD) */
            sprintf(szFolder, "op_bill_%s_%s_%s", elems[1], elems[2], table_date);
        }
        else
        {
            /* 表名： op_bill_厂家_话单类型_端局_时间(MMDD) */
            sprintf(szFolder, "op_bill_%s_%s_%s_%s", elems[1], elems[3], elems[2], table_date);
        }
    }
    else if(strcmp(elems[1], "nsn") == 0)
    {
        /* 表名： op_bill_厂家_话单类型_端局_时间(MMDD) */
        sprintf(szFolder, "op_bill_%s_%s_%s_%s", elems[1], elems[3], elems[2], table_date);
    }
    else
    {
        err_log("valid_out_file_name_and_get_folder_name: file name format incorrect: %s\n", out_file_name);
        return 1;
    }

	//确认目录是否存在,不存在则创建
	sprintf(ret_folder_name,"%s/%s",file_out_dir, szFolder);
	P();
	if(access(ret_folder_name,F_OK) == -1)
	{
		if(mkdir(ret_folder_name,0755)!=0)
		{
			err_log("valid_out_file_name_and_get_folder_name: mkdir incorrect: %s\n", ret_folder_name);
			V();
			return 1;
		}
	}
	V();

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

    /* 采集点编号 */
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }

    /* 厂家 */
    ptr_front = ++ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }

    /* 网元名称 */
    ptr_front = ++ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }

    /* 获取网元名称 */
    strncpy(ret_ne_name, ptr_front, ptr_back - ptr_front);

    return 0;
}

static void sigint_handler(int signum)
{
    if(signum == SIGINT)
    {
        g_term_flag = 1;
    }
}

