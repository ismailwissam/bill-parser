/**************************************************************************/
/* collect.c                                                              */
/*                                                                        */ 
/* ftp采集程序                                                            */ 
/*                                                                        */
/* v1.0 2005.05 haojianting                                               */
/*                                                                        */
/* v1.1 2010.04 刘洋                                                      */
/* ChangeLog:                                                             */
/*     添加大量Debug输出,增加代码可调性                                   */
/*     程序结构优化                                                       */
/*     增加FTP端口设置,使可连接非默认端口FTP                              */
/*     输出文件名:                                                        */
/*     采集点_厂商名_设备名_时间串_原名(.dat)                             */
/*     重写下载逻辑,使其加快下载速度                                      */
/*                                                                        */
/* v1.2 2010.6 wangxiaohui                                                */
/* ChangeLog:                                                             */
/*     代码重构，修正程序Bug                                              */
/*     修正日志输出方案，以采集点为存放单位变更为以MSC设备名称为存放单位  */
/*     修正采集开始时间方案，变更为实时数据采集                           */
/*                                                                        */
/* v1.3 2010.8 wangxiaohui                                                */
/*     解决两个不能正常Ftp采集的Msc：hzgs4, tzhds2。                      */
/*     针对这两个网元，Ftp的传输模式由被动模式调整为主动模式，其他不变。  */
/**************************************************************************/

/************************************************************************/
/* include 头文件 */
/************************************************************************/
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
#include "Ftp.h"

/************************************************************************/
/* 宏定义 */
/************************************************************************/
#define      MAX_FILENAME        256
#define      MAX_LONG_FILENAME   512
#define      MAX_BUFFER          2048
#define      MAX_TIME            15
#define      MAX_DATE            9
#define      MAX_COMPANY         64
#define      MAX_DEVICE          64
#define      MAX_FTP_IP          32
#define      MAX_FTP_USER        32
#define      MAX_FTP_PWD         32
#define      FTP_TIME_OUT        300
#define      MAX_CHILD_PROCESS   128
#define      SLEEP_TIME          60
#define      WORK                "./work"
#define      LOG                 "./log"
#define      CONF                "./conf"
#define      ERR_LOG_FILE        "./log/collect_err"
#define      PREFIX_RUN_LOG_FILE "./log/collect_run"
#define      DEFAULT_RUN_DIR     "./"
#define      DEFAULT_COMMIT_DIR  "./data/collect"

/************************************************************************/
/* 全局变量定义 */
/************************************************************************/
int          g_nCurrentProcessNumber    = 0;
char *       g_szProgName               = NULL;
char *       g_szFileCommitDir          = NULL;
char *       g_szRunDir                 = NULL;
char *       g_szRecollectRunDir        = NULL;
int          g_nDebug                   = 0;	//设为1开启调试模式
int          g_nParallelChildProcess    = 1;
int          g_nAdjustMinute            = 0;
char         g_szCollectStartTime[MAX_TIME];
int          g_term_flag                = 0;

/************************************************************************/
/* 结构体定义 */
/************************************************************************/
typedef struct 
{
	int pid;
	int sleep_time;
} t_child_process_status;

//FTP采集信息
typedef struct {
	//采集点编号
	int   collect_point;
	//厂商和设备
	char  company[MAX_COMPANY];
	char  device[MAX_DEVICE];
	//登陆信息,v1.1添加端口port
	char  ip[MAX_FTP_IP];
	int   port; //端口port:华为6621
	char  usr[MAX_FTP_USER];
	char  password[MAX_FTP_PWD];
	//各种目录
	char  path_str[MAX_FILENAME];
	char  path_up[MAX_FILENAME];
	char  path_last[MAX_FILENAME];
	char  path_pre[MAX_FILENAME];
	char  path_suf[MAX_FILENAME];
	int   is_multi_path;  
	//各种文件
	char  file_str[MAX_FILENAME];
	char  file_pre[MAX_FILENAME];
	char  file_suf[MAX_FILENAME];
	//备份目录
	char  backup_path[MAX_FILENAME];
	//各种标识
	int   is_backup;
	int   is_commit;
	int   is_interval_file;
	int   current_process_number;
} collect_conf; 

/************************************************************************/
/* 枚举定义 */
/************************************************************************/
typedef enum parse_code 
{
	PARSE_FAIL 	    = -1,
	PARSE_MATCH	    = 1 ,
	PARSE_UNMATCH	= 2 ,
} parse_code_e;

/************************************************************************/
/* 函数声明 */
/************************************************************************/
static void         usage(int status);
static int          get_time(char * ret_cur_time);
static int          get_collect_time(char * ret_collect_time, int adjust_minute);
static int          err_log(const char * format,...);
static void         daemon_start(void);
static parse_code_e pre_suf_check(const char * name,const char * prefix, const char * suffix);
static parse_code_e get_orig_file_name(const char * dir_name, const char * prefix,const char * suffix, char * ret_file_name);
static int          verify_collect_conf(int * ret_point_num);
static parse_code_e get_collect_conf(int nCollectPointNo, int nCurrentProcessNumber, collect_conf * pCollectConf);
static int          process_collect(int current_number, int parallel_number);
static int			point_collect(int nCollectPointNo, int nCurrentProcessNo);
static int		 	convert_date_hw(char* lpOutDateTime, const char* lpInTime, const char* lpInDate);
static int			convert_date_hw_sp6(char* lpOutDateTime, char* lpInTime, const char* lpInDate);
static int			convert_date_nsn(char* lpOutDateTime, const char* lpInTime, const char* lpInDate);
static int			get_file_passive(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long * file_size);
static int			get_file_port(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long * file_size);
static int			backup_file(collect_conf * p_collect_conf, const char * remote_file_name, const char* lpTimeStamp);
static int			commit_file(collect_conf * p_collect_conf, const char * remote_file_name, const char* lpTimeStamp);
static int			get_backup_name(collect_conf * pCollectConf, const char * szRemoteFileName, const char* lpTimeStamp, char * szBackupName);
static int			run_log(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long file_size, const char* lpTimeStamp);
static int          gen_recollect_record(collect_conf * p_collect_conf, 
                        const char * start_time, const char * end_time);
static void         sigint_handler(int signum);

/*
*  Display the syntax for starting this program.
*/
static void usage(int status)
{
	FILE *output = status?stderr:stdout;
	
	fprintf(output,"Usage: %s [-o commit path] [-r collect run path] [-R recollect run path] [-p process number] [-m adjust minute] [-d]\n",g_szProgName);
	fprintf(output,"\nOptions:\n");
	fprintf(output,"        -o file commit directory, default is ./data\n");
	fprintf(output,"        -r collect run directory, default is ./\n");
	fprintf(output,"        -R recollect run directory, can be null.\n");
	fprintf(output,"        -p max parallel process number, default is 1\n");
    fprintf(output,"        -m adjust minute, default is 0\n");
	fprintf(output,"        -d debug flag\n");
	
	exit(status);
}

static int get_time(char * ret_cur_time)
{
	time_t t;
	struct tm *systime;
	
	t=time(NULL);
	if(t==-1)
	{
		strcpy(ret_cur_time,"yyyymmddhhmiss");
		return 1;
	}
	
	systime=localtime(&t);
	if(systime==NULL)
	{
		strcpy(ret_cur_time,"yyyymmddhhmiss");
		return 1;
	}
	
	sprintf(ret_cur_time,"%04d%02d%02d%02d%02d%02d",systime->tm_year+1900,systime->tm_mon+1,\
		systime->tm_mday,systime->tm_hour,systime->tm_min,systime->tm_sec);
	
	return 0;
}

static int get_collect_time(char * ret_collect_time, int adjust_minute)
{
	time_t t;
	struct tm *systime;
	
	t=time(NULL);
	if(t==-1)
	{
		strcpy(ret_collect_time,"yyyymmddhhmiss");
		return 1;
	}
	
    /* adjust time */
    if(adjust_minute != 0)
    {
        t += adjust_minute * 60;
    }

	systime=localtime(&t);
	if(systime==NULL)
	{
		strcpy(ret_collect_time,"yyyymmddhhmiss");
		return 1;
	}
	
	sprintf(ret_collect_time,"%04d%02d%02d%02d%02d%02d",systime->tm_year+1900,systime->tm_mon+1,\
		systime->tm_mday,systime->tm_hour,systime->tm_min,systime->tm_sec);
	
	return 0;
}

/************************************************************************/
/* 错误日志记录函数 */
/************************************************************************/
static int err_log(const char * format,...)
{
	time_t t;
	struct tm *systime;
	FILE *fp;
	
	char   tmp[MAX_FILENAME];
	
	va_list ap;
	
 	if(g_nDebug)
 	{
 		va_start(ap, format);
 		vprintf(format,ap);
 		va_end(ap);
 		printf("\n");
 	}
	
	if(g_nCurrentProcessNumber!=0)
	{
		sprintf(tmp,"%s.%03d",ERR_LOG_FILE,g_nCurrentProcessNumber);
		fp=fopen(tmp,"a+");
	}
	else
		fp=fopen(ERR_LOG_FILE,"a+");
	
	if(fp==NULL)
	{
		return 1;
	}
	
	t=time(NULL);
	systime=localtime(&t);
	fprintf(fp,"------------------------------------------------------------\n");
	fprintf(fp,asctime(systime));
	
	va_start(ap, format);
	vfprintf(fp,format,ap);
	va_end(ap);
	fprintf(fp,"\n");
	
	fclose(fp);
	
	return 0;
}

/************************************************************************/
/* 开启守护进程 */
/************************************************************************/
static void daemon_start(void)
{
	int childpid;
	
	umask(022);
	
	if(getppid() == 1) 
	{
		return; 
	}
	
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	
	if(setpgrp()==-1)
    {
		err_log("daemon_start: can't change process group\n");
    }
	signal(SIGHUP,SIG_IGN);
	
	
	if((childpid = fork())<0) 
	{
		err_log("daemon_start: fork error\n");
		exit(1);
	}
	else if(childpid > 0)
	{
		exit(0);
	}
}

/************************************************************************/
/* 入口函数*/
/************************************************************************/
int main(int argc,char * argv[])
{
    int     ret = 0;
	int     nArgVal;
	struct  stat stat_buff;
	char    szFileName[MAX_FILENAME];
	char    szTmpFileName[MAX_LONG_FILENAME];
	int     nConfLine = 0;
	int     i;
	t_child_process_status oChildProcessStatus[MAX_CHILD_PROCESS];
	pid_t   pidChild;
	pid_t   pidReValWait;

    /* register the signal SIGINT handler */
    if(signal(SIGINT, &sigint_handler) == SIG_ERR)
    {
        err_log("main: signal register fail\n");
        ret = 1;
        goto Exit_Pro;
    }
	
    /* get the grogram name */
	if (( g_szProgName = strrchr(argv[0], '/')) == NULL)
		g_szProgName = argv[0];
	else
		g_szProgName++;

	/*  Process the options.  */
	while ((nArgVal = getopt(argc, argv, "o:r:R:p:m:d")) != EOF) {
		
		switch(nArgVal) 
		{
			
		case 'o':
			g_szFileCommitDir = strdup(optarg);
			break;	
		case 'r':
			g_szRunDir = strdup(optarg);
			break;	
		case 'R':
			g_szRecollectRunDir = strdup(optarg);
			break;	
		case 'p':
			g_nParallelChildProcess = atoi(optarg);
			break;
        case 'm':
            g_nAdjustMinute = atoi(optarg);
            break;
		case 'd':
			g_nDebug = 1;
			break;	
		default:
			usage(1);
			break;
			
		}
	}
	
    /* 设置默认值 */
    if(g_szRunDir == NULL)
    {
        g_szRunDir = (char *)malloc(MAX_FILENAME);
        if(g_szRunDir == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(g_szRunDir, DEFAULT_RUN_DIR);
    }

    if(g_szFileCommitDir == NULL)
    {
        g_szFileCommitDir = (char *)malloc(MAX_FILENAME);
        if(g_szFileCommitDir == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
    }

	/* Change dir */
	if(chdir(g_szRunDir)==-1)
	{
		err_log("main: chdir to %s fail\n",g_szRunDir);
        ret = 1;
        goto Exit_Pro;
	}

	/* Check dir WORK*/
	if(stat(WORK,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",WORK);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
		if(!S_ISDIR(stat_buff.st_mode))	
		{
 			err_log("main: %s isn't a dir\n",WORK);
            ret = 1;
            goto Exit_Pro;
		}
	}
	
	/* Check dir LOG*/
	if(stat(LOG,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",LOG);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
		if(!S_ISDIR(stat_buff.st_mode))	
		{
			err_log("main: %s isn't a dir\n",LOG);
            ret = 1;
            goto Exit_Pro;
		}
	}
	
	/* Check dir CONF*/
	if(stat(CONF,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",CONF);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
		if(!S_ISDIR(stat_buff.st_mode))	
		{
			err_log("main: %s isn't a dir\n",CONF);
            ret = 1;
            goto Exit_Pro;
		}
	}
	
	/* Check commit dir */
	if(stat(g_szFileCommitDir,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",g_szFileCommitDir);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
		if(!S_ISDIR(stat_buff.st_mode))	
		{
			err_log("main: %s isn't a dir\n",g_szFileCommitDir);
            ret = 1;
            goto Exit_Pro;
		}
	}

	/* 验证采集配置 */
	if(verify_collect_conf(&nConfLine) != 0)
	{
		err_log("main: verify_collect_conf fail\n");
        ret = 1;
        goto Exit_Pro;
	}

	if(nConfLine <= 0)
	{
		err_log("main: collect point num is incorrect: %d\n",nConfLine);
        ret = 1;
        goto Exit_Pro;
	}

	if(g_nParallelChildProcess<=0)                g_nParallelChildProcess=1;
	if(g_nParallelChildProcess>MAX_CHILD_PROCESS) g_nParallelChildProcess=MAX_CHILD_PROCESS;
    if(g_nParallelChildProcess>nConfLine)         g_nParallelChildProcess=nConfLine;

	/*清理work目录下文件*/
	memset(szFileName, 0, sizeof(szFileName));
	while( get_orig_file_name(WORK,NULL,NULL,szFileName) == PARSE_MATCH )
	{
		sprintf(szTmpFileName,"%s/%s",WORK,szFileName);
		if ( unlink(szTmpFileName) != 0 )
		{
			err_log("main: unlink file %s fail\n",szTmpFileName);
            ret = 1;
            goto Exit_Pro;
		}
		memset(szFileName,0,sizeof(szFileName));
	}

    /* 获取采集开始时间 */
    memset(g_szCollectStartTime, 0, sizeof(g_szCollectStartTime));
    if(get_collect_time(g_szCollectStartTime, g_nAdjustMinute) != 0)
    {
        err_log("main: get collect start time fail\n");
        ret = 1;
        goto Exit_Pro;
    }

	/*初始化 子进程结构数组*/
	for(i=0;i<MAX_CHILD_PROCESS;i++)
		memset(&oChildProcessStatus[i],0,sizeof(t_child_process_status));
	
    /* 进入守护状态 */
	daemon_start();

	/*采集处理*/
	while(1)
	{
		/*创建子进程*/
		for(i=0;i<g_nParallelChildProcess;i++)
		{
			if(oChildProcessStatus[i].pid==0&&oChildProcessStatus[i].sleep_time<=0)
			{
				if((pidChild=fork())<0) /*fork error*/
				{
					err_log("main: fork error\n");
					exit(1);
				}
				else if(pidChild>0)     /**/ 
				{
					oChildProcessStatus[i].pid        = pidChild;
					oChildProcessStatus[i].sleep_time = SLEEP_TIME;
				}
				else if(pidChild==0)    /*child process*/
				{
					process_collect(i+1, g_nParallelChildProcess);
					exit(0);
				}
			}
		}
		
		/*回收子进程*/
		for(i=0;i<g_nParallelChildProcess;i++)
		{
			if(oChildProcessStatus[i].pid>0)
			{
				pidReValWait = waitpid(oChildProcessStatus[i].pid,NULL,WNOHANG);
				if(pidReValWait>0)
				{
					oChildProcessStatus[i].pid=0;
				}
				if(pidReValWait<0)
				{
					err_log("main: waitpid fail\n");
					exit(1);
				}
			}
		}
		
        /* 如果设置了终止标志，中止所有的采集进程并退出循环 */
        if(g_term_flag)
        {
            for(i=0;i<g_nParallelChildProcess;i++)
            {
                if(oChildProcessStatus[i].pid>0)
                {
                    kill(oChildProcessStatus[i].pid, SIGKILL);
                }
            }
            break;
        }

		sleep(1);

		/*每秒递减sleep_time*/
		for( i=0 ; i < g_nParallelChildProcess; i++ )
		{
			if( oChildProcessStatus[i].pid == 0 && oChildProcessStatus[i].sleep_time > 0 )
			{	
				oChildProcessStatus[i].sleep_time--;
			}
		}
	}
	
Exit_Pro:
    if(g_szRunDir != NULL)
    {
        free(g_szRunDir);
    }
    if(g_szFileCommitDir != NULL)
    {
        free(g_szFileCommitDir);
    }
    if(g_szRecollectRunDir != NULL)
    {
        free(g_szRecollectRunDir);
    }
	return ret;
}

/*
*  pre_suf_check()
*/
static parse_code_e pre_suf_check(const char * name, const char * prefix, const char * suffix)
{
	if(name==NULL)
		return PARSE_UNMATCH;
	
	/*前缀匹配检查*/
	if(prefix!=NULL)
	{
		if(strstr(name,prefix)!=name)
			return PARSE_UNMATCH;
	}
	/*后缀匹配检查*/
	if(suffix!=NULL)
	{
		if(strlen(name)<strlen(suffix))
			return PARSE_UNMATCH;
		if(strcmp( (name+strlen(name)-strlen(suffix)),suffix) != 0 )
			return PARSE_UNMATCH;
	}
	
	return PARSE_MATCH;
}

/*
*  get_orig_file_name()
*/
static parse_code_e get_orig_file_name(const char * dir_name, const char * prefix, const char * suffix,char * ret_file_name)
{
	int              ret;
	DIR *            pdir=NULL;
	struct dirent *	 pdirent;
	struct stat stat_buff;
	char             tmp_file_name[MAX_LONG_FILENAME];
	
	ret=PARSE_UNMATCH;
	
	if(dir_name==NULL)
		pdir=opendir("./.");
	else
		pdir=opendir(dir_name);
	
	if(pdir==NULL)
	{
		err_log("get_orig_file_name: opendir fail\n");
		ret=PARSE_FAIL;
		goto Exit_Pro;	
	}
	while( (pdirent=readdir(pdir))!=NULL )
	{
        /* 前后缀匹配检查 */
		if( pre_suf_check(pdirent->d_name, prefix, suffix) != PARSE_MATCH )
        {
			continue;
        }
	
		/*文件检查*/
		if(dir_name!=NULL)
		{
			strcpy(tmp_file_name,dir_name);
			strcat(tmp_file_name,"/");
			strcat(tmp_file_name,pdirent->d_name);
		}
		else
			strcpy(tmp_file_name,pdirent->d_name);
		
		if(stat(tmp_file_name,&stat_buff)==-1)
		{
			err_log("get_orig_file_name: stat %s fail\n",pdirent->d_name);
			ret=PARSE_FAIL;
			break;
		}
		if(S_ISREG(stat_buff.st_mode))
		{
			strcpy(ret_file_name,pdirent->d_name);
			ret=PARSE_MATCH;
			break;	
		}	
	}
	
Exit_Pro:
	if(pdir!=NULL)
		closedir(pdir);
	return ret;
}

/************************************************************************/
/* 验证采集配置函数 */
/************************************************************************/
/*
* verify_collect_conf()
*                      return 1 fail, 0 success
*/
static int verify_collect_conf(int * ret_point_num)
{
	int           ret = 0;
	FILE          *pFile=NULL;
	char          szTempFileName[MAX_FILENAME];
	char	      szBuff[MAX_BUFFER];
	char          szContent[12][MAX_FILENAME];
	int           nLen;
    int           line;
	
	sprintf(szTempFileName,"%s/collect.conf",CONF);

	pFile = fopen(szTempFileName,"r");
	if(pFile==NULL)
	{
		err_log("verify_collect_conf: fopen %s fail\n",szTempFileName);
		ret = 1;
		goto Exit_Pro;
	}
	
    line = 0;

	while(1)
	{
		memset(szBuff,0,sizeof(szBuff));
		memset(szContent,0,sizeof(szContent));
		if(fgets(szBuff,sizeof(szBuff),pFile)==NULL)
			break;
		
		line++;

        if(szBuff[0] == '#')
            continue;
    
        if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],\
            szContent[4],szContent[5],szContent[6],szContent[7],szContent[8],szContent[9],szContent[10],szContent[11]) != 12 )
        {
            err_log("verify_collect_conf: collect.conf line %d incorrect\n",line);
            ret=1;
            goto Exit_Pro;
        }
        
        nLen=strlen(szContent[0]);
        if(nLen!=8 || szContent[0][0]!='<' || szContent[0][nLen-1]!='>')
        {
            err_log("verify_collect_conf: collect.conf line %d incorrect\n",line);
            ret=1;
            goto Exit_Pro;
        }
        
        szContent[0][nLen-1]='\0';
        if(atoi(&szContent[0][1])!=line)
        {
            err_log("verify_collect_conf: collect.conf line %d incorrect\n",line);
            ret=1;
            goto Exit_Pro;
        }
	}

    (*ret_point_num) = line;
	
Exit_Pro:
	if(pFile!=NULL)
	{
		fclose(pFile);
	}
	return ret;
}

/************************************************************************/
/* 获取采集配置 */
/************************************************************************/
static parse_code_e get_collect_conf(int nCollectPointNo,int nCurrentProcessNumber,collect_conf * pCollectConf)
{
	parse_code_e  ret;
	FILE        * pFile=NULL;
	char          szTempFileName[MAX_FILENAME];
	char	      szBuff[MAX_BUFFER];
	char          szContent[12][MAX_FILENAME];
	char        * szStr=NULL;
	int           nLen;
	int           nLineNo;
	
	ret = PARSE_UNMATCH;
	
	sprintf(szTempFileName,"%s/collect.conf",CONF);
	pFile = fopen(szTempFileName,"r");
	if(pFile == NULL)
	{
		err_log("get_collect_conf: fopen %s fail\n",szTempFileName);
		ret = PARSE_FAIL;
		goto Exit_Pro;
	}
	
	nLineNo = 0;
	while(1)
	{
		memset(szBuff,0,sizeof(szBuff));
		if(fgets(szBuff,sizeof(szBuff),pFile)==NULL)
		{
			if(nLineNo < nCollectPointNo)
			{
				err_log("get_collect_conf: collect.conf line less %d\n",nCollectPointNo);
				ret = PARSE_FAIL;
				goto Exit_Pro;
			}
			break;
		}
       	
		nLineNo++;

        if(nLineNo != nCollectPointNo)
            continue;
    
        if(szBuff[0] == '#')
        {
            ret = PARSE_UNMATCH;
            break;
        }
        
        memset(szContent,0,sizeof(szContent));
        if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],\
            szContent[5],szContent[6],szContent[7],szContent[8],szContent[9],szContent[10],szContent[11]) != 12 )
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret = PARSE_FAIL;
            goto Exit_Pro;
        }
        
        pCollectConf->collect_point = nLineNo;
        strcpy(pCollectConf->ip,szContent[3]);
        strcpy(pCollectConf->company,szContent[1]);
        strcpy(pCollectConf->device,szContent[2]);
        pCollectConf->port = atoi(szContent[4]);
        strcpy(pCollectConf->usr,szContent[5]);
        strcpy(pCollectConf->password,szContent[6]);
        strcpy(pCollectConf->path_str,szContent[7]);
        strcpy(pCollectConf->file_str,szContent[8]);
        strcpy(pCollectConf->backup_path,szContent[9]);
        pCollectConf->current_process_number = nCurrentProcessNumber;

        /*path_up,path_last,path_pre,path_suf,is_multi_path*/
        nLen = strlen(szContent[7]);
        if(nLen < 1)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret = PARSE_FAIL;
            goto Exit_Pro;
        }
        szStr = strrchr(szContent[7],'/');
        if(szStr == NULL)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret=PARSE_FAIL;
            goto Exit_Pro;
        }
        strncpy(pCollectConf->path_up,szContent[7],szStr-szContent[7]+1);
        if( &szContent[7][nLen-1]-szStr>0 )
        {
            strncpy(pCollectConf->path_last,szStr+1,&szContent[7][nLen-1]-szStr);
        }
        
        if( (nLen=strlen(pCollectConf->path_last))>0 )
        {
            szStr=strchr(pCollectConf->path_last,'*');
            
            if(szStr==NULL)
            {
                pCollectConf->is_multi_path=0;
            }
            else
            {
                pCollectConf->is_multi_path=1;
                if(szStr>pCollectConf->path_last)
                {
                    strncpy(pCollectConf->path_pre,pCollectConf->path_last,szStr-pCollectConf->path_last);
                }
            }
            
            szStr=strrchr(pCollectConf->path_last,'*');
            if(szStr!=NULL)
            {
                if( &(pCollectConf->path_last[nLen-1])-szStr>0 )
                {
                    strncpy(pCollectConf->path_suf,szStr+1,&(pCollectConf->path_last[nLen-1])-szStr);
                }
            }
        }
        else
        {
            pCollectConf->is_multi_path=0;
        }	
        
        /*file_pre,file_suf*/
        nLen = strlen(szContent[8]);
        if(nLen < 1)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret=PARSE_FAIL;
            goto Exit_Pro;
        }
        szStr=strchr(szContent[8],'*');
        if(szStr == NULL)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret=PARSE_FAIL;
            goto Exit_Pro;
        }
        if(szStr>szContent[8])
        {
            strncpy(pCollectConf->file_pre,szContent[8],szStr-szContent[8]);
        }
        szStr=strrchr(szContent[8],'*');
        if(szStr == NULL)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret=PARSE_FAIL;
            goto Exit_Pro;
        }
        if( &szContent[8][nLen-1]-szStr>0 )
        {
            strncpy(pCollectConf->file_suf,szStr+1,&szContent[8][nLen-1]-szStr);
        }
        
        /*is_backup,is_commit,is_interval_file*/
        if(strcasecmp(szContent[9],"not")==0)
        {
            pCollectConf->is_backup=0;
        }
        else
        {
            pCollectConf->is_backup=1;
        }
        
        if(strcasecmp(szContent[10],"yes")==0)
        {
            pCollectConf->is_commit=1;
        }
        else
        {
            pCollectConf->is_commit=0;
        }
        
        if(strcasecmp(szContent[11],"yes")==0)
        {
            pCollectConf->is_interval_file=1;
        }
        else
        {
            pCollectConf->is_interval_file=0;
        }
        
        ret = PARSE_MATCH;

        break;
	}

Exit_Pro:	
	if(pFile!=NULL)
		fclose(pFile);
	return ret;
}

/*
*process_collect()
*
*
*/
static int process_collect(int current_number,int parallel_number)
{
	int     max_collect_point = 0;
	int     factor;
    int     collect_point;
	
	/*设置全局子进程编号*/
	g_nCurrentProcessNumber=current_number;

	if(verify_collect_conf(&max_collect_point) != 0)
    {
        err_log("process_collect: verify_collect_conf fail\n");
        return 1;
    }

	if(max_collect_point <= 0)
	{
		err_log("process_collect: verify_collect_conf collect pointer num is incorrect %d\n",max_collect_point);
		return 1;
	}
	
	factor = 0;
	while(1)
	{
		collect_point = factor * parallel_number + current_number;
		if(collect_point > max_collect_point)  break;
		point_collect(collect_point, current_number);
		factor++;
	}
	
	return 0;
}

/************************************************************************/
/* 浙江话单采集入库专用采集函数 */
/************************************************************************/
static int point_collect(int nCollectPointNo,int nCurrentProcessNo)
{
	int nRet = 0;
	int nRetVal;
	//配置文件结构
	collect_conf curCollectConf;
	//目录列表,文件列表,时间端点文件名
	char szDirListFile[MAX_FILENAME];
	char szFileListFile[MAX_FILENAME];
	char szTimePointFile[MAX_FILENAME];
	char szBackupFile[MAX_FILENAME];
	//文件句柄
	FILE * pFileDirList = NULL;
	FILE * pFileFileList = NULL;
	FILE * pFileTimePoint = NULL;
	//临时buffer
	char szBuff[MAX_BUFFER];
	//解析数组
	char szContent[9][MAX_FILENAME];
	//时间端点和临时时间端点
	char szTimePoint[MAX_TIME];
	char szTempTimePoint[MAX_TIME];
	char szTimePointPre[MAX_DATE];
	char szFileTimeStamp[MAX_TIME];
	char szFileTimeStampPre[MAX_DATE];
	long lFileSize;

	char  file_name[MAX_FILENAME];
	char  tmp_file_name[MAX_LONG_FILENAME];
	char  pre[MAX_DATE];

    //获取连接配置
	memset(&curCollectConf,0,sizeof(curCollectConf));
	nRetVal = get_collect_conf(nCollectPointNo,nCurrentProcessNo,&curCollectConf);
	if(nRetVal == PARSE_FAIL)
	{
		err_log("point_collect: get_collect_conf fail, nCollectPointNo=%d\n",nCollectPointNo);
		nRet = 1;
		goto Exit_Pro;
	}
	if(nRetVal == PARSE_UNMATCH) 
	{
		/*collect.conf配置文件中,该行以#开头,不处理直接跳出*/
		nRet = 1;		
		goto Exit_Pro;
	}

	sprintf(szDirListFile, "%s/%06d_DirList", WORK, curCollectConf.current_process_number);
	sprintf(szFileListFile, "%s/%06d_FileList", WORK, curCollectConf.current_process_number);
	sprintf(szTimePointFile, "%s/%s[%s]_TimePoint", CONF, curCollectConf.device, curCollectConf.ip);

    /* 从采集时间点记录文件中读取上一次采集的最后时间
     * 如果记录时间在当前时间（调整后的当前时间）之后，则使用记录的时间作为本此采集的开始时间
     * 如果记录时间在当前时间之前，则使用当前时间作为本此采集开始时间，同时把中间没有采集的
     * 时间段生成一条补采记录写入到补采程序的配置文件collect.conf中，补采程序会自动完成这段
     * 时间的数据补采
     */
	pFileTimePoint = fopen(szTimePointFile , "r"); 
	if( (NULL == pFileTimePoint) )
	{
		strcpy(szTimePoint, g_szCollectStartTime);
	}
	else if ( NULL == fgets(szTimePoint, sizeof(szTimePoint), pFileTimePoint) )
	{
		strcpy(szTimePoint, g_szCollectStartTime);
	}
    else
    {
        if(strncmp(szTimePoint, g_szCollectStartTime, 14) < 0)
        {
            /* 生成一条补采记录 */
            if(gen_recollect_record(&curCollectConf, szTimePoint, g_szCollectStartTime) != 0)
            {
                err_log("point_collect: generate recollect record fail\n");
            }

            strcpy(szTimePoint, g_szCollectStartTime);
        }
    }
    strcpy(szTempTimePoint, szTimePoint);
	strncpy(szTimePointPre, szTimePoint, 8);
	szTimePointPre[8] = '\0';
    if(pFileTimePoint != NULL)
    {
        fclose(pFileTimePoint);
        pFileTimePoint = NULL;
    }

    //处理文件 
	if (0 == strncmp("hzgs6", curCollectConf.device, 5) || 
		0 == strncmp("hzgs11", curCollectConf.device, 6) || 
		0 == strncmp("hzgs20", curCollectConf.device, 6) || 
		0 == strncmp("nbogs22", curCollectConf.device, 7) || 
		0 == strncmp("wzhgs26", curCollectConf.device, 7) || 
		0 == strncmp("shxgs7", curCollectConf.device, 6) || 
		0 == strncmp("jihgs2", curCollectConf.device, 6) || 
		0 == strncmp("jihgs11", curCollectConf.device, 7) || 
		0 == strncmp("zshgs1", curCollectConf.device, 6) || 
		0 == strncmp("lshgs1", curCollectConf.device, 6) || 
		0 == strncmp("lshgs2", curCollectConf.device, 6) || 
		0 == strncmp("tzhds4", curCollectConf.device, 6) || 
		0 == strncmp("huzds1", curCollectConf.device, 6) || 
		0 == strncmp("hzgs9", curCollectConf.device, 5))
	{
		/* 删除旧的目录列表文件 */
		if( 0 == access(szDirListFile, F_OK) )
		{
			if(0 != unlink(szDirListFile))
			{
				err_log("point_collect: unlink %s fail\n",szDirListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* 建立Ftp连接 */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,1,g_nDebug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* 获取目录列表 */
		nRetVal = Ftp_Dir(szDirListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* 关闭Ftp连接 */
		Ftp_Close();

		//打开文件
		pFileDirList = fopen( szDirListFile , "r" );
		if(pFileDirList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szDirListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//循环进入目录
		while (1)
		{		
			//获取一行配置
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileDirList) == NULL )
				break;

			//应该是 4 个参数
			if( sscanf(szBuff,"%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3]) != 4 )
				continue;

			//判断是否要进入
			if ( NULL == strstr(szContent[2], "DIR") ) //目录
			{
				continue;	
			}
			if ( '2' != szContent[3][0] || '0' != szContent[3][1] )	//以20开头
			{
				continue;	
			}
			if ( 8 != strlen(szContent[3])) //8位长
			{
				continue;	
			}
			strcpy(szFileTimeStampPre, szContent[3]);
			if ( 0 < strncmp(szTimePointPre, szFileTimeStampPre, 8))
			{
				continue;	
			}
	
			/* 删除旧的下载文件列表文件 */
			if( 0 == access(szFileListFile, F_OK) )
			{
				if(0 != unlink(szFileListFile))
				{
					err_log("point_collect: unlink %s fail\n",szDirListFile);
					nRet = 1;		
					goto Exit_Pro;
				}
			}			

			/* 建立Ftp连接 */
			nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
				curCollectConf.port,FTP_TIME_OUT,1,1,g_nDebug);
			if(nRetVal != 0)
			{
				err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
					curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
				nRet = 1;		
				goto Exit_Pro;
			}

			//进入目录
			nRetVal = Ftp_Cd(szContent[3]);
			if( 0 != nRetVal )
			{
				err_log("point_collect: Ftp_Cd fail,collect_point=%d,dir=%s\n",curCollectConf.collect_point,szContent[8]);
				nRet = 1;		
				goto Exit_Pro;
			}

			/* 获取最新的下载文件列表 */
			nRetVal = Ftp_Dir(szFileListFile);
			if( 0 != nRetVal )
			{
				err_log("point_collect: collect_point=%d,Ftp_Dir fail",curCollectConf.collect_point);
				nRet = 1;		
				goto Exit_Pro;
			}

            /* 关闭Ftp连接 */
			Ftp_Close();

			//打开文件
			pFileFileList = fopen( szFileListFile , "r" );
			if(szFileListFile == NULL)
			{
				err_log("point_collect: fopen %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}

			//循环下载
			while (1)
			{
				//获取一行配置
				memset(szBuff, 0 , sizeof(szBuff));
				memset(szContent, 0 , sizeof(szContent));
				if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				{
					break;
				}

				//应该是4个参数
				if(sscanf(szBuff,"%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3]) != 4 )
					continue;

                //如果是旧的话单文件，就不再下载
                if(strncmp(szContent[3], "gz", 2) != 0)
                {
                    continue;
                }

				//判断是否要下载
				if (0 != convert_date_hw_sp6(szFileTimeStamp, szContent[1], szFileTimeStampPre))//拿到文件时间戳
				{
					continue;
				}

				if ( 0 < strncmp(szTimePoint, szFileTimeStamp, 14)) //比较时间
				{
					continue;	
				}

				if(get_backup_name(&curCollectConf,szContent[3],szFileTimeStamp,szBackupFile)!=0) //获取备份文件名
				{
					err_log("point_collect: get_backup_name fail\n");
					nRet = 1;
					goto Exit_Pro;
				}
				if(0 == access(szBackupFile,F_OK))  //文件存在就不下载
				{
					//给时间戳到临时时间端点
					if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
					{
						strcpy(szTempTimePoint, szFileTimeStamp);
					}
					continue;
				}

                //下载文件
				if(get_file_passive(&curCollectConf, szFileTimeStampPre, szContent[3], &lFileSize)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_passive FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //备份文件
				if(backup_file(&curCollectConf, szContent[3], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //提交文件
				if(commit_file(&curCollectConf, szContent[3], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //记录日志
				if(run_log(&curCollectConf,szFileTimeStampPre,szContent[3],lFileSize,szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //给时间戳到临时时间端点
				if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
				{
					strcpy(szTempTimePoint, szFileTimeStamp);
				}
			}
		}
	}
    else if(0 == strncmp("hzgs4", curCollectConf.device, 5))
    {
		/* 华为特殊MSC设备的采集 */

		/* 删除旧的目录列表文件 */
		if( 0 == access(szDirListFile, F_OK) )
		{
			if(0 != unlink(szDirListFile))
			{
				err_log("point_collect: unlink %s fail\n",szDirListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* 建立Ftp连接 */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,0,g_nDebug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* 获取目录列表 */
		nRetVal = Ftp_Dir(szDirListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* 关闭Ftp连接 */
		Ftp_Close();

		//打开文件
		pFileDirList = fopen( szDirListFile , "r" );
		if(pFileDirList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szDirListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//循环进入目录
		while (1)
		{		
			//获取一行配置
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileDirList) == NULL )
				break;

			//应该是 4 个参数
			if( sscanf(szBuff,"%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3]) != 4 )
				continue;

			//判断是否要进入
			if ( NULL == strstr(szContent[2], "DIR") ) //目录
			{
				continue;	
			}
			if ( '2' != szContent[3][0] || '0' != szContent[3][1] )	//以20开头
			{
				continue;	
			}
			if ( 8 != strlen(szContent[3])) //8位长
			{
				continue;	
			}
			strcpy(szFileTimeStampPre, szContent[3]);
			if ( 0 < strncmp(szTimePointPre, szFileTimeStampPre, 8))
			{
				continue;	
			}
	
			/* 删除旧的下载文件列表文件 */
			if( 0 == access(szFileListFile, F_OK) )
			{
				if(0 != unlink(szFileListFile))
				{
					err_log("point_collect: unlink %s fail\n",szDirListFile);
					nRet = 1;		
					goto Exit_Pro;
				}
			}			

			/* 建立Ftp连接 */
			nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
				curCollectConf.port,FTP_TIME_OUT,1,0,g_nDebug);
			if(nRetVal != 0)
			{
				err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
					curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
				nRet = 1;		
				goto Exit_Pro;
			}

			//进入目录
			nRetVal = Ftp_Cd(szContent[3]);
			if( 0 != nRetVal )
			{
				err_log("point_collect: Ftp_Cd fail,collect_point=%d,dir=%s\n",curCollectConf.collect_point,szContent[8]);
				nRet = 1;		
				goto Exit_Pro;
			}

			/* 获取最新的下载文件列表 */
			nRetVal = Ftp_Dir(szFileListFile);
			if( 0 != nRetVal )
			{
				err_log("point_collect: collect_point=%d,Ftp_Dir fail",curCollectConf.collect_point);
				nRet = 1;		
				goto Exit_Pro;
			}

            /* 关闭Ftp连接 */
			Ftp_Close();

			//打开文件
			pFileFileList = fopen( szFileListFile , "r" );
			if(szFileListFile == NULL)
			{
				err_log("point_collect: fopen %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}

			//循环下载
			while (1)
			{
				//获取一行配置
				memset(szBuff, 0 , sizeof(szBuff));
				memset(szContent, 0 , sizeof(szContent));
				if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				{
					break;
				}

				//应该是4个参数
				if(sscanf(szBuff,"%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3]) != 4 )
					continue;

                //如果是旧的话单文件，就不再下载
                if(strncmp(szContent[3], "gz", 2) != 0)
                {
                    continue;
                }

				//判断是否要下载
				if (0 != convert_date_hw_sp6(szFileTimeStamp, szContent[1], szFileTimeStampPre))//拿到文件时间戳
				{
					continue;
				}

				if ( 0 < strncmp(szTimePoint, szFileTimeStamp, 14)) //比较时间
				{
					continue;	
				}

				if(get_backup_name(&curCollectConf,szContent[3],szFileTimeStamp,szBackupFile)!=0) //获取备份文件名
				{
					err_log("point_collect: get_backup_name fail\n");
					nRet = 1;
					goto Exit_Pro;
				}
				if(0 == access(szBackupFile,F_OK))  //文件存在就不下载
				{
					//给时间戳到临时时间端点
					if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
					{
						strcpy(szTempTimePoint, szFileTimeStamp);
					}
					continue;
				}

                //下载文件
				if(get_file_port(&curCollectConf, szFileTimeStampPre, szContent[3], &lFileSize)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_port FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //备份文件
				if(backup_file(&curCollectConf, szContent[3], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //提交文件
				if(commit_file(&curCollectConf, szContent[3], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //记录日志
				if(run_log(&curCollectConf,szFileTimeStampPre,szContent[3],lFileSize,szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //给时间戳到临时时间端点
				if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
				{
					strcpy(szTempTimePoint, szFileTimeStamp);
				}
			}
		}
    }
    else if(0 == strncmp("tzhds2", curCollectConf.device, 6))
    {
		/* 删除旧的目录列表文件 */
		if( 0 == access(szDirListFile, F_OK) )
		{
			if(0 != unlink(szDirListFile))
			{
				err_log("point_collect: unlink %s fail\n",szDirListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* 建立Ftp连接 */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,0,g_nDebug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* 获取最新的目录列表 */
		nRetVal = Ftp_Dir(szDirListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* 关闭Ftp连接 */
		Ftp_Close();

		//打开文件
		pFileDirList = fopen( szDirListFile , "r" );
		if(pFileDirList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szDirListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//循环进入目录
		while (1)
		{		
			//获取一行配置
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileDirList) == NULL )
				break;

			//应该是9个参数
			if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
				szContent[5],szContent[6],szContent[7],szContent[8]) != 9 )
				continue;

			//判断是否要进入
			if ( 'd' != szContent[0][0] ) //目录
			{
				continue;	
			}
			if ( '2' != szContent[8][0] || '0' != szContent[8][1] )	//以20开头
			{
				continue;	
			}
			if ( 8 != strlen(szContent[8])) //8位长
			{
				continue;	
			}

			strcpy(szFileTimeStampPre, szContent[8]);
			if ( 0 < strncmp(szTimePointPre, szFileTimeStampPre, 8))
			{
				continue;	
			}
	
			/* 删除旧的下载文件列表文件 */
			if( 0 == access(szFileListFile, F_OK) )
			{
				if(0 != unlink(szFileListFile))
				{
					err_log("point_collect: unlink %s fail\n",szDirListFile);
					nRet = 1;		
					goto Exit_Pro;
				}
			}			
		
			/* 建立Ftp连接 */
			nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
				curCollectConf.port,FTP_TIME_OUT,1,0,g_nDebug);
			if(nRetVal != 0)
			{
				err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
					curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
				nRet = 1;		
				goto Exit_Pro;
			}

			//进入目录
			nRetVal = Ftp_Cd(szContent[8]);
			if( 0 != nRetVal )
			{
				err_log("point_collect: Ftp_Cd fail,collect_point=%d,dir=%s\n",curCollectConf.collect_point,szContent[8]);
				nRet = 1;		
				goto Exit_Pro;
			}

			/* 获取最新的下载文件列表 */
			nRetVal = Ftp_Dir(szFileListFile);
			if( 0 != nRetVal )
			{
				err_log("point_collect: collect_point=%d,Ftp_Dir fail",curCollectConf.collect_point);
				nRet = 1;		
				goto Exit_Pro;
			}

            /* 关闭Ftp连接 */
			Ftp_Close();

			//打开文件
			pFileFileList = fopen( szFileListFile , "r" );
			if(szFileListFile == NULL)
			{
				err_log("point_collect: fopen %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}

			//循环下载
			while (1)
			{
				//获取一行配置
				memset(szBuff, 0 , sizeof(szBuff));
				memset(szContent, 0 , sizeof(szContent));
				if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				{
					break;
				}

				//应该是9个参数
				if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
					szContent[5],szContent[6],szContent[7],szContent[8]) != 9 )
					continue;

                //如果是旧的话单文件，就不再下载
                if(strncmp(szContent[8], "gz", 2) != 0)
                {
                    continue;
                }

				//判断是否要下载
				if ( '-' != szContent[0][0] ) //文件
				{
					continue;	
				}
				if (0 != convert_date_hw(szFileTimeStamp, szContent[7], szFileTimeStampPre))//拿到文件时间戳
				{
					continue;
				}
				if ( 0 < strncmp(szTimePoint, szFileTimeStamp, 14)) //比较时间
				{
					continue;	
				}

				if(get_backup_name(&curCollectConf,szContent[8],szFileTimeStamp,szBackupFile)!=0) //获取备份文件名
				{
					err_log("point_collect: get_backup_name fail\n");
					nRet = 1;
					goto Exit_Pro;
				}
				if(0 == access(szBackupFile,F_OK))  //文件存在就不下载
				{
					//给时间戳到临时时间端点
					if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
					{
						strcpy(szTempTimePoint, szFileTimeStamp);
					}
					continue;
				}

                //下载文件
				if(get_file_port(&curCollectConf, szFileTimeStampPre, szContent[8], &lFileSize)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_port FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //备份文件
				if(backup_file(&curCollectConf, szContent[8], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //提交文件
				if(commit_file(&curCollectConf, szContent[8], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //记录日志
				if(run_log(&curCollectConf,szFileTimeStampPre,szContent[8],lFileSize,szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //给时间戳到临时时间端点
				if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
				{
					strcpy(szTempTimePoint, szFileTimeStamp);
				}
			}
		}
    }
	else if(0 == strncmp("hw", curCollectConf.company, 2))
	{
        //hw:获取目录列表,挨个进入目录获取文件列表,然后对比文件时间和时间端点,如果大
		/* 删除旧的目录列表文件 */
		if( 0 == access(szDirListFile, F_OK) )
		{
			if(0 != unlink(szDirListFile))
			{
				err_log("point_collect: unlink %s fail\n",szDirListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* 建立Ftp连接 */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,1,g_nDebug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* 获取最新的目录列表 */
		nRetVal = Ftp_Dir(szDirListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* 关闭Ftp连接 */
		Ftp_Close();

		//打开文件
		pFileDirList = fopen( szDirListFile , "r" );
		if(pFileDirList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szDirListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//循环进入目录
		while (1)
		{		
			//获取一行配置
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileDirList) == NULL )
				break;

			//应该是9个参数
			if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
				szContent[5],szContent[6],szContent[7],szContent[8]) != 9 )
				continue;

			//判断是否要进入
			if ( 'd' != szContent[0][0] ) //目录
			{
				continue;	
			}
			if ( '2' != szContent[8][0] || '0' != szContent[8][1] )	//以20开头
			{
				continue;	
			}
			if ( 8 != strlen(szContent[8])) //8位长
			{
				continue;	
			}

			strcpy(szFileTimeStampPre, szContent[8]);
			if ( 0 < strncmp(szTimePointPre, szFileTimeStampPre, 8))
			{
				continue;	
			}
	
			/* 删除旧的下载文件列表文件 */
			if( 0 == access(szFileListFile, F_OK) )
			{
				if(0 != unlink(szFileListFile))
				{
					err_log("point_collect: unlink %s fail\n",szDirListFile);
					nRet = 1;		
					goto Exit_Pro;
				}
			}			
		
			/* 建立Ftp连接 */
			nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
				curCollectConf.port,FTP_TIME_OUT,1,1,g_nDebug);
			if(nRetVal != 0)
			{
				err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
					curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
				nRet = 1;		
				goto Exit_Pro;
			}

			//进入目录
			nRetVal = Ftp_Cd(szContent[8]);
			if( 0 != nRetVal )
			{
				err_log("point_collect: Ftp_Cd fail,collect_point=%d,dir=%s\n",curCollectConf.collect_point,szContent[8]);
				nRet = 1;		
				goto Exit_Pro;
			}

			/* 获取最新的下载文件列表 */
			nRetVal = Ftp_Dir(szFileListFile);
			if( 0 != nRetVal )
			{
				err_log("point_collect: collect_point=%d,Ftp_Dir fail",curCollectConf.collect_point);
				nRet = 1;		
				goto Exit_Pro;
			}

            /* 关闭Ftp连接 */
			Ftp_Close();

			//打开文件
			pFileFileList = fopen( szFileListFile , "r" );
			if(szFileListFile == NULL)
			{
				err_log("point_collect: fopen %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}

			//循环下载
			while (1)
			{
				//获取一行配置
				memset(szBuff, 0 , sizeof(szBuff));
				memset(szContent, 0 , sizeof(szContent));
				if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				{
					break;
				}

				//应该是9个参数
				if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
					szContent[5],szContent[6],szContent[7],szContent[8]) != 9 )
					continue;

                //如果是旧的话单文件，就不再下载
                if(strncmp(szContent[8], "gz", 2) != 0)
                {
                    continue;
                }

				//判断是否要下载
				if ( '-' != szContent[0][0] ) //文件
				{
					continue;	
				}
				if (0 != convert_date_hw(szFileTimeStamp, szContent[7], szFileTimeStampPre))//拿到文件时间戳
				{
					continue;
				}
				if ( 0 < strncmp(szTimePoint, szFileTimeStamp, 14)) //比较时间
				{
					continue;	
				}

				if(get_backup_name(&curCollectConf,szContent[8],szFileTimeStamp,szBackupFile)!=0) //获取备份文件名
				{
					err_log("point_collect: get_backup_name fail\n");
					nRet = 1;
					goto Exit_Pro;
				}
				if(0 == access(szBackupFile,F_OK))  //文件存在就不下载
				{
					//给时间戳到临时时间端点
					if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
					{
						strcpy(szTempTimePoint, szFileTimeStamp);
					}
					continue;
				}

                //下载文件
				if(get_file_passive(&curCollectConf, szFileTimeStampPre, szContent[8], &lFileSize)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_passive FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //备份文件
				if(backup_file(&curCollectConf, szContent[8], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //提交文件
				if(commit_file(&curCollectConf, szContent[8], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //记录日志
				if(run_log(&curCollectConf,szFileTimeStampPre,szContent[8],lFileSize,szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //给时间戳到临时时间端点
				if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
				{
					strcpy(szTempTimePoint, szFileTimeStamp);
				}
			}
		}
	}
	else if(0 == strncmp("nsn", curCollectConf.company, 3))
	{
        //nsn:获取文件列表,对比文件时间和时间端点,如果大
		/* 删除旧的目录列表文件 */
		if( 0 == access(szFileListFile, F_OK) )
		{
			if(0 != unlink(szFileListFile))
			{
				err_log("point_collect: unlink %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* 建立Ftp连接 */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,1,g_nDebug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* 获取最新的目录列表 */
		nRetVal = Ftp_Dir(szFileListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* 关闭Ftp连接 */
		Ftp_Close();

		//打开文件
		pFileFileList = fopen( szFileListFile , "r" );
		if(pFileFileList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szFileListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//循环下载
		while (1)
		{
			//获取一行配置
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				break;

			//应该是6个参数
			if( sscanf(szBuff,"%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],
				szContent[4],szContent[5]) != 6 )
				continue;

			//判断是否要下载
			if ( 10 != strlen(szContent[5]) )//文件名10个字符
			{
				continue;	
			}			
			if ( '.' != szContent[5][6] || 'D' != szContent[5][7] || 
				 'A' != szContent[5][8] || 'T' != szContent[5][9] ) //文件名尾固定.DAT
			{
				continue;	
			}
			if (0 != convert_date_nsn(szFileTimeStamp, szContent[0], szContent[1]))//拿到文件时间戳
			{
				continue;
			}
			if ( 0 < strncmp(szTimePoint, szFileTimeStamp, 14)) //比较时间
			{
				continue;	
			}

			if(get_backup_name(&curCollectConf,szContent[5],szFileTimeStamp,szBackupFile)!=0) //获取备份文件名
			{
				err_log("point_collect: get_backup_name fail\n");
				nRet = 1;
				goto Exit_Pro;
			}

			if(0 == access(szBackupFile,F_OK))  //文件存在就不下载
			{
				//给时间戳到临时时间端点
				if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
				{
					strcpy(szTempTimePoint, szFileTimeStamp);
				}
				continue;
			}

			//下载文件
			if(get_file_passive(&curCollectConf, NULL, szContent[5], &lFileSize) != 0 )
			{
				err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_passive FAIL\n",nCollectPointNo,szContent[5]);
				nRet = 1;
				goto Exit_Pro;
			}
			
			//备份文件
			if(backup_file(&curCollectConf, szContent[5], szFileTimeStamp)!=0)
			{
				err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[5]);
				nRet = 1;
				goto Exit_Pro;
			}

			//提交文件
			if(commit_file(&curCollectConf, szContent[5], szFileTimeStamp)!=0)
			{
				err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[5]);
				nRet = 1;
				goto Exit_Pro;
			}

			//记录日志
			if(run_log(&curCollectConf,NULL,szContent[5],lFileSize,szFileTimeStamp)!=0)
			{
				err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[5]);
				nRet = 1;
				goto Exit_Pro;
			}
		
			//给时间戳到临时时间端点
			if ( 0 > strncmp(szTempTimePoint, szFileTimeStamp, 14)) //大者写入
			{
				strcpy(szTempTimePoint, szFileTimeStamp);
			}
		}	
	}
    else
    {
        err_log("point_collect: unknown vendor device, vendor = %s, msc = %s\n", curCollectConf.company, curCollectConf.device);
        nRet = 1;
        goto Exit_Pro;
    }

    //清理	
	if ( strncmp(szTimePoint, szTempTimePoint ,14) <= 0 )
	{
		if (pFileTimePoint != NULL)
		{
			fclose(pFileTimePoint);
			pFileTimePoint = NULL;
		}

		pFileTimePoint = fopen( szTimePointFile , "w" );
		if(pFileTimePoint == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szFileListFile);
			nRet = 1;
			goto Exit_Pro;
		}
		fprintf(pFileTimePoint, "%s", szTempTimePoint);
		fclose(pFileTimePoint);
		pFileTimePoint = NULL;
	}

	//清理work目录下,xxx_ 开头文件,xxx为当前子进程编号
	memset(file_name,0,sizeof(file_name));
	sprintf(pre,"%03d",nCurrentProcessNo);
	while(get_orig_file_name(WORK,pre,NULL,file_name)==PARSE_MATCH)
	{
		sprintf(tmp_file_name,"%s/%s",WORK,file_name);
		
		if ( unlink(tmp_file_name)!=0 )
		{
			err_log("point_collect: unlink file %s fail\n",tmp_file_name);
			nRet=1;
			goto Exit_Pro;
		}
		memset(file_name,0,sizeof(file_name));
	}

Exit_Pro:
	Ftp_Close();
	if (pFileDirList != NULL)
	{
		fclose(pFileDirList);
		pFileDirList = NULL;
	}
	if (pFileFileList != NULL)
	{
		fclose(pFileFileList);
		pFileFileList = NULL;
	}
	if (pFileTimePoint != NULL)
	{
		fclose(pFileTimePoint);
		pFileTimePoint = NULL;
	}
	return nRet;
}

static int convert_date_hw(char* lpOutDateTime, const char* lpInTime, const char* lpInDate)
{
	/************************************************************************/
	/* lpInDate: 20100520 */
	/* lpInTime: 1:55 */
	/* lpOutDateTime: 20100520015500 */
	/************************************************************************/
	if ( 4 == strlen(lpInTime)) //1:55
	{
		sprintf(lpOutDateTime,"%s0%c%c%c00", 
			lpInDate, lpInTime[0], lpInTime[2], lpInTime[3]);
	}
	else if (5 == strlen(lpInTime)) //12:14
	{
		sprintf(lpOutDateTime,"%s%c%c%c%c00", 
			lpInDate, lpInTime[0], lpInTime[1], lpInTime[3], lpInTime[4]);
	}
	else
	{
        err_log("convert_date_hw: time format incorrect: %s\n", lpInTime);
        return 1;
	}
	return 0;
}

static int convert_date_hw_sp6(char* lpOutDateTime, char* lpInTime, const char* lpInDate)
{
	/************************************************************************/
	/* lpInDate: 20100520 */
	/* lpInTime: 09:07AM 12:45AM 12:45PM*/
	/* lpOutDateTime: 20100520015500 */
	/************************************************************************/
	if ('1' == lpInTime[0] && '2' == lpInTime[1] )
	{
		//12点 为 0点
		lpInTime[0] = '0';
		lpInTime[1] = '0';
	}
	if ('P' == lpInTime[5])
	{
		//下午
		//时间 + 12
		lpInTime[0] += 1; 
		lpInTime[1] += 2;
		if (lpInTime[1] > '9')
		{
			lpInTime[0] += 1;
			lpInTime[1] -= 10;
		}
	}
	sprintf(lpOutDateTime, "%s%c%c%c%c00", 
		lpInDate, lpInTime[0], lpInTime[1], lpInTime[3], lpInTime[4]);

	return 0;
}

static int convert_date_nsn(char* lpOutDateTime, const char* lpInTime, const char* lpInDate)
{
/************************************************************************/
/* lpInDate: 29.08.2005 */
/* lpInTime: 13.58:57 */
/* lpOutDateTime: 20100520015500 */
/************************************************************************/
	sprintf(lpOutDateTime,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 
		lpInDate[6],lpInDate[7],lpInDate[8],lpInDate[9],lpInDate[3],lpInDate[4],lpInDate[0],lpInDate[1],
		lpInTime[0], lpInTime[1], lpInTime[3],lpInTime[4], lpInTime[6], lpInTime[7]);
	return 0;
}

static int get_file_passive(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long * file_size)
{
	int  ret;
	int  ftp_ret;
	long succ_bytes;
	char tmp_file_name[MAX_LONG_FILENAME];
	
	ret=0;

    /* 如果数据在本地已经存在，先删除　*/
	sprintf(tmp_file_name,"%s/%03d_%s",WORK,p_collect_conf->current_process_number,remote_file_name);
	if( 0 == access(tmp_file_name, F_OK) )
	{
		if(0 != unlink(tmp_file_name))
		{
			err_log("get_file_passive: unlink %s fail\n",tmp_file_name);
			ret = 1;		
			goto Exit_Pro;
		}
	}

	/* 建立Ftp连接 */
	ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,p_collect_conf->port,FTP_TIME_OUT,1,1,g_nDebug);
	if(ftp_ret!=0)
	{
		err_log("get_file_passive: Ftp_Init fail,collect_point=%d\n",p_collect_conf->collect_point);
		ret=1;
		goto Exit_Pro;
	}

	/* 如果指定了远程目录，则跳转到该目录 */
	if(remote_path != NULL && strlen(remote_path) > 0)
	{
		ftp_ret = Ftp_Cd(remote_path);
		if(ftp_ret!=0)
		{
			err_log("get_file_passive: Ftp_Cd fail,collect_point=%d,dir=%s\n",p_collect_conf->collect_point,remote_path);
			ret=1;
			goto Exit_Pro;
		}
	}

	/* Ftp_Receive() */
	succ_bytes=0;
	ftp_ret = Ftp_Receive(remote_file_name, tmp_file_name, 0, &succ_bytes);
	if(ftp_ret!=0)
	{
		err_log("get_file_passive: Ftp_Receive fail,collect_point=%d,file=%s\n",p_collect_conf->collect_point,remote_file_name);
		ret=1;
		goto Exit_Pro;
	}
	
	*file_size=succ_bytes;

Exit_Pro:
	Ftp_Close();
	return ret;
}
static int get_file_port(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long * file_size)
{
	int  ret;
	int  ftp_ret;
	long succ_bytes;
	char tmp_file_name[MAX_LONG_FILENAME];
	
	ret=0;

    /* 如果数据在本地已经存在，先删除　*/
	sprintf(tmp_file_name,"%s/%03d_%s",WORK,p_collect_conf->current_process_number,remote_file_name);
	if( 0 == access(tmp_file_name, F_OK) )
	{
		if(0 != unlink(tmp_file_name))
		{
			err_log("get_file_port: unlink %s fail\n",tmp_file_name);
			ret = 1;		
			goto Exit_Pro;
		}
	}

	/* 建立Ftp连接 */
	ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,p_collect_conf->port,FTP_TIME_OUT,1,0,g_nDebug);
	if(ftp_ret!=0)
	{
		err_log("get_file_port: Ftp_Init fail,collect_point=%d\n",p_collect_conf->collect_point);
		ret=1;
		goto Exit_Pro;
	}

	/* 如果指定了远程目录，则跳转到该目录 */
	if(remote_path != NULL && strlen(remote_path) > 0)
	{
		ftp_ret = Ftp_Cd(remote_path);
		if(ftp_ret!=0)
		{
			err_log("get_file_port: Ftp_Cd fail,collect_point=%d,dir=%s\n",p_collect_conf->collect_point,remote_path);
			ret=1;
			goto Exit_Pro;
		}
	}

	/* Ftp_Receive() */
	succ_bytes=0;
	ftp_ret = Ftp_Receive(remote_file_name, tmp_file_name, 0, &succ_bytes);
	if(ftp_ret!=0)
	{
		err_log("get_file_port: Ftp_Receive fail,collect_point=%d,file=%s\n",p_collect_conf->collect_point,remote_file_name);
		ret=1;
		goto Exit_Pro;
	}
	
	*file_size=succ_bytes;

Exit_Pro:
	Ftp_Close();
	return ret;
}

static int backup_file(collect_conf * pCollectConf, const char * szRemoteFileName, const char* lpTimeStamp)
{
	char szBackupName[MAX_LONG_FILENAME];
	char szTempFileName[MAX_LONG_FILENAME];
    char szCommand[MAX_BUFFER];
	
	/*判断是否备份*/ 
	if(pCollectConf->is_backup)
	{
		sprintf(szTempFileName,"%s/%03d_%s",WORK,pCollectConf->current_process_number,szRemoteFileName);
		
		memset(szBackupName,0,sizeof(szBackupName));
		if(get_backup_name(pCollectConf,szRemoteFileName,lpTimeStamp,szBackupName)!=0)
		{
			err_log("backup_file: get_backup_name fail\n");
            return 1;
		}

        sprintf(szCommand, "cp %s %s", szTempFileName, szBackupName);
        if(system(szCommand) == -1)
        {
			err_log("backup_file: link %s to %s fail\n", szTempFileName, szBackupName);
            return 1;
        }
	} 	
	return 0;
}

static int get_backup_name(collect_conf * pCollectConf,const char * szRemoteFileName,const char* lpTimeStamp,char * szBackupName)
{
	char   szTemp[MAX_LONG_FILENAME];

	sprintf(szTemp,"%s/%s/%s",pCollectConf->backup_path,pCollectConf->company,pCollectConf->device);
	sprintf(szBackupName,"%s/%06d_%s_%s_%s_%s",szTemp,pCollectConf->collect_point,pCollectConf->company,pCollectConf->device,lpTimeStamp,szRemoteFileName);
	if(access(szTemp,F_OK) == -1)
	{
		if(mkdir(szTemp,0755)!=0)
		{
			err_log("get_backup_name: mkdir %s fail\n",szTemp);
            return 1;
		}
	}
	return 0;
}

static int commit_file(collect_conf * p_collect_conf, const char * remote_file_name, const char* lpTimeStamp)
{
	char  commit_name[MAX_FILENAME];
	char  tmp_file_name[MAX_LONG_FILENAME];
	char  tmp_file_name_2[MAX_LONG_FILENAME];
    char  szCommand[MAX_BUFFER];

    //判断是否提交
    if(p_collect_conf->is_commit) 
    {
        //link工作目录文件至提交目录
        memset(commit_name,0,sizeof(commit_name));
		sprintf(commit_name,"@%06d_%s_%s_%s_%s",p_collect_conf->collect_point,p_collect_conf->company,p_collect_conf->device,lpTimeStamp,remote_file_name);

		sprintf(tmp_file_name,"%s/%03d_%s",WORK,p_collect_conf->current_process_number,remote_file_name);
		sprintf(tmp_file_name_2,"%s/%s",g_szFileCommitDir,commit_name);
  
		if(access(tmp_file_name_2,F_OK)==-1)
		{
            /* 如果文件以gz开头,则进行解压 */
            if( 'g' == remote_file_name[0] && 'z' == remote_file_name[1] )
            {
                sprintf(szCommand, "gzip -S .dat -d %s", tmp_file_name);
                if(system(szCommand) == -1)
                {
                    err_log("commit_file: uncompress %s fail\n", tmp_file_name);
                    return 1;
                }
                tmp_file_name[strlen(tmp_file_name) - 4] = '\0';
            }
     
            if(link(tmp_file_name,tmp_file_name_2)!=0)
			{
				err_log("commit_file: link %s to %s fail\n",tmp_file_name,tmp_file_name_2);
                return 1;
			}
		}
		else
		{
			err_log("commit_file: target file %s exist\n",tmp_file_name_2);
		}
    }
	return 0;
}

//记录collect_run.{ne_name}.{YYYYMMDD}文件
static int run_log(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long file_size, const char* lpTimeStamp)
{
	char  local_file_name[MAX_FILENAME];
	char  tmp_file_name[MAX_LONG_FILENAME];
	char  run_log_time[MAX_TIME];
	char  commit_time[MAX_TIME];
	FILE* fp = NULL;

	get_time(run_log_time);
	get_time(commit_time);
	run_log_time[8]='\0';
	
    sprintf(local_file_name,"%06d_%s_%s_%s_%s",p_collect_conf->collect_point,p_collect_conf->company,p_collect_conf->device,lpTimeStamp,remote_file_name);
	sprintf(tmp_file_name,"%s.%s.%s",PREFIX_RUN_LOG_FILE,p_collect_conf->device,run_log_time);
	
	fp=fopen(tmp_file_name,"a+");
	if(fp==NULL)
	{
		err_log("run_log: fopen %s fail\n",tmp_file_name);
        return 1;
	}
	if(remote_path != NULL && strlen(remote_path) > 0)
	{
		fprintf(fp, "<%06d> %s %s %s %s %s %ld %s %s %d %d\n",
                p_collect_conf->collect_point,
                p_collect_conf->device,
                p_collect_conf->ip,
                remote_path,
			    remote_file_name,
                local_file_name,
                file_size,
                lpTimeStamp,
                commit_time,
                p_collect_conf->is_backup,
                p_collect_conf->is_commit);
	}
	else
	{
		fprintf(fp, "<%06d> %s %s . %s %s %ld %s %s %d %d\n",
                p_collect_conf->collect_point,
                p_collect_conf->device,
                p_collect_conf->ip,
			    remote_file_name,
                local_file_name,
                file_size,
                lpTimeStamp,
                commit_time,
                p_collect_conf->is_backup,
                p_collect_conf->is_commit);
	}
	fclose(fp);
	fp=NULL;
    return 0;
}

static int gen_recollect_record(collect_conf * p_collect_conf, 
                        const char * start_time, const char * end_time)
{
    char   recollect_conf[MAX_FILENAME];
    FILE   *fp = NULL;

    if(g_szRecollectRunDir == NULL)
        return 1;

    sprintf(recollect_conf, "%s/conf/collect.conf", g_szRecollectRunDir);
    fp = fopen(recollect_conf, "a+");
    if(fp == NULL)
    {
        err_log("gen_recollect_record: fopen %s fail\n", recollect_conf);
        return 1;
    }

    fprintf(fp, "<%06d> %s %s %s %d %s %s %s %s %s %s %s %s %s\n",
            p_collect_conf->collect_point,
            p_collect_conf->company,
            p_collect_conf->device,
            p_collect_conf->ip,
            p_collect_conf->port,
            p_collect_conf->usr,
            p_collect_conf->password,
            p_collect_conf->path_str,
            p_collect_conf->file_str,
            p_collect_conf->backup_path,
            (p_collect_conf->is_commit ? "yes" : "no"),
            (p_collect_conf->is_interval_file ? "yes" : "no"),
            start_time, end_time);

    fclose(fp);
    fp = NULL;
    return 0;
}

static void sigint_handler(int signum)
{
    if(signum == SIGINT)
    {
        g_term_flag = 1;
    }
}

