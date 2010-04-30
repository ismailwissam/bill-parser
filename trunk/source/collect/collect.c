/*************************************************/
/*                                               */
/* collect.c                                     */
/*                                               */ 
/*          ftp采集程序 v1.0 2005.05 haojianting */
/*                      v1.1 2010.04 刘洋        */
/*v1.1  ChangeLog:                               */
/*v1.1: * 修改变量命名,使用匈牙利命名法          */
/*v1.1: * 添加大量注释,增加代码可读性            */
/*v1.1: * 添加大量Debug输出,增加代码可调性       */
/*v1.1: * 程序结构优化                           */
/*v1.1: * 增加FTP端口设置,使可连接非默认端口FTP  */
/*************************************************/


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
#include <setjmp.h>
#include "Ftp.h"

/************************************************************************/
/* 宏定义 */
/************************************************************************/
//FTP超时时间
#define      FTP_TIME_OUT      300
//最大子进程数
#define      MAX_CHILD_PROCESS 128
//进程初始时间
#define      SLEEP_TIME        60
//Work,Log,Conf目录
#define      WORK     "./work"
#define      LOG      "./log"
#define      CONF     "./conf"
//Log文件
#define      ERR_LOG_FILE        "./log/collect_err.log"
#define      PREFIX_RUN_LOG_FILE "./log/collect_run"


/************************************************************************/
/* 全局变量定义 */
/************************************************************************/
/*用于每个子进程设置其编号*/
int          g_nCurrentProcessNumber    = 0;

char *       g_szProgName               = NULL;
char *       g_szFileCommitDir        = "./data";
char *       g_szRunDir                = "./";
int          g_nDebug                  = 1;	//TODO:发布时将此值修改为0
int          g_nParallelChildProcess = 1;

/************************************************************************/
/* 结构体定义 */
/************************************************************************/
//子进程id及时间
typedef struct 
{
	int pid;
	int sleep_time;
} t_child_process_status;

//FTP采集信息
typedef struct {
	//编号
	int   collect_point;
	
	//登陆信息,v1.1添加端口port
	char  ip[256];
	int   port; //端口port:华为6621
	char  usr[256];
	char  password[256];
	//各种目录
	char  path_str[256];
	char  path_up[256];
	char  path_last[256];
	char  path_pre[256];
	char  path_suf[256];
	int   is_multi_path;  
	//各种文件
	char  file_str[256];
	char  file_pre[256];
	char  file_suf[256];
	//备份目录
	char  backup_path[256];
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
static void  usage(int status);
int          get_time(char * par);
void         err_log(char * format,...);
void         daemon_start(void);

parse_code_e pre_suf_check(char * name,char * prefix,char * suffix);
parse_code_e get_orig_file_name(char * dir_name,char * prefix,char * suffix,char * ret_file_name);

int          verify_collect_conf(void);

parse_code_e get_collect_conf(int lineno_conf,int current_process_number,collect_conf * p_collect_conf);

parse_code_e parse_get_file_name(collect_conf * p_collect_conf,char * curr_file,char * p_file_name);
parse_code_e parse_get_dir_name(collect_conf * p_collect_conf,char * curr_dir,char * p_dir_name);
parse_code_e parse_get_file_name_2(collect_conf * p_collect_conf,char * curr_dir,char * curr_file,char * p_dir_name,char * p_file_name);
parse_code_e parse_collect_conf(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name);

int          get_remote_file(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name,long * file_size);

int          get_backup_name(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name,char * backup_name);
int          backup_file(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name);

int          get_commit_name(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name,char * commit_name);
int          commit_file(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name,char * begin,char * end,long file_size);

int          point_collect(int lineno_conf,int current_process_number);
void         process_collect(int current_number,int parallel_number);

/*
*  Display the syntax for starting this program.
*/
static void usage(int status)
{
	FILE *output = status?stderr:stdout;
	
	fprintf(output,"Usage: %s [-o file commit path] [-r run path] [-p parallel child number] [-d]\n",g_szProgName);
	fprintf(output,"\nOptions:\n");
	fprintf(output,"        -o changes the file commit directory to that specified in path, default is ./data\n");
	fprintf(output,"        -r changes the run directory to that specified in path, default is ./\n");
	fprintf(output,"        -p changes the parallel child number, default is 1\n");
	fprintf(output,"        -d         debug flag\n");
	
	exit(status);
}

int  get_time(char * par)
{
	time_t t;
	struct tm *systime;
	
	t=time(NULL);
	if(t==-1)
	{
		strcpy(par,"yyyymmddhhmiss");
		return 1;
	}
	
	systime=localtime(&t);
	if(systime==NULL)
	{
		strcpy(par,"yyyymmddhhmiss");
		return 1;
	}
	
	sprintf(par,"%04d%02d%02d%02d%02d%02d",systime->tm_year+1900,systime->tm_mon+1,\
		systime->tm_mday,systime->tm_hour,systime->tm_min,systime->tm_sec);
	
	return 0;
}

/************************************************************************/
/* 错误日志记录函数 */
/************************************************************************/
void err_log(char * format,...)
{
	time_t t;
	struct tm *systime;
	FILE *fp;
	
	char   tmp[256];
	
	va_list ap;
	
// 	if(g_nDebug)
// 	{
// 		va_start(ap, format);
// 		vprintf(format,ap);
// 		va_end(ap);
// 		printf("\n");
// 	}
	
	if(g_nCurrentProcessNumber!=0)
	{
		sprintf(tmp,"%s.%03d",ERR_LOG_FILE,g_nCurrentProcessNumber);
		fp=fopen(tmp,"a+");
	}
	else
		fp=fopen(ERR_LOG_FILE,"a+");
	
	if(fp==NULL)
	{
		return;
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
	
	return;
}

/************************************************************************/
/* 开启守护进程 */
/************************************************************************/
void daemon_start(void)
{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("2001:daemon_start in.\n");	

	int childpid;
	
	umask(022);
	
	if(getppid() == 1 ) 
	{
		return; 
	}
	
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	
	if(setpgrp()==-1)
		err_log("daemon_start: can't change process group\n");
	signal(SIGHUP,SIG_IGN);
	
	
	if((childpid=fork())<0) 
	{
		err_log("daemon_start: fork error\n");
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("9014:daemon_start Fork error.\n");	
		exit(1);
	}
	else if(childpid > 0)
	{
		
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("2002:daemon_start out AND Main process exit.\n");	
		exit(0);
	}
	else if (childpid == 0)
	{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("2003:Sub process Start:%d.\n",(int)getpid());	
	}
	
}

/************************************************************************/
/* 入口函数*/
/************************************************************************/
int main(int argc,char * argv[])
{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("1001:System start.\n");

	int     nArgVal;
	struct  stat stat_buff;
	char    szFileName[256];
	char    szTmpFileName[512];
	int     nConfLine;
	int     i;
	t_child_process_status oChildProcessStatus[MAX_CHILD_PROCESS];
	
	pid_t   pidChild;
	pid_t   pidReValWait;
	
	if (( g_szProgName = strrchr(argv[0], '/')) == NULL)
		g_szProgName = argv[0];
	else
		g_szProgName++;
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("1002:Parse the option and Check the dir.\n");
	
	/*  Process the options.  */
	while ((nArgVal = getopt(argc, argv, "o:r:p:d")) != EOF) {
		
		switch(nArgVal) 
		{
			
		case 'o':
			g_szFileCommitDir = strdup(optarg);
			break;	
		case 'r':
			g_szRunDir = strdup(optarg);
			break;	
		case 'p':
			g_nParallelChildProcess = atoi(optarg);
			break;
		case 'd':
			g_nDebug = 1;
			break;	
		default:
			usage(1);
			break;
			
		}
	}
	
	/* Change dir */
	if(chdir(g_szRunDir)==-1)
	{
		err_log("main: chdir to %s fail\n",g_szRunDir);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("9001:Change dir failed.\n");
		return 1;
	}

	/* Check dir WORK*/
	if(stat(WORK,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",WORK);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("9002:Check WORK dir failed.1\n");
		return 1;
	}
	else
	{
		if(!S_ISDIR(stat_buff.st_mode))	
		{
 			err_log("main: %s isn't a dir\n",WORK);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("9003:Check WORK dir failed.2\n");
			return 1;
		}
	}
	
	/* Check dir LOG*/
	if(stat(LOG,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",LOG);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("9004:Check LOG dir failed.1\n");
		return 1;
	}
	else
	{
		if(!S_ISDIR(stat_buff.st_mode))	
		{
			err_log("main: %s isn't a dir\n",LOG);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("9005:Check LOG dir failed.1\n");
			return 1;
		}
	}
	
	/* Check dir CONF*/
	if(stat(CONF,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",CONF);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("9006:Check CONF dir failed.1\n");
		return 1;
	}
	else
	{
		if(!S_ISDIR(stat_buff.st_mode))	
		{
			err_log("main: %s isn't a dir\n",CONF);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("9007:Check CONF dir failed.2\n");
			return 1;
		}
	}
	
	/* Check dir g_szFileCommitDir*/
	if(stat(g_szFileCommitDir,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",g_szFileCommitDir);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("9008:Check COMMIT dir failed.1\n");

		return 1;
	}
	else
	{
		if(!S_ISDIR(stat_buff.st_mode))	
		{
			err_log("main: %s isn't a dir\n",g_szFileCommitDir);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("9009:Check COMMIT dir failed.2\n");	
			return 1;
		}
	}

/************************************************************************/
/* 获取采集配置 */
/************************************************************************/
	/*verify_collect_conf*/
	nConfLine = verify_collect_conf();
	if(nConfLine<=0)
	{
		err_log("main: verify_collect_conf=%d,fail\n",nConfLine);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("9010:Verify collect conf failed.\n");	

		return 1;
	}
/************************************************************************/
/* 子进程数目要小于最大进程数和配置文件行数,大于0 */
/************************************************************************/
	/*verify g_nParallelChildProcess*/
	if(g_nParallelChildProcess<=0)                g_nParallelChildProcess=1;
	if(g_nParallelChildProcess>MAX_CHILD_PROCESS) g_nParallelChildProcess=MAX_CHILD_PROCESS;
		if(g_nParallelChildProcess>nConfLine)
		g_nParallelChildProcess=nConfLine;
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("1003:Check Finished.\n-FileCommitDir:%s\n-RunDir:%s\n-ParallelChildProcess: %d\n-Clean WORK dir.\n",g_szFileCommitDir,g_szRunDir,g_nParallelChildProcess);	

	/*清理work目录下文件*/
	memset(szFileName, 0, sizeof(szFileName));
	while( get_orig_file_name(WORK,NULL,NULL,szFileName) == PARSE_MATCH )
	{
		sprintf(szTmpFileName,"%s/%s",WORK,szFileName);
		
		err_log("main: clear file:%s#\n",szTmpFileName);
		if ( unlink(szTmpFileName) != 0 )
		{
			err_log("main: unlink file %s fail\n",szTmpFileName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("9013:Unlink file fail.\n");	

			return 1;
		}
		memset(szFileName,0,sizeof(szFileName));
	}

/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("1004:Init sub_process pool.\n");	
	
	/*初始化 子进程结构数组*/
	for(i=0;i<MAX_CHILD_PROCESS;i++)
		memset(&oChildProcessStatus[i],0,sizeof(t_child_process_status));
	
	daemon_start();
/************************************************************************/
/* 主程序到这里已经推出,守护进程继续执行 */
/************************************************************************/
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("1005:Deamon Process while(1) in.\n");	

	/*采集处理*/
	while(1)
	{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
// 		if(g_nDebug)	printf("Deamon|BEEP!!!\n");	

		/*创建子进程*/
		for(i=0;i<g_nParallelChildProcess;i++)
		{
			if(oChildProcessStatus[i].pid==0&&oChildProcessStatus[i].sleep_time<=0)
			{
				if((pidChild=fork())<0) /*fork error*/
				{
					err_log("main: fork error\n");
/************************************************************************/
/*  Alur changed */
/************************************************************************/
					if(g_nDebug)	printf("9011:Fork error.\n");	
					exit(1);
				}
				else if(pidChild>0)     /**/ 
				{
/************************************************************************/
/*  守护进程:记录子进程信息,设置子进程初始时间 */
/************************************************************************/
					oChildProcessStatus[i].pid        = pidChild;
					oChildProcessStatus[i].sleep_time = SLEEP_TIME;
/************************************************************************/
/*  Alur changed */
/************************************************************************/
					if(g_nDebug)	printf("1006:Create sub_process %d.\n",(int)pidChild);	
				}
				else if(pidChild==0)    /*child process*/
				{
/************************************************************************/
/* 子进程:干活吧!搬东西!! */
/************************************************************************/
					process_collect(i+1,g_nParallelChildProcess);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
					if(g_nDebug)	printf("1007:Child process dead:%d.\n",(int)getpid());	
					exit(0);
				}
			}
		}
		
		/*回收子进程*/
		for(i=0;i<g_nParallelChildProcess;i++)
		{
			if(oChildProcessStatus[i].pid>0)
			{
/************************************************************************/
/* 查询已经结束的子进程,若没有返回0 */
/************************************************************************/
				pidReValWait = waitpid(oChildProcessStatus[i].pid,NULL,WNOHANG);
				if(pidReValWait>0)
				{
/************************************************************************/
/* 有结束的子进程,情况子进程pid */
/************************************************************************/
					oChildProcessStatus[i].pid=0;
				}
				if(pidReValWait<0)
				{
					err_log("main: waitpid fail\n");
/************************************************************************/
/*  Alur changed */
/************************************************************************/
					if(g_nDebug)	printf("9012:Wait pid fail.\n");	
					exit(1);
				}
			}
		}
		
/************************************************************************/
/* 等一秒 */
/************************************************************************/
		sleep(1);
/************************************************************************/
/* 将所有子进程的时间减一秒 */
/************************************************************************/		
		/*每秒递减sleep_time*/
		for( i=0 ; i < g_nParallelChildProcess; i++ )
		{
			if( oChildProcessStatus[i].pid == 0 && oChildProcessStatus[i].sleep_time > 0 )
			{	
				oChildProcessStatus[i].sleep_time--;
			}
		}
	}
	
	return 0;
}



/*
*  pre_suf_check()
*/
parse_code_e pre_suf_check(char * name,char * prefix,char * suffix)
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
parse_code_e get_orig_file_name(char * dir_name,char * prefix,char * suffix,char * ret_file_name)
{
	int              ret;
	DIR *            pdir=NULL;
	struct dirent *	 pdirent;
	struct stat stat_buff;
	char             tmp_file_name[512];
	
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
		/*前后缀匹配检查*/
		if(prefix!=NULL)
		{
			if(strstr(pdirent->d_name,prefix)!=pdirent->d_name)
				continue;
		}
		if(suffix!=NULL)
		{
			if(strlen(pdirent->d_name)<strlen(suffix))
				continue;
			if(strcmp( (pdirent->d_name+strlen(pdirent->d_name)-strlen(suffix)),suffix) != 0 )
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
/* 获取采集配置函数 */
/************************************************************************/
/*
*verify_collect_conf()
*                      return -1 fail,>=0 success line
*
*/
int verify_collect_conf(void)
{

/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("3001:verify_collect_conf in.\n");	

	int           r = 0;
	FILE        * pFile=NULL;
	char          szTempFileName[256];
	char	      szBuff[2048];
	char          szContent[10][256];
	int           nLen;
	
	sprintf(szTempFileName,"%s/collect.conf",CONF);

/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("3002:Try to open file: %s.\n",szTempFileName);	

	pFile = fopen(szTempFileName,"r");
	if(pFile==NULL)
	{
		err_log("verify_collect_conf: fopen %s fail\n",szTempFileName);
		r = -1;
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("3101:Open file failed.\n");	

		goto Exit_Pro;
	}
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("3003:Start while(1).\n");	
	
	while(1)
	{
		memset(szBuff,0,sizeof(szBuff));
		memset(szContent,0,sizeof(szContent));
		if(fgets(szBuff,sizeof(szBuff),pFile)==NULL)
			break;
		
		r++;
		
		if(szBuff[0]!='#')
		{     	
			if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],\
				szContent[4],szContent[5],szContent[6],szContent[7],szContent[8],szContent[9]) != 10 )
			{
				err_log("verify_collect_conf: collect.conf line %d incorrect\n",r);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("3102:Conf file line %d incorrect.1\n",r);	

				r=-1;
				goto Exit_Pro;
			}
			
			nLen=strlen(szContent[0]);
			if(nLen!=8 || szContent[0][0]!='<' || szContent[0][nLen-1]!='>')
			{
				err_log("verify_collect_conf: collect.conf line %d incorrect\n",r);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("3103:Conf file line %d incorrect.2\n",r);	

				r=-1;
				goto Exit_Pro;
			}
			
			szContent[0][nLen-1]='\0';
			if(atoi(&szContent[0][1])!=r)
			{
				err_log("verify_collect_conf: collect.conf line %d incorrect\n",r);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("3104:Conf file line %d incorrect.3\n",r);	

				r=-1;
				goto Exit_Pro;
			}
		}
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("3004:Parsing Result:\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n",
								szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
								szContent[5],szContent[6],szContent[7],szContent[8],szContent[9]);	
		
	}
	
	fclose(pFile);
	pFile=NULL;
	
Exit_Pro:
	if(pFile!=NULL)
	{
		fclose(pFile);
	}
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("3005:Parsing finished AND out.\n");	

// 	if(g_nDebug)
// 	{
// 		err_log("verify_collect_conf: debug r=%d#",r);
// 	}
	return r;
}

/************************************************************************/
/* 获取采集配置 */
/************************************************************************/
parse_code_e get_collect_conf(int nConfLineNo,int nCurrentProcessNumber,collect_conf * pCollectConf)
{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("6001:get_collect_conf in\n");	

	parse_code_e  oR;
	FILE        * pFile=NULL;
	char          szTempFileName[256];
	char	      szBuff[2048];
	char          szContent[10][256];
	char        * szStr=NULL;
	int           nLen;
	int           i;
	
	oR = PARSE_UNMATCH;
	
	sprintf(szTempFileName,"%s/collect.conf",CONF);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("6002:try to open file: %s\n", szTempFileName);	
	pFile = fopen(szTempFileName,"r");
	if(pFile == NULL)
	{
		err_log("get_collect_conf: fopen %s fail\n",szTempFileName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("6101:Open file failed AND return PARSE_FAIL: %s\n", szTempFileName);	
		oR = PARSE_FAIL;
		goto Exit_Pro;
	}
	
	i = 0;
	while(1)
	{
		memset(szBuff,0,sizeof(szBuff));
		if(fgets(szBuff,sizeof(szBuff),pFile)==NULL)
		{
			if(i < nConfLineNo)
			{
				err_log("get_collect_conf: collect.conf line less %d\n",nConfLineNo);
				oR = PARSE_FAIL;
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if( g_nDebug )	printf("6102:i !< nConfLineNo AND return PARSE_FAIL: %s\n", szTempFileName);	
				goto Exit_Pro;
			}
			break;
		}
       	
		i++;
		
		if(i == nConfLineNo)
		{
			if(szBuff[0] == '#')
			{
				oR = PARSE_UNMATCH;
				break;
			}
			
			memset(szContent,0,sizeof(szContent));
			if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],\
				szContent[5],szContent[6],szContent[7],szContent[8],szContent[9]) != 10 )
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("6103:sscanf error AND return PARSE_FAIL: %s\n", szTempFileName);	
				oR = PARSE_FAIL;
				goto Exit_Pro;
			}
			
			pCollectConf->collect_point = i;
			strcpy(pCollectConf->ip,szContent[1]);
			pCollectConf->port = atoi(szContent[2]);
			strcpy(pCollectConf->usr,szContent[3]);
			strcpy(pCollectConf->password,szContent[4]);
			strcpy(pCollectConf->path_str,szContent[5]);
			strcpy(pCollectConf->file_str,szContent[6]);
			strcpy(pCollectConf->backup_path,szContent[7]);
			pCollectConf->current_process_number = nCurrentProcessNumber;

/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("6003:Start checking.\n");	

			/*path_up,path_last,path_pre,path_suf,is_multi_path*/
			nLen = strlen(szContent[5]);
			if(nLen < 1)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("6104:Line %d is incorrect: szContent[5] nLen < 1.\n", i);	
				oR = PARSE_FAIL;
				goto Exit_Pro;
			}
			szStr = strrchr(szContent[5],'/');
			if(szStr == NULL)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("6105:Line %d is incorrect: szContent[5] szStr == NULL.\n", i);	
				oR=PARSE_FAIL;
				goto Exit_Pro;
			}
			strncpy(pCollectConf->path_up,szContent[5],szStr-szContent[5]+1);
			if( &szContent[5][nLen-1]-szStr>0 )
			{
				strncpy(pCollectConf->path_last,szStr+1,&szContent[5][nLen-1]-szStr);
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
			nLen = strlen(szContent[6]);
			if(nLen < 1)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("6106:Line %d is incorrect: szContent[6] nLen < 1.\n", i);	
				oR=PARSE_FAIL;
				goto Exit_Pro;
			}
			szStr=strchr(szContent[6],'*');
			if(szStr == NULL)
			{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("6107:Line %d is incorrect: szContent[6] szStr == NULL.1\n", i);	
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
				oR=PARSE_FAIL;
				goto Exit_Pro;
			}
			if(szStr>szContent[6])
			{
				strncpy(pCollectConf->file_pre,szContent[6],szStr-szContent[6]);
			}
			szStr=strrchr(szContent[6],'*');
			if(szStr == NULL)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("6107:Line %d is incorrect: szContent[6] szStr == NULL.2\n", i);	
				oR=PARSE_FAIL;
				goto Exit_Pro;
			}
			if( &szContent[6][nLen-1]-szStr>0 )
			{
				strncpy(pCollectConf->file_suf,szStr+1,&szContent[6][nLen-1]-szStr);
			}
			
			/*is_backup,is_commit,is_interval_file*/
			if(strcasecmp(szContent[7],"not")==0)
			{
				pCollectConf->is_backup=0;
			}
			else
			{
				pCollectConf->is_backup=1;
			}
			
			if(strcasecmp(szContent[8],"yes")==0)
			{
				pCollectConf->is_commit=1;
			}
			else
			{
				pCollectConf->is_commit=0;
			}
			
			if(strcasecmp(szContent[9],"yes")==0)
			{
				pCollectConf->is_interval_file=1;
			}
			else
			{
				pCollectConf->is_interval_file=0;
			}
			
			oR = PARSE_MATCH;
/************************************************************************/
/* Alur changed */
/************************************************************************/	
			if (g_nDebug)	printf("6004:Parsing finished:\n-%s\n-%s\n-%s   %d\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n-%s\n",
				szContent[0],szContent[1],szContent[2],pCollectConf->port,szContent[3],szContent[4],
				szContent[5],szContent[6],szContent[7],szContent[8],szContent[9]);

			break;
			}
			
	}
Exit_Pro:	
	if(pFile!=NULL)
		fclose(pFile);

// 	if(g_nDebug)
// 	{
// 		err_log("get_collect_conf: debug oR=%d#\n",oR);
// 		err_log("get_collect_conf: debug p_collect_conf->collect_point         =%d#",p_collect_conf->collect_point);
// 		err_log("get_collect_conf: debug p_collect_conf->ip                    =%s#",p_collect_conf->ip);
// 		err_log("get_collect_conf: debug p_collect_conf->usr                   =%s#",p_collect_conf->usr);
// 		err_log("get_collect_conf: debug p_collect_conf->password              =%s#",p_collect_conf->password);
// 		err_log("get_collect_conf: debug p_collect_conf->path_str              =%s#",p_collect_conf->path_str);
// 		err_log("get_collect_conf: debug p_collect_conf->path_up               =%s#",p_collect_conf->path_up);
// 		err_log("get_collect_conf: debug p_collect_conf->path_last             =%s#",p_collect_conf->path_last);
// 		err_log("get_collect_conf: debug p_collect_conf->path_pre              =%s#",p_collect_conf->path_pre);
// 		err_log("get_collect_conf: debug p_collect_conf->path_suf              =%s#",p_collect_conf->path_suf);
// 		err_log("get_collect_conf: debug p_collect_conf->is_multi_path         =%d#",p_collect_conf->is_multi_path);
// 		err_log("get_collect_conf: debug p_collect_conf->file_str              =%s#",p_collect_conf->file_str);
// 		err_log("get_collect_conf: debug p_collect_conf->file_pre              =%s#",p_collect_conf->file_pre);
// 		err_log("get_collect_conf: debug p_collect_conf->file_suf              =%s#",p_collect_conf->file_suf);
// 		err_log("get_collect_conf: debug p_collect_conf->backup_path           =%s#",p_collect_conf->backup_path);
// 		err_log("get_collect_conf: debug p_collect_conf->is_backup             =%d#",p_collect_conf->is_backup);
// 		err_log("get_collect_conf: debug p_collect_conf->is_commit             =%d#",p_collect_conf->is_commit);
// 		err_log("get_collect_conf: debug p_collect_conf->is_interval_file      =%d#",p_collect_conf->is_interval_file);
// 		err_log("get_collect_conf: debug p_collect_conf->current_process_number=%d#",p_collect_conf->current_process_number);
// 	}

/************************************************************************/
/* Alur changed */
/************************************************************************/	
	if (g_nDebug)	printf("6006:get_collect_conf out.\n");

	return oR;
}


parse_code_e parse_get_file_name(collect_conf * p_collect_conf,char * curr_file,char * p_file_name)
{
	parse_code_e  r;
	
	int           ftp_ret;
	char          dir_out_file[256];
	
	FILE         *fp=NULL;
	char	      buff[2048];
	char          content[9][256];
	
	char         *pre=NULL;
	char         *suf=NULL;
	parse_code_e  r_pre_suf_check;
	
	r=PARSE_UNMATCH;
	
	sprintf(dir_out_file,"%s/%03d_dir_tmp",WORK,p_collect_conf->current_process_number);
	
	if(access(dir_out_file,F_OK)==0)
	{
		if(unlink(dir_out_file)!=0)
		{
			err_log("parse_get_file_name: unlink %s fail\n",dir_out_file);
			r=PARSE_FAIL;
			goto Exit_Pro;
		}
	}
	
	ftp_ret=Ftp_Dir(dir_out_file);
	if(ftp_ret!=0)
	{
		err_log("parse_get_file_name: collect_point=%d,Ftp_Dir fail",p_collect_conf->collect_point);
		r=PARSE_FAIL;
		goto Exit_Pro;
	}
	
	fp=fopen(dir_out_file,"r");
	if(fp==NULL)
	{
		err_log("parse_get_file_name: fopen %s fail\n",dir_out_file);
		r=PARSE_FAIL;
		goto Exit_Pro;
	}	
	
	if(p_collect_conf->file_pre[0]!='\0')
	{
		pre=p_collect_conf->file_pre;
	}
	else
	{
		pre=NULL;
	}		
	if(p_collect_conf->file_suf[0]!='\0')
	{
		suf=p_collect_conf->file_suf;
	}
	else
	{
		suf=NULL;
	}
	
	while(1)
	{
		memset(buff,0,sizeof(buff));
		memset(content,0,sizeof(content));
		if(fgets(buff,sizeof(buff),fp)==NULL)
			break;
		
		if( sscanf(buff,"%s%s%s%s%s%s%s%s%s",content[0],content[1],content[2],content[3],content[4],\
			content[5],content[6],content[7],content[8])!=9 )
			continue;
		
		if(content[0][0]!='-' )
			continue;
		if(strlen(content[0])!=10)
			continue;       		
		
		/*文件名前后缀匹配检查*/
		r_pre_suf_check=pre_suf_check(content[8],pre,suf);
		
		if(r_pre_suf_check==PARSE_FAIL || r_pre_suf_check==PARSE_UNMATCH)
			continue;
		
		/*和curr_file比较*/
		if(curr_file!=NULL)
		{
			if(strcmp(content[8],curr_file)>0)
			{
				strcpy(p_file_name,content[8]);
				r=PARSE_MATCH;
				break;
			}
		}
		else
		{
			strcpy(p_file_name,content[8]);
			r=PARSE_MATCH;
			break;
		}
	}
	fclose(fp);
	fp=NULL;
	
Exit_Pro:
	if(fp!=NULL)
		fclose(fp);
	if(g_nDebug)
	{
		err_log("parse_get_file_name: debug r=%d#",r);
		err_log("parse_get_file_name: debug p_file_name=%s#",p_file_name);
	}
	
	return r;
}

parse_code_e parse_get_dir_name(collect_conf * p_collect_conf,char * curr_dir,char * p_dir_name)
{
	parse_code_e  r;
	int           ftp_ret;
	char          dir_out_file[256];
	
	FILE         *fp=NULL;
	char	      buff[2048];
	char          content[9][256];
	
	char         *pre=NULL;
	char         *suf=NULL;
	parse_code_e  r_pre_suf_check;
	
	r=PARSE_UNMATCH;
	
	sprintf(dir_out_file,"%s/%03d_dir_tmp",WORK,p_collect_conf->current_process_number);
	
	if(access(dir_out_file,F_OK)==0)
	{
		if(unlink(dir_out_file)!=0)
		{
			err_log("parse_get_dir_name: unlink %s fail\n",dir_out_file);
			r=PARSE_FAIL;
			goto Exit_Pro;
		}
	}
	
	ftp_ret=Ftp_Dir(dir_out_file);
	if(ftp_ret!=0)
	{
		err_log("parse_get_dir_name: collect_point=%d,Ftp_Dir fail",p_collect_conf->collect_point);
		r=PARSE_FAIL;
		goto Exit_Pro;
	}
	
	fp=fopen(dir_out_file,"r");
	if(fp==NULL)
	{
		err_log("parse_get_dir_name: fopen %s fail\n",dir_out_file);
		r=PARSE_FAIL;
		goto Exit_Pro;
	}	
	
	
	if(p_collect_conf->path_pre[0]!='\0')
	{
		pre=p_collect_conf->path_pre;
	}
	else
	{
		pre=NULL;
	}		
	if(p_collect_conf->path_suf[0]!='\0')
	{
		suf=p_collect_conf->path_suf;
	}
	else
	{
		suf=NULL;
	}
	
	while(1)
	{
		memset(buff,0,sizeof(buff));
		memset(content,0,sizeof(content));
		if(fgets(buff,sizeof(buff),fp)==NULL)
			break;
		
		if( sscanf(buff,"%s%s%s%s%s%s%s%s%s",content[0],content[1],content[2],content[3],content[4],\
			content[5],content[6],content[7],content[8])!=9 )
			continue;
		
		if(content[0][0]!='d' )
			continue;
		if(strlen(content[0])!=10)
			continue;       		
		
		/*文件名前后缀匹配检查*/
		r_pre_suf_check=pre_suf_check(content[8],pre,suf);
		
		if(r_pre_suf_check==PARSE_FAIL || r_pre_suf_check==PARSE_UNMATCH)
			continue;
		
		/*和curr_dir比较*/
		if(curr_dir!=NULL)
		{
			if(strcmp(content[8],curr_dir)>0)
			{
				strcpy(p_dir_name,content[8]);
				r=PARSE_MATCH;
				break;
			}
		}
		else
		{
			strcpy(p_dir_name,content[8]);
			r=PARSE_MATCH;
			break;
		}
	}
	fclose(fp);
	fp=NULL;
	
Exit_Pro:
	if(fp!=NULL)
		fclose(fp);
	if(g_nDebug)
	{
		err_log("parse_get_dir_name: debug r=%d#",r);
		err_log("parse_get_dir_name: debug p_dir_name=%s#",p_dir_name);
	}
	return r;
}

parse_code_e parse_get_file_name_2(collect_conf * p_collect_conf,char * curr_dir,char * curr_file,char * p_dir_name,char * p_file_name)
{
	parse_code_e  r;
	parse_code_e  r_dir;
	parse_code_e  r_file;
	
	int           ftp_ret;
	
	char          dir_name[256];
	char          file_name[256];
	
	char          dir_name_2[256];
	char          file_name_2[256];
	
	r=PARSE_UNMATCH;
	
	/*=====================================================*/
	if(curr_dir==NULL || curr_dir[0]=='\0' )
	{
		memset(dir_name,0,sizeof(dir_name));
		while(1)
		{
			
			memset(dir_name_2,0,sizeof(dir_name_2));
			strcpy(dir_name_2,dir_name);
			memset(dir_name,0,sizeof(dir_name));
			
			r_dir=parse_get_dir_name(p_collect_conf,dir_name_2,dir_name);
			
			if(r_dir==PARSE_FAIL || r_dir==PARSE_UNMATCH)
			{
				r=r_dir;
				goto Exit_Pro;
			}
			
			if(r_dir==PARSE_MATCH)
			{
				ftp_ret=Ftp_Cd(dir_name);
				if(ftp_ret!=0)
				{
					err_log("parse_get_file_name_2: collect_point=%d,dir=%s,Ftp_Cd fail\n",p_collect_conf->collect_point,dir_name);
					r=PARSE_FAIL;
					goto Exit_Pro;
				}
				
				memset(file_name,0,sizeof(file_name));
				
				r_file=parse_get_file_name(p_collect_conf,NULL,file_name);
				
				if(r_file==PARSE_FAIL)
				{
					r=r_file;
					goto Exit_Pro;
				}
				
				ftp_ret=Ftp_Cdup();
				if(ftp_ret!=0)
				{
					err_log("parse_get_file_name_2: collect_point=%d Ftp_Cdup fail\n",p_collect_conf->collect_point);
					r=PARSE_FAIL;
					goto Exit_Pro;
				}
				
				if(r_file==PARSE_UNMATCH)
				{
					continue;
				}
				
				if(r_file==PARSE_MATCH)
				{
					strcpy(p_dir_name,dir_name);
					strcpy(p_file_name,file_name);
					
					r=r_file;
					goto Exit_Pro;				
				}
			}
		}
	}	
	/*=====================================================*/
	else
	{
		ftp_ret=Ftp_Cd(curr_dir);
		/********************************/
		if(ftp_ret==0)
		{
			memset(dir_name,0,sizeof(dir_name));
			strcpy(dir_name,curr_dir);
			
			memset(file_name_2,0,sizeof(file_name_2));
			if(curr_file!=NULL)
				strcpy(file_name_2,curr_file);
			
			memset(file_name,0,sizeof(file_name));
			
			r_file=parse_get_file_name(p_collect_conf,file_name_2,file_name);
			
			if(r_file==PARSE_FAIL)
			{
				r=r_file;
				goto Exit_Pro;
			}
			
			ftp_ret=Ftp_Cdup();
			if(ftp_ret!=0)
			{
				err_log("parse_get_file_name_2: collect_point=%d Ftp_Cdup fail\n",p_collect_conf->collect_point);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
			
			if(r_file==PARSE_UNMATCH)
			{
				/*----------------------------------*/
				while(1)
				{
					
					memset(dir_name_2,0,sizeof(dir_name_2));
					strcpy(dir_name_2,dir_name);
					memset(dir_name,0,sizeof(dir_name));
					
					r_dir=parse_get_dir_name(p_collect_conf,dir_name_2,dir_name);
					
					if(r_dir==PARSE_FAIL || r_dir==PARSE_UNMATCH)
					{
						r=r_dir;
						goto Exit_Pro;
					}
					
					if(r_dir==PARSE_MATCH)
					{
						ftp_ret=Ftp_Cd(dir_name);
						if(ftp_ret!=0)
						{
							err_log("parse_get_file_name_2: collect_point=%d,dir=%s,Ftp_Cd fail\n",p_collect_conf->collect_point,dir_name);
							r=PARSE_FAIL;
							goto Exit_Pro;
						}
						
						memset(file_name,0,sizeof(file_name));
						
						r_file=parse_get_file_name(p_collect_conf,NULL,file_name);
						
						if(r_file==PARSE_FAIL)
						{
							r=r_file;
							goto Exit_Pro;
						}
						
						ftp_ret=Ftp_Cdup();
						if(ftp_ret!=0)
						{
							err_log("parse_get_file_name_2: collect_point=%d Ftp_Cdup fail\n",p_collect_conf->collect_point);
							r=PARSE_FAIL;
							goto Exit_Pro;
						}
						
						if(r_file==PARSE_UNMATCH)
						{
							continue;
						}
						
						if(r_file==PARSE_MATCH)
						{
							strcpy(p_dir_name,dir_name);
							strcpy(p_file_name,file_name);
							
							r=r_file;
							goto Exit_Pro;				
						}
					}
				}
				/*----------------------------------*/
			}
			
			if(r_file==PARSE_MATCH)
			{
				strcpy(p_dir_name,dir_name);
				strcpy(p_file_name,file_name);
				
				r=r_file;
				goto Exit_Pro;				
			}
		}
		/********************************/
		else
		{
			memset(dir_name,0,sizeof(dir_name));
			strcpy(dir_name,curr_dir);
			while(1)
			{
				
				memset(dir_name_2,0,sizeof(dir_name_2));
				strcpy(dir_name_2,dir_name);
				memset(dir_name,0,sizeof(dir_name));
				
				r_dir=parse_get_dir_name(p_collect_conf,dir_name_2,dir_name);
				
				if(r_dir==PARSE_FAIL || r_dir==PARSE_UNMATCH)
				{
					r=r_dir;
					goto Exit_Pro;
				}
				
				if(r_dir==PARSE_MATCH)
				{
					ftp_ret=Ftp_Cd(dir_name);
					if(ftp_ret!=0)
					{
						err_log("parse_get_file_name_2: collect_point=%d,dir=%s,Ftp_Cd fail\n",p_collect_conf->collect_point,dir_name);
						r=PARSE_FAIL;
						goto Exit_Pro;
					}
					
					memset(file_name,0,sizeof(file_name));
					
					r_file=parse_get_file_name(p_collect_conf,NULL,file_name);
					
					if(r_file==PARSE_FAIL)
					{
						r=r_file;
						goto Exit_Pro;
					}
					
					ftp_ret=Ftp_Cdup();
					if(ftp_ret!=0)
					{
						err_log("parse_get_file_name_2: collect_point=%d Ftp_Cdup fail\n",p_collect_conf->collect_point);
						r=PARSE_FAIL;
						goto Exit_Pro;
					}
					
					if(r_file==PARSE_UNMATCH)
					{
						continue;
					}
					
					if(r_file==PARSE_MATCH)
					{
						strcpy(p_dir_name,dir_name);
						strcpy(p_file_name,file_name);
						
						r=r_file;
						goto Exit_Pro;				
					}
				}
			}
		}
		/********************************/
	}
	/*=====================================================*/
	
Exit_Pro:
	if(g_nDebug)
	{
		err_log("parse_get_file_name_2: debug r=%d#",r);
		err_log("parse_get_file_name_2: debug p_dir_name =%s#",p_dir_name);
		err_log("parse_get_file_name_2: debug p_file_name=%s#",p_file_name);
	}
	return r;
}

/************************************************************************/
/* 解析采集配置文件并尝试连接FTP */
/************************************************************************/
/*
*parse_collect_conf()
*                      if return PARSE_MATCH,szRemotePath and szRemoteFileName are given
*
*/
parse_code_e parse_collect_conf(collect_conf * pCollectConf,char * szRemotePath,char * szRemoteFileName)
{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("5001:parse_collect_conf in.\n");	
	
	parse_code_e  oR;
	parse_code_e  oRFile;
	parse_code_e  oRFile2;
	
	int           nFtpRet;
	char          szDirName[256];
	char          szFileName[256];
	char          szDirFromConf[256];
	char          szFileFromConf[256];
	
	FILE          * pFile=NULL;
	char          szTempFileName[256];
	int           nTempCollectPoint;
	
	oR = PARSE_UNMATCH;
	
	nTempCollectPoint=0;
	memset(szDirFromConf,0,sizeof(szDirFromConf));
	memset(szFileFromConf,0,sizeof(szFileFromConf));
	
	sprintf(szTempFileName,"%s/%06d_run.conf",CONF,pCollectConf->collect_point);

	if(access(szTempFileName,F_OK) == 0)
	{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("5002:Try to open file: %s.\n",szTempFileName);	

		pFile = fopen(szTempFileName,"r");
		if(pFile == NULL)
		{
			err_log("parse_collect_conf: fopen %s fail\n",szTempFileName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("5101:Open file failed AND return PARSE_FAIL: %s.\n",szTempFileName);	
			oR = PARSE_FAIL;
			goto Exit_Pro;
		}
		fscanf(pFile,"<%d>%s%s",&nTempCollectPoint,szDirFromConf,szFileFromConf);
		if(nTempCollectPoint != pCollectConf->collect_point)
		{
			err_log("parse_collect_conf: %s szContent fail\n",szTempFileName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("5102:nTempCollectPoint != pCollectConf->collect_point AND return PARSE_FAIL: %s.\n",szTempFileName);	
			oR = PARSE_FAIL;
			goto Exit_Pro;
		}
		fclose(pFile);
		pFile=NULL;
	}
	
/************************************************************************/
/* Alur changed:增加端口 */
/************************************************************************/
	if(g_nDebug)	printf("5003:Try to Ftp_Init.\n");	
	/* Ftp_Init() */
	nFtpRet = Ftp_Init(pCollectConf->usr,pCollectConf->password,pCollectConf->ip,pCollectConf->port,FTP_TIME_OUT,1,1,g_nDebug);
// 	nFtpRet=Ftp_Init(pCollectConf->usr,pCollectConf->password,pCollectConf->ip,0,FTP_TIME_OUT,1,1,debug);
	if(nFtpRet!=0)
	{
		err_log("parse_collect_conf: Ftp_Init fail,collect_point=%d\n",pCollectConf->collect_point);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("5103:Ftp_Init failed.\n");	
		oR=PARSE_FAIL;
		goto Exit_Pro;
	}

/************************************************************************/
/* Alur changed:增加端口 */
/************************************************************************/
	if(g_nDebug)	printf("5004:Try to Ftp_Cd: %s\n",pCollectConf->path_up);	
	
	/* Ftp_Cd() */
	nFtpRet=Ftp_Cd(pCollectConf->path_up);
	if(nFtpRet!=0)
	{
		err_log("parse_collect_conf: Ftp_Cd fail,collect_point=%d,dir=%s\n",pCollectConf->collect_point,pCollectConf->path_up);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("5104:Ftp_Cd failed: %s.\n",pCollectConf->path_up);	
		oR = PARSE_FAIL;
		goto Exit_Pro;
	}
	

	/* Multi PATH */
	if(pCollectConf->is_multi_path)
	{

		/* Multi PATH, Interval */
		if(pCollectConf->is_interval_file)
		{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("5005:Multi PATH Interval start.\n");	

			memset(szDirName,0,sizeof(szDirName));
			memset(szFileName,0,sizeof(szFileName));
			
			oRFile2 = parse_get_file_name_2(pCollectConf,szDirFromConf,szFileFromConf,szDirName,szFileName);
			
			if(oRFile2 == PARSE_FAIL || oRFile2 == PARSE_UNMATCH)
			{
				oR=oRFile2;
				goto Exit_Pro;
			}
			
			if(oRFile2 == PARSE_MATCH)
			{
				strcpy(szRemotePath,szDirName);
				strcpy(szRemoteFileName,szFileName);
				
				memset(szDirName,0,sizeof(szDirName));
				memset(szFileName,0,sizeof(szFileName));
				
				oRFile2 = parse_get_file_name_2(pCollectConf,szRemotePath,szRemoteFileName,szDirName,szFileName);
				
				oR = oRFile2;
				goto Exit_Pro;
			}
		}
		/* Multi PATH, NOT Interval */
		else
		{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("5006:Multi PATH NOT Interval start.\n");	

			memset(szDirName,0,sizeof(szDirName));
			memset(szFileName,0,sizeof(szFileName));
			
			oRFile2=parse_get_file_name_2(pCollectConf,szDirFromConf,szFileFromConf,szDirName,szFileName);
			
			if(oRFile2==PARSE_MATCH)
			{
				strcpy(szRemotePath,szDirName);
				strcpy(szRemoteFileName,szFileName);
			}
			
			oR=oRFile2;
			goto Exit_Pro;
		}
	}
	/* NOT Multi PATH */
	else
	{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if( g_nDebug )	printf("5007:NOT Multi PATH start AND try to Ftp_Cd:%s.\n",szRemotePath);	

		strcpy( szRemotePath, pCollectConf->path_last );
		if(*szRemotePath != '\0')
		{  
			nFtpRet = Ftp_Cd(szRemotePath);
			if(nFtpRet != 0)
			{
				err_log("parse_collect_conf: Ftp_Cd 2 fail,collect_point=%d,dir=%s\n",pCollectConf->collect_point,szRemotePath);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("5105: Ftp_Cd failed AND return PARSE_FAIL.\n-%s\n-%s\n",szRemotePath,szTempFileName);	
				oR = PARSE_FAIL;
				goto Exit_Pro;
			}
		}  	
		/* NOT Multi PATH, Interval */
		if(pCollectConf->is_interval_file)
		{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if( g_nDebug )	printf("5008:NOT Multi PATH, Interval start.\n");	

			memset(szFileName,0,sizeof(szFileName));
			
			oRFile=parse_get_file_name(pCollectConf,szFileFromConf,szFileName);
			
			if(oRFile==PARSE_FAIL || oRFile==PARSE_UNMATCH)
			{
				oR=oRFile;
				goto Exit_Pro;
			}
			
			if(oRFile==PARSE_MATCH)
			{
				strcpy(szRemoteFileName,szFileName);
				
				memset(szFileName,0,sizeof(szFileName));
				
				oRFile=parse_get_file_name(pCollectConf,szRemoteFileName,szFileName);
				
				oR=oRFile;
				goto Exit_Pro;
			}
			
			
		}
		/* NOT Multi PATH, NOT Interval */
		else
		{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if( g_nDebug )	printf("5009:NOT Multi PATH, NOT Interval start.\n");	

			memset(szFileName,0,sizeof(szFileName));
			oRFile=parse_get_file_name(pCollectConf,szFileFromConf,szFileName);
			
			if(oRFile == PARSE_MATCH)
				strcpy(szRemoteFileName,szFileName);
			
			oR=oRFile;
			goto Exit_Pro;
		}
	}
	
Exit_Pro:
	if(pFile!=NULL)
		fclose(pFile);
	/* Ftp_Close() */
	Ftp_Close();
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("5010:parse_collect_conf out.\n");	
	
	return oR;
}

/*
*get_remote_file()
*                  return 0 success, 1 fail
*
*/
int get_remote_file(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name,long * file_size)
{
	int  r;
	int  ftp_ret;
	long succ_bytes;
	char tmp_file_name[512];
	
	r=0;

/************************************************************************/
/* alur changed:增加端口*/
/************************************************************************/
	/* Ftp_Init() */
	ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,p_collect_conf->port,FTP_TIME_OUT,1,1,g_nDebug);
//		ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,0,FTP_TIME_OUT,1,1,debug);
	if(ftp_ret!=0)
	{
		err_log("get_remote_file: Ftp_Init fail,collect_point=%d\n",p_collect_conf->collect_point);
		r=1;
		goto Exit_Pro;
	}
	
	/* Ftp_Cd() 1 */
	ftp_ret=Ftp_Cd(p_collect_conf->path_up);
	if(ftp_ret!=0)
	{
		err_log("get_remote_file: Ftp_Cd fail,collect_point=%d,dir=%s\n",p_collect_conf->collect_point,p_collect_conf->path_up);
		r=1;
		goto Exit_Pro;
	}
	
	/* Ftp_Cd() 2 */
	if(*remote_path!='\0')
	{  
		ftp_ret=Ftp_Cd(remote_path);
		if(ftp_ret!=0)
		{
			err_log("get_remote_file: Ftp_Cd 2 fail,collect_point=%d,dir=%s\n",p_collect_conf->collect_point,remote_path);
			r=1;
			goto Exit_Pro;
		}
	}  
	/* Ftp_Receive() */
	succ_bytes=0;
	sprintf(tmp_file_name,"%s/%03d_%s",WORK,p_collect_conf->current_process_number,remote_file_name);
	ftp_ret=Ftp_Receive(remote_file_name,tmp_file_name,0,&succ_bytes);
	*file_size=succ_bytes;
	
	if(ftp_ret!=0)
	{
		err_log("get_remote_file: Ftp_Receive fail,collect_point=%d,file=%s\n",p_collect_conf->collect_point,remote_file_name);
		r=1;
		goto Exit_Pro;
	}
	
Exit_Pro:
	/* Ftp_Close() */
	Ftp_Close();
	
	return r;
}

/************************************************************************/
/* 获取备份文件名 */
/************************************************************************/
/*
*get_backup_name()
*             return 0 success, 1 fail
*
*/
int get_backup_name(collect_conf * pCollectConf,char * szRemotePath,char * szRemoteFileName,char * szBackupName)
{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("8001:get_backup_name in\n");	
	
	int    nRe = 0;
	
	char   szTemp[512];
	char   szTempPath[256];
	char  *szString = NULL;
	
	int    nLen=0;
	
	if(pCollectConf->is_multi_path)
	{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("8002:is_multi_path.\n");	

		sprintf(szBackupName,"%s/%s/%s",pCollectConf->backup_path,szRemotePath,szRemoteFileName);
		sprintf(szTemp,"%s/%s",pCollectConf->backup_path,szRemotePath);
		szTemp[sizeof(szTemp)-1]='\0';
		if(access(szTemp,F_OK) == -1)
		{
			if(mkdir(szTemp,0755)!=0)
			{
				err_log("get_backup_name: mkdir %s fail\n",szTemp);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("8101:mkdir fail:%s\n",szTemp);	

				nRe=1;
				goto Exit_Pro;
			}
		}
	}
	else
	{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("8003:NOT is_multi_path.\n");	

		memset(szTempPath,0,sizeof(szTempPath));
		szString=strchr(szRemoteFileName,'.');
		if(szString == NULL)
		{
			err_log("get_backup_name: get szTempPath from %s fail\n",szRemoteFileName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("8102:szString == NULL\n");	
			nRe=1;
			goto Exit_Pro;
		}
		
		nLen=strlen(szRemoteFileName);
		if(&szRemoteFileName[nLen-1]-szString-1>=0)
		{
			if(&szRemoteFileName[nLen-1]-szString>10)
				strncpy(szTempPath,szString+1,10);
			else
				strncpy(szTempPath,szString+1,&szRemoteFileName[nLen-1]-szString);
		}
		else
		{
			err_log("get_backup_name: get szTempPath from %s fail\n",szRemoteFileName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("8103: ! &szRemoteFileName[nLen-1]-szString-1>=0\n");	
			nRe=1;
			goto Exit_Pro;
		}
			
		
		//??
		sprintf(szBackupName,"%s/%s/%s",pCollectConf->backup_path,szTempPath,szRemoteFileName);
		sprintf(szTemp,"%s/%s",pCollectConf->backup_path,szTempPath);
		szTemp[sizeof(szTemp)-1]='\0';
		if(access(szTemp,F_OK)==-1)
		{
			if(mkdir(szTemp,0755)!=0)
			{
				err_log("get_backup_name: mkdir %s fail\n",szTemp);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("8104:mkdir fail:%s\n",szTemp);	
				nRe=1;
				goto Exit_Pro;
			}
		}
	}
	
Exit_Pro:
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("8004:get_backup_name out\n");	

	return nRe;
}

/************************************************************************/
/* 备份文件 */
/************************************************************************/
/*
*backup_file()
*              return 0 success, 1 fail
*
*/
int backup_file(collect_conf * pCollectConf,char * szRemotePath,char * szRemoteFileName)
{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("7001:backup_file in\n");	

	int  nRe;
	
	FILE *pFileIn=NULL;
	FILE *pFileOut=NULL;
	
	char szBackupName[512];
	
	char szTempFileName[512];
	
	int  nRLen;
	int  nWLen;
	int  nBuffLen;
	char szBuff[8192];
	
	nRe = 0;
	
	/*判断是否备份*/ 
	if(pCollectConf->is_backup)
	{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("7002:Need Backup.\n");	
 
		sprintf(szTempFileName,"%s/%03d_%s",WORK,pCollectConf->current_process_number,szRemoteFileName);
		
		memset(szBackupName,0,sizeof(szBackupName));
		if(get_backup_name(pCollectConf,szRemotePath,szRemoteFileName,szBackupName)!=0)
		{
			err_log("backup_file: get_backup_name fail\n");
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("7101:get_backup_name fail.\n");	
			nRe=1;
			goto Exit_Pro;
		}
		szBackupName[sizeof(szBackupName)-1]='\0';
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("7003:Open Temp File AND Backup File.\n-TempFile:%s\n-BackupFile:%s\n",
			szTempFileName,szBackupName);	
		
		pFileIn = fopen(szTempFileName,"r");
		if(pFileIn==NULL)
		{
			err_log("backup_file: fopen %s fail\n",szTempFileName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("7102:Open Temp file fail : %s\n",szTempFileName);	
			nRe=1;
			goto Exit_Pro;
		}
		
		pFileOut=fopen(szBackupName,"w+");
		if(pFileOut==NULL)
		{
			err_log("backup_file: fopen %s fail\n",szBackupName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("7103:Open Backup file fail : %s\n",szBackupName);	
			nRe=1;
			goto Exit_Pro;
		}
/************************************************************************/
/*  Alur changed */
/************************************************************************/
		if(g_nDebug)	printf("7004:Start backup.\n");	
		
		nBuffLen=sizeof(szBuff);
		while(1)
		{
			nRLen=fread(szBuff,1,nBuffLen,pFileIn);
			if(nRLen>0)
			{
				nWLen=fwrite(szBuff,1,nRLen,pFileOut);
				if(nWLen<nRLen)
				{
					err_log("backup_file: fwrite %s fail\n",szBackupName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
					if(g_nDebug)	printf("7104:fwrinte fail.\n");	

					nRe=1;
					goto Exit_Pro;
				}
			}
			if(nRLen<nBuffLen)
			{
				if( ferror(pFileIn) )
				{
					err_log("backup_file: fread %s fail\n",szTempFileName);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
					if(g_nDebug)	printf("7105:fread fail.\n");	
					nRe=1;
					goto Exit_Pro;
				}
				break;
			}
		}
	} 	
Exit_Pro:
	if(pFileIn!=NULL) fclose(pFileIn);
	if(pFileOut!=NULL) fclose(pFileOut);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("7001:backup_file out\n");	
	
	return nRe;
}

/*
*get_commit_name()
*             return 0 success, 1 fail
*
*/
int get_commit_name(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name,char * commit_name)
{
	int    r;
	
	r=0;
	
	if(p_collect_conf->is_multi_path)
	{
		sprintf(commit_name,"%06d_%s_%s",p_collect_conf->collect_point,remote_path,remote_file_name);
	}
	else
	{
		sprintf(commit_name,"%06d_%s",p_collect_conf->collect_point,remote_file_name);
	}
	
	
	return r;
}

/*
*commit_file()
*              return 0 success, 1 fail
*
*/
int commit_file(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name,char * begin,char * end,long file_size)
{
	int   ret;
	
	FILE  * fp=NULL;
	
	char  run_log_time[15];
	
	char  commit_name[256];
	
	char  file_name[256];
	
	char  tmp_file_name[512];
	char  tmp_file_name_2[512];
	
	char  pre[9];
	
	ret=0;
	
	
    //判断是否提交
    if(p_collect_conf->is_commit) 
    {
        //link工作目录文件至提交目录
        memset(commit_name,0,sizeof(commit_name));
		if(get_commit_name(p_collect_conf,remote_path,remote_file_name,commit_name)!=0)
		{
			err_log("commit_file: get_commit_name fail\n");
			ret=1;
			goto Exit_Pro;
		}
		commit_name[sizeof(commit_name)-1]='\0';
		
		sprintf(tmp_file_name,"%s/%03d_%s",WORK,p_collect_conf->current_process_number,remote_file_name);
		sprintf(tmp_file_name_2,"%s/%s",g_szFileCommitDir,commit_name);
		
		if(access(tmp_file_name_2,F_OK)==-1)
		{
			if(link(tmp_file_name,tmp_file_name_2)!=0)
			{
				err_log("commit_file: link %s to %s fail\n",tmp_file_name,tmp_file_name_2);
				ret=1;
				goto Exit_Pro;
			}
		}
		else
		{
			err_log("commit_file: target file %s exist\n",tmp_file_name_2);
		}
    }
	
	
	//记录collect_run.xxxxxx.YYYYMMDD文件
	get_time(run_log_time);
	run_log_time[8]='\0';
	
	sprintf(tmp_file_name,"%s.%06d.%s",PREFIX_RUN_LOG_FILE,p_collect_conf->collect_point,run_log_time);
	
	fp=fopen(tmp_file_name,"a+");
	if(fp==NULL)
	{
		err_log("commit_file: fopen %s fail\n",tmp_file_name);
		ret=1;
		goto Exit_Pro;
	}
	if(*remote_path!='\0')
	{
		fprintf(fp,"<%06d> %s %s %s %s %ld %d %d\n",p_collect_conf->collect_point,remote_path, \
			remote_file_name,begin,end,file_size,p_collect_conf->is_backup,p_collect_conf->is_commit);
	}
	else
	{
		fprintf(fp,"<%06d> . %s %s %s %ld %d %d\n",p_collect_conf->collect_point, \
			remote_file_name,begin,end,file_size,p_collect_conf->is_backup,p_collect_conf->is_commit);
	}
	fclose(fp);
	fp=NULL;	
	
	
	//清理work目录下,xxx_ 开头文件,xxx为子进程编号
	memset(file_name,0,sizeof(file_name));
	sprintf(pre,"%03d",p_collect_conf->current_process_number);
	while(get_orig_file_name(WORK,pre,NULL,file_name)==PARSE_MATCH)
	{
		sprintf(tmp_file_name_2,"%s/%s",WORK,file_name);
		
		if ( unlink(tmp_file_name_2)!=0 )
		{
			err_log("commit_file: unlink file %s fail\n",tmp_file_name_2);
			ret=1;
			goto Exit_Pro;
		}
		memset(file_name,0,sizeof(file_name));
	}
	
	//修改xxxxxx_run.conf文件
	sprintf(tmp_file_name,"%s/%06d_run.conf",CONF,p_collect_conf->collect_point);
	fp=fopen(tmp_file_name,"w+");
	if(fp==NULL)
	{
		err_log("commit_file: fopen %s fail\n",tmp_file_name);
		ret=1;
		goto Exit_Pro;
	}
	if(*remote_path!='\0')
	{
		fprintf(fp,"<%06d> %s %s",p_collect_conf->collect_point,remote_path,remote_file_name);
	}
	else
	{
		fprintf(fp,"<%06d> .  %s",p_collect_conf->collect_point,remote_file_name);
	}
	
	fclose(fp);
	fp=NULL;
	
	
Exit_Pro:
	if(fp!=NULL) fclose(fp);
	
	return ret;
}


/************************************************************************/
/* 采集函数 */
/************************************************************************/
/*
*point_collect()
*                return 0 success,1 fail
*
*/
int point_collect(int nConfLineNo,int nCurrentProcessNo)
{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
	if(g_nDebug)	printf("4001:point_collect in.\n");	

	int  nGetConf;
	int  nParseConf;
	char szRemotePath[256];
	char szRemoteFileName[256];
	
	char   cBegin[15];
	char   cEnd[15];
	long   lFileSize;
	
	collect_conf oMyCollectConf;
	
	while(1)
	{
		memset(&oMyCollectConf,0,sizeof(oMyCollectConf));
		nGetConf = get_collect_conf(nConfLineNo,nCurrentProcessNo,&oMyCollectConf);
		if(nGetConf == PARSE_FAIL)
		{
			err_log("point_collect: nConfLineNo=%d,get_collect_conf fail\n",nConfLineNo);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("4101:Line: %d get failed.\n",nConfLineNo);	
			return 1;
		}
		
		if(nGetConf == PARSE_UNMATCH) /*collect.conf配置文件中,该行以#开头*/
		{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("4102:Line: %d start with '#'.\n",nConfLineNo);	
			return 0;
		}
		
		memset(szRemotePath,0,sizeof(szRemotePath));
		memset(szRemoteFileName,0,sizeof(szRemoteFileName));

		nParseConf = parse_collect_conf(&oMyCollectConf,szRemotePath,szRemoteFileName);
		
		if(nParseConf==PARSE_FAIL)
		{
			err_log("point_collect: nConfLineNo=%d,parse_collect_conf return PARSE_FAIL\n",nConfLineNo);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("4103:Line: %d parse failed.\n",nConfLineNo);	
			return 1;
		}
		
		if(nParseConf==PARSE_UNMATCH)
		{
/************************************************************************/
/*  Alur changed */
/************************************************************************/
			if(g_nDebug)	printf("4104:Line: %d unmatch.\n",nConfLineNo);	

			return 0;
		}
		
		
		if(nParseConf == PARSE_MATCH)
		{
			get_time(cBegin);
			
/************************************************************************/
/* Alur changed*/
/************************************************************************/			
			if(g_nDebug)	printf("4002:Try to Execute get_remote_file.\n");
			//ftp get文件
			if(get_remote_file(&oMyCollectConf,szRemotePath,szRemoteFileName,&lFileSize)!=0)
			{
				err_log("point_collect: nConfLineNo=%d,szRemoteFileName=%s,get_remote_file FAIL\n",nConfLineNo,szRemoteFileName);
/************************************************************************/
/* Alur changed*/
/************************************************************************/			
				if(g_nDebug)	printf("4105:get_remote_file failed.\n");	
				return 1;
			}
			//备份文件
			if(backup_file(&oMyCollectConf,szRemotePath,szRemoteFileName)!=0)
			{
				err_log("point_collect: nConfLineNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nConfLineNo,szRemoteFileName);
/************************************************************************/
/* Alur changed*/
/************************************************************************/			
				if(g_nDebug)	printf("4106:backup_file failed.-%s\n-%s\n",szRemotePath,szRemoteFileName);	
				return 1;
			}
			
			get_time(cEnd);
/************************************************************************/
/* Alur changed*/
/************************************************************************/			
			if(g_nDebug)	printf("4003:Try to Execute commit_file.\n");
			
			//提交文件
			if(commit_file(&oMyCollectConf,szRemotePath,szRemoteFileName,cBegin,cEnd,lFileSize)!=0)
			{
				err_log("point_collect: nConfLineNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nConfLineNo,szRemoteFileName);
/************************************************************************/
/* Alur changed*/
/************************************************************************/			
				if(g_nDebug)	printf("4107:commit_file failed.-%s\n-%s\n-%s\n-%s\n-%ld\n",
					szRemotePath,szRemoteFileName,cBegin,cEnd,lFileSize);	
				return 1;
			}
			
			continue;
		}
		
		err_log("point_collect: nConfLineNo=%d,parse_collect_conf return UNKNOW\n",nConfLineNo);
/************************************************************************/
/* Alur changed*/
/************************************************************************/			
		if(g_nDebug)	printf("4108:parse_collect_conf return UNKNOW.\n");	

		return 1;
	}
	
/************************************************************************/
/* Alur changed*/
/************************************************************************/			
	if(g_nDebug)	printf("4004:point_collect out.\n");
	
	return 0;
}

/*
*process_collect()
*
*
*/
void process_collect(int current_number,int parallel_number)
{
	int     lineno_conf;
	int     i,j;
	
	/*设置全局子进程编号*/
	g_nCurrentProcessNumber=current_number;
	
	/*Ftp jmp*/
	if(sigsetjmp(g_jmp_buf_2002,1))
	{
	       err_log("point_collect: sigsetjmp  return 1\n");
		   exit(1);	
	}
	
	/*verify_collect_conf*/
	lineno_conf=verify_collect_conf();
	if(lineno_conf<=0)
	{
		err_log("main: verify_collect_conf=%d,fail\n",lineno_conf);
		return;
	}
	
	i=0;
	while(1)
	{
		j=i*parallel_number+current_number;
		if(j>lineno_conf)  break;
		point_collect(j,current_number);
		i++;
	}
	
	return;
}
