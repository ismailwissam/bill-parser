/***************************************************************************/
/* clean_data_pretreat.c                                                   */
/*                                                                         */ 
/* 话单解析数据预处理目录清理程序v1.0                                      */
/*                                                                         */
/* created by wangxiaohui at 2010.6                                        */
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

/* micor definition */
#define MAX_FILENAME            256
#define MAX_DATE                15
#define DEFAULT_PRETREAT_DIR    "./data/pretreat"
#define DEFAULT_RUN_DIR         "./"
#define LOG_DIR                 "./log"
#define ERR_LOG_FILE            "./log/clean_data_pretreat_err"
#define PREFIX_RUN_LOG_FILE     "./log/clean_data_pretreat_run"
#define TRUE                    1
#define FALSE                   0
#define CLEAN_STATUS_SUCC       "clean_success"
#define CLEAN_STATUS_FAIL       "clean_failed"
#define CLEAN_STATUS_NOEMPTY    "dir_no_empty"

/* data type definition */
typedef unsigned int boolean;

typedef enum parse_code {
	PARSE_FAIL = -1,
	PARSE_MATCH	= 1,
	PARSE_UNMATCH = 2,
} parse_code_e;

/* global variant defintion */
char *       progname               = NULL;
int          save_day               = 3;
int          debug                  = 0;

/* static functions definitions */
static void usage(int status);
static int get_time(char * par);
static int err_log(const char * format, ...);
static int commit_log(const char * dir_name, const char * clean_time, const char * clean_status);
static parse_code_e pre_suf_check(const char * name, const char * prefix, const char * suffix);
static int clean_data_pretreat(const char * pretreat_dir);
static int check_dir_empty(const char * dir_name, boolean * is_empty);

/*
 * main()
 */
int main(int argc, char * argv[])
{
    int     ret = 0;
	int     argval;
	struct  stat stat_buff;
    char *  run_dir = NULL;
    char *  pretreat_dir = NULL;
	
    /* 获取程序名称 */
    if((progname = strrchr(argv[0],  '/')) == NULL)
    {
        progname = argv[0];
    }
    else
    {
        progname++;
    }
	
 	/*  处理参数  */
    while((argval = getopt(argc,  argv,  "r:p:s:d")) != EOF) 
    {
        switch(argval) 
        {
            case 'r':
                run_dir = strdup(optarg);
                break;	
            case 'p':
                pretreat_dir = strdup(optarg);
                break;
            case 's':
                save_day = atoi(optarg);
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

    if(pretreat_dir == NULL)
    {
        pretreat_dir = (char *)malloc(MAX_FILENAME);
        if(pretreat_dir == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(pretreat_dir, DEFAULT_PRETREAT_DIR);
    }

	/* 如果指定了运行目录，切换到运行目录 */
    if(chdir(run_dir) == -1)
    {
        err_log("main: chdir to %s fail\n", run_dir);
        ret = 1;
        goto Exit_Pro;
    }

	/* 检查日志目录 */
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

	/* 检查数据预处理目录 */
	if(stat(pretreat_dir, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", pretreat_dir);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", pretreat_dir);
            ret = 1;
            goto Exit_Pro;
        }
	}

    if(clean_data_pretreat(pretreat_dir) != 0)
    {
        err_log("main: clean data pretreat fail: %s\n", pretreat_dir);
        ret = 1;
        goto Exit_Pro;
    }

Exit_Pro:
    if(run_dir != NULL)
    {
        free(run_dir);
        run_dir = NULL;
    }
    if(pretreat_dir != NULL)
    {
        free(pretreat_dir);
        pretreat_dir = NULL;
    }
   	return ret;
}

/* 
 * clean_data_pretreat()
 * 
 *        clean the data pretreat directory.
 *
 *        return 0: success; return 1: failed
 */
static int clean_data_pretreat(const char * pretreat_dir)
{
    int             ret = 0;
    char            tmp_file_name[MAX_FILENAME];
	DIR *           pdir = NULL;
	struct dirent *	pdirent;
	struct  stat    stat_buff;
    time_t          now_time;
    char            clean_time[MAX_DATE];
    boolean         is_dir_empty;

    pdir = opendir(pretreat_dir);
	if(pdir == NULL)
	{
		err_log("clean_data_pretreat: opendir fail: %s\n", pretreat_dir);
		ret = 1;
		goto Exit_Pro;	
	}

	/* 扫描文件夹下所有文件 */
	while((pdirent = readdir(pdir)) != NULL)
	{
		/* 简单检查,要以op_bill_开头 */
		if(pre_suf_check(pdirent->d_name, "op_bill_", NULL) != PARSE_MATCH)
        {
			continue;
        }

        /* 获取文件属性 */
        sprintf(tmp_file_name, "%s/%s", pretreat_dir, pdirent->d_name);
		if(stat(tmp_file_name, &stat_buff) == -1)
		{
			err_log("clean_data_pretreat: stat %s fail\n", pdirent->d_name);
			ret = 1;
            break;
		}

        /* 处理对象必须是目录 */
        if(!S_ISDIR(stat_buff.st_mode))
        {
            continue;
        }

        /* 获取当前时间 */
        now_time = time(NULL);

        if( ((now_time - stat_buff.st_mtime) / (24 * 60 * 60)) + 1 >= save_day )
        {
            if( check_dir_empty(tmp_file_name, &is_dir_empty) != 0 )
            {
                err_log("clean_data_pretreat: check dir emtpy fail: %s\n", tmp_file_name);
                ret = 1;
                break;
            }

            get_time(clean_time);

            if(is_dir_empty)
            {
                /* 如果超期目录为空，则删除 */
                if(rmdir(tmp_file_name) != 0)
                {
                    err_log("clean_data_pretreat: rmdir %s fail\n", tmp_file_name);

                    if(commit_log(pdirent->d_name, clean_time, CLEAN_STATUS_FAIL) != 0)
                    {
                        err_log("clean_data_pretreat: commit run log fail\n");
                        ret = 1;
                        break;
                    }

                    ret = 1;
                    break;
                }

                if(commit_log(pdirent->d_name, clean_time, CLEAN_STATUS_SUCC) != 0)
                {
                    err_log("clean_data_pretreat: commit run log fail\n");
                    ret = 1;
                    break;
                }
            }
            else
            {
                /* 如果超期目录不为空,则记录运行日志 */
                if(commit_log(pdirent->d_name, clean_time, CLEAN_STATUS_NOEMPTY) != 0)
                {
                    err_log("clean_data_pretreat: commit run log fail\n");
                    ret = 1;
                    break;
                }
            }
        }
	}

Exit_Pro:
    /* 关闭目录 */
    if(pdir != NULL)
    {
        closedir(pdir);
        pdir = NULL;
    }
    return ret;
}

/*
 *  Display the syntax for starting this program.
 */
static void usage(int status)
{
    FILE *output = status?stderr:stdout;
    
    fprintf(output,"Usage: %s [-r run_path] [-p pretreat_path] [-s save_day] [-d]\n",progname);
    fprintf(output,"\nOptions:\n");
    fprintf(output,"        -r set the run path, default is ./\n");
    fprintf(output,"        -p set the pretreat data path, default is ./data/pretreat\n");
    fprintf(output,"        -s set the save day of pretreat data, default is 3\n");
    fprintf(output,"        -d debug flag\n");

    exit(status);
}

/* 
 * get_time()
 *
 *        get the current date time.
 *
 *        return 0: success; return 1: failed
 */
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

/* 
 * err_log()
 *
 *         commit the error log 
 *
 * */
static int err_log(const char * format, ...)
{
	time_t t;
	struct tm *systime;
	FILE *fp;
	va_list ap;

	if(debug)
	{
  		va_start(ap,  format);
  		vprintf(format, ap);
  		va_end(ap);
  		printf("\n");
	}

    fp = fopen(ERR_LOG_FILE, "a+");
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

/*
 *  pre_suf_check()
 *
 *          prefix and suffix match check
 *
 *          return PARSE_MATCH is match; return PARSE_UNMATCH is not match.
 */
static parse_code_e pre_suf_check(const char * name, const char * prefix, const char * suffix)
{
	if(name == NULL)
		return PARSE_UNMATCH;
		
	/*前缀匹配检查*/
	if(prefix !=  NULL)
	{
		if(strstr(name, prefix) != name)
			return PARSE_UNMATCH;
	}
	/*后缀匹配检查*/
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
 * check_dir_empty()
 *
 *         check if the directory is empty.
 *
 *         return 0 success,  1 fail
 */
static int check_dir_empty(const char * dir_name, boolean * is_empty)
{
    int        ret = 0;
    DIR *      pdir = NULL;
    int        count = 0;
    
    (*is_empty) = TRUE;

    pdir = opendir(dir_name);
    if(pdir == NULL)
    {
        err_log("check_dir_empty: opendir fail: %s\n", dir_name);
        ret = 1;
        goto Exit_Pro;
    }

    while(readdir(pdir) != NULL)
    {
        if(++count > 2)
        {
            (*is_empty) = FALSE;
            break;
        }
    }

Exit_Pro:
    if(pdir != NULL)
    {
        closedir(pdir);
        pdir = NULL;
    }
    return ret;
}

/*
 * commit_log()
 *
 *         commit running log. 
 *
 *         return 0 success,  1 fail
 */
static int commit_log(const char * dir_name, const char * clean_time, const char * clean_status)
{
    int   ret = 0;
    FILE  *fp = NULL;
	char  run_log_time[MAX_DATE];
	char  tmp_file_name[MAX_FILENAME];

    /* 取得当前日期 */
	get_time(run_log_time);
	run_log_time[8] = '\0';
	
	sprintf(tmp_file_name, "%s.%s", PREFIX_RUN_LOG_FILE, run_log_time);
	fp = fopen(tmp_file_name, "a+");
	if(fp == NULL)
	{
		err_log("commit_log: fopen %s fail\n", tmp_file_name);
		ret = 1;
		goto Exit_Pro;
	}

	fprintf(fp, "%s %s %s\n", dir_name, clean_time, clean_status);

Exit_Pro:
	if(fp != NULL)
    {
        fclose(fp);
    }
	return ret;    
}

