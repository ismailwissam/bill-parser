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
typedef enum parse_code {
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
	if(g_nDebug)	printf("1003:Check Finished.\nFileCommitDir:%s\nRunDir:%s\nParallelChildProcess: %d\nClean WORK dir.\n",g_szFileCommitDir,g_szRunDir,g_nParallelChildProcess);	

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
		if(g_nDebug)	printf("Deamon|BEEP!!!\n");	

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
	char          szContent[9][256];
	int           len;
	
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
			if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],\
				szContent[4],szContent[5],szContent[6],szContent[7],szContent[8])!=9 )
			{
				err_log("verify_collect_conf: collect.conf line %d incorrect\n",r);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("3102:Conf file line %d incorrect.1\n",r);	

				r=-1;
				goto Exit_Pro;
			}
			
			len=strlen(szContent[0]);
			if(len!=8 || szContent[0][0]!='<' || szContent[0][len-1]!='>')
			{
				err_log("verify_collect_conf: collect.conf line %d incorrect\n",r);
/************************************************************************/
/*  Alur changed */
/************************************************************************/
				if(g_nDebug)	printf("3103:Conf file line %d incorrect.2\n",r);	

				r=-1;
				goto Exit_Pro;
			}
			
			szContent[0][len-1]='\0';
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
		if(g_nDebug)	printf("3004:Parsing Result:\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
								szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
								szContent[5],szContent[6],szContent[7],szContent[8]);	
		
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

parse_code_e get_collect_conf(int lineno_conf,int current_process_number,collect_conf * p_collect_conf)
{
	parse_code_e  r;
	
	FILE        * fp=NULL;
	
	char          tmp_file_name[256];
	char	      buff[2048];
	char          content[9][256];
	char        * p_str=NULL;
	int           len;
	int           i;
	
	r=PARSE_UNMATCH;
	
	sprintf(tmp_file_name,"%s/collect.conf",CONF);
	fp=fopen(tmp_file_name,"r");
	if(fp==NULL)
	{
		err_log("get_collect_conf: fopen %s fail\n",tmp_file_name);
		r=PARSE_FAIL;
		goto Exit_Pro;
	}
	
	i=0;
	while(1)
	{
		memset(buff,0,sizeof(buff));
		if(fgets(buff,sizeof(buff),fp)==NULL)
		{
			if(i<lineno_conf)
			{
				err_log("get_collect_conf: collect.conf line less %d\n",lineno_conf);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
			break;
		}
       	
		i++;
		
		if(i==lineno_conf)
		{
			if(buff[0]=='#')
			{
				r=PARSE_UNMATCH;
				break;
			}
			
			memset(content,0,sizeof(content));
			if( sscanf(buff,"%s%s%s%s%s%s%s%s%s",content[0],content[1],content[2],content[3],content[4],\
				content[5],content[6],content[7],content[8])!=9 )
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
			
			p_collect_conf->collect_point=i;
			strcpy(p_collect_conf->ip,content[1]);
			strcpy(p_collect_conf->usr,content[2]);
			strcpy(p_collect_conf->password,content[3]);
			strcpy(p_collect_conf->path_str,content[4]);
			strcpy(p_collect_conf->file_str,content[5]);
			strcpy(p_collect_conf->backup_path,content[6]);
			p_collect_conf->current_process_number=current_process_number;
/************************************************************************/
/* Alur changed */
/************************************************************************/	
			if (g_nDebug)	printf("%s\n%s\n%s\n%s\n%s\n%s\n",content[1],content[2],content[3],content[4],content[5],content[6]);
			/*path_up,path_last,path_pre,path_suf,is_multi_path*/
			len=strlen(content[4]);
			if(len<1)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
			p_str=strrchr(content[4],'/');
			if(p_str==NULL)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
			strncpy(p_collect_conf->path_up,content[4],p_str-content[4]+1);
			if( &content[4][len-1]-p_str>0 )
			{
				strncpy(p_collect_conf->path_last,p_str+1,&content[4][len-1]-p_str);
			}
			
			if( (len=strlen(p_collect_conf->path_last))>0 )
			{
				p_str=strchr(p_collect_conf->path_last,'*');
				
				if(p_str==NULL)
				{
					p_collect_conf->is_multi_path=0;
				}
				else
				{
					p_collect_conf->is_multi_path=1;
					if(p_str>p_collect_conf->path_last)
					{
						strncpy(p_collect_conf->path_pre,p_collect_conf->path_last,p_str-p_collect_conf->path_last);
					}
				}
				
				p_str=strrchr(p_collect_conf->path_last,'*');
				if(p_str!=NULL)
				{
					if( &(p_collect_conf->path_last[len-1])-p_str>0 )
					{
						strncpy(p_collect_conf->path_suf,p_str+1,&(p_collect_conf->path_last[len-1])-p_str);
					}
				}
			}
			else
			{
				p_collect_conf->is_multi_path=0;
			}	
			
			/*file_pre,file_suf*/
			len=strlen(content[5]);
			if(len<1)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
			p_str=strchr(content[5],'*');
			if(p_str==NULL)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
			if(p_str>content[5])
			{
				strncpy(p_collect_conf->file_pre,content[5],p_str-content[5]);
			}
			p_str=strrchr(content[5],'*');
			if(p_str==NULL)
			{
				err_log("get_collect_conf: collect.conf line %d incorrect\n",i);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
			if( &content[5][len-1]-p_str>0 )
			{
				strncpy(p_collect_conf->file_suf,p_str+1,&content[5][len-1]-p_str);
			}
			
			/*is_backup,is_commit,is_interval_file*/
			if(strcasecmp(content[6],"not")==0)
			{
				p_collect_conf->is_backup=0;
			}
			else
			{
				p_collect_conf->is_backup=1;
			}
			
			if(strcasecmp(content[7],"yes")==0)
			{
				p_collect_conf->is_commit=1;
			}
			else
			{
				p_collect_conf->is_commit=0;
			}
			
			if(strcasecmp(content[8],"yes")==0)
			{
				p_collect_conf->is_interval_file=1;
			}
			else
			{
				p_collect_conf->is_interval_file=0;
			}
			
			r=PARSE_MATCH;
			break;
			}
			
	}
Exit_Pro:	
	if(fp!=NULL)
		fclose(fp);
	if(g_nDebug)
	{
		err_log("get_collect_conf: debug r=%d#\n",r);
		err_log("get_collect_conf: debug p_collect_conf->collect_point         =%d#",p_collect_conf->collect_point);
		err_log("get_collect_conf: debug p_collect_conf->ip                    =%s#",p_collect_conf->ip);
		err_log("get_collect_conf: debug p_collect_conf->usr                   =%s#",p_collect_conf->usr);
		err_log("get_collect_conf: debug p_collect_conf->password              =%s#",p_collect_conf->password);
		err_log("get_collect_conf: debug p_collect_conf->path_str              =%s#",p_collect_conf->path_str);
		err_log("get_collect_conf: debug p_collect_conf->path_up               =%s#",p_collect_conf->path_up);
		err_log("get_collect_conf: debug p_collect_conf->path_last             =%s#",p_collect_conf->path_last);
		err_log("get_collect_conf: debug p_collect_conf->path_pre              =%s#",p_collect_conf->path_pre);
		err_log("get_collect_conf: debug p_collect_conf->path_suf              =%s#",p_collect_conf->path_suf);
		err_log("get_collect_conf: debug p_collect_conf->is_multi_path         =%d#",p_collect_conf->is_multi_path);
		err_log("get_collect_conf: debug p_collect_conf->file_str              =%s#",p_collect_conf->file_str);
		err_log("get_collect_conf: debug p_collect_conf->file_pre              =%s#",p_collect_conf->file_pre);
		err_log("get_collect_conf: debug p_collect_conf->file_suf              =%s#",p_collect_conf->file_suf);
		err_log("get_collect_conf: debug p_collect_conf->backup_path           =%s#",p_collect_conf->backup_path);
		err_log("get_collect_conf: debug p_collect_conf->is_backup             =%d#",p_collect_conf->is_backup);
		err_log("get_collect_conf: debug p_collect_conf->is_commit             =%d#",p_collect_conf->is_commit);
		err_log("get_collect_conf: debug p_collect_conf->is_interval_file      =%d#",p_collect_conf->is_interval_file);
		err_log("get_collect_conf: debug p_collect_conf->current_process_number=%d#",p_collect_conf->current_process_number);
	}
	return r;
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

/*
*parse_collect_conf()
*                      if return PARSE_MATCH,remote_path and remote_file_name are given
*
*/
parse_code_e parse_collect_conf(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name)
{
	
	parse_code_e  r;
	parse_code_e  r_file;
	parse_code_e  r_file_2;
	
	int           ftp_ret;
	char          dir_name[256];
	char          file_name[256];
	char          dirfromconf[256];
	char          filefromconf[256];
	
	FILE          * fp=NULL;
	char          tmp_file_name[256];
	int           tmp_collect_point;
	
	r=PARSE_UNMATCH;
	
	tmp_collect_point=0;
	memset(dirfromconf,0,sizeof(dirfromconf));
	memset(filefromconf,0,sizeof(filefromconf));
	
	sprintf(tmp_file_name,"%s/%06d_run.conf",CONF,p_collect_conf->collect_point);
	if(access(tmp_file_name,F_OK)==0)
	{
		fp=fopen(tmp_file_name,"r");
		if(fp==NULL)
		{
			err_log("parse_collect_conf: fopen %s fail\n",tmp_file_name);
			r=PARSE_FAIL;
			goto Exit_Pro;
		}
		fscanf(fp,"<%d>%s%s",&tmp_collect_point,dirfromconf,filefromconf);
		if(tmp_collect_point!=p_collect_conf->collect_point)
		{
			err_log("parse_collect_conf: %s content fail\n",tmp_file_name);
			r=PARSE_FAIL;
			goto Exit_Pro;
		}
		fclose(fp);
		fp=NULL;
	}
	
/************************************************************************/
/* alur changed:尝试修改端口以适应华为设备*/
/************************************************************************/
	/* Ftp_Init() */
	int iDefaultPort = 6621;
	ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,iDefaultPort,FTP_TIME_OUT,1,1,g_nDebug);
	printf("2.FTP premters: usr:%s, pwd:%s, ip:%s, port:%d.\n",p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,iDefaultPort);
// 	ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,0,FTP_TIME_OUT,1,1,debug);
	if(ftp_ret!=0)
	{
		err_log("parse_collect_conf: Ftp_Init fail,collect_point=%d\n",p_collect_conf->collect_point);
		r=PARSE_FAIL;
		goto Exit_Pro;
	}
	
	/* Ftp_Cd() */
	ftp_ret=Ftp_Cd(p_collect_conf->path_up);
	if(ftp_ret!=0)
	{
		err_log("parse_collect_conf: Ftp_Cd fail,collect_point=%d,dir=%s\n",p_collect_conf->collect_point,p_collect_conf->path_up);
		r=PARSE_FAIL;
		goto Exit_Pro;
	}
	
	/* Multi PATH */
	if(p_collect_conf->is_multi_path)
	{
		/* Multi PATH, Interval */
		if(p_collect_conf->is_interval_file)
		{
			memset(dir_name,0,sizeof(dir_name));
			memset(file_name,0,sizeof(file_name));
			
			r_file_2=parse_get_file_name_2(p_collect_conf,dirfromconf,filefromconf,dir_name,file_name);
			
			if(r_file_2==PARSE_FAIL || r_file_2==PARSE_UNMATCH)
			{
				r=r_file_2;
				goto Exit_Pro;
			}
			
			if(r_file_2==PARSE_MATCH)
			{
				strcpy(remote_path,dir_name);
				strcpy(remote_file_name,file_name);
				
				memset(dir_name,0,sizeof(dir_name));
				memset(file_name,0,sizeof(file_name));
				
				r_file_2=parse_get_file_name_2(p_collect_conf,remote_path,remote_file_name,dir_name,file_name);
				
				r=r_file_2;
				goto Exit_Pro;
			}
		}
		/* Multi PATH, NOT Interval */
		else
		{
			memset(dir_name,0,sizeof(dir_name));
			memset(file_name,0,sizeof(file_name));
			
			r_file_2=parse_get_file_name_2(p_collect_conf,dirfromconf,filefromconf,dir_name,file_name);
			
			if(r_file_2==PARSE_MATCH)
			{
				strcpy(remote_path,dir_name);
				strcpy(remote_file_name,file_name);
			}
			
			r=r_file_2;
			goto Exit_Pro;
		}
	}
	/* NOT Multi PATH */
	else
	{
		strcpy(remote_path,p_collect_conf->path_last);
		if(*remote_path!='\0')
		{  
			ftp_ret=Ftp_Cd(remote_path);
			if(ftp_ret!=0)
			{
				err_log("parse_collect_conf: Ftp_Cd 2 fail,collect_point=%d,dir=%s\n",p_collect_conf->collect_point,remote_path);
				r=PARSE_FAIL;
				goto Exit_Pro;
			}
		}  	
		/* NOT Multi PATH, Interval */
		if(p_collect_conf->is_interval_file)
		{
			memset(file_name,0,sizeof(file_name));
			
			r_file=parse_get_file_name(p_collect_conf,filefromconf,file_name);
			
			if(r_file==PARSE_FAIL || r_file==PARSE_UNMATCH)
			{
				r=r_file;
				goto Exit_Pro;
			}
			
			if(r_file==PARSE_MATCH)
			{
				strcpy(remote_file_name,file_name);
				
				memset(file_name,0,sizeof(file_name));
				
				r_file=parse_get_file_name(p_collect_conf,remote_file_name,file_name);
				
				r=r_file;
				goto Exit_Pro;
			}
			
			
		}
		/* NOT Multi PATH, NOT Interval */
		else
		{
			memset(file_name,0,sizeof(file_name));
			r_file=parse_get_file_name(p_collect_conf,filefromconf,file_name);
			
			if(r_file==PARSE_MATCH)
				strcpy(remote_file_name,file_name);
			
			r=r_file;
			goto Exit_Pro;
		}
	}
	
Exit_Pro:
	if(fp!=NULL)
		fclose(fp);
	/* Ftp_Close() */
	Ftp_Close();
	
	return r;
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
/* alur changed:尝试修改端口以适应华为设备*/
/************************************************************************/
	/* Ftp_Init() */
	int iDefaultPort = 6621;
	ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,iDefaultPort,FTP_TIME_OUT,1,1,g_nDebug);
	printf("1.FTP premters: usr:%s, pwd:%s, ip:%s, port:%d.\n",p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,iDefaultPort);
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

/*
*get_backup_name()
*             return 0 success, 1 fail
*
*/
int get_backup_name(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name,char * backup_name)
{
	int    r;
	
	char   tmp[512];
	char   tmp_path[256];
	char  *p_str=NULL;
	
	int    len=0;
	
	r=0;
	
	if(p_collect_conf->is_multi_path)
	{
		sprintf(backup_name,"%s/%s/%s",p_collect_conf->backup_path,remote_path,remote_file_name);
		sprintf(tmp,"%s/%s",p_collect_conf->backup_path,remote_path);
		tmp[sizeof(tmp)-1]='\0';
		if(access(tmp,F_OK)==-1)
		{
			if(mkdir(tmp,0755)!=0)
			{
				err_log("get_backup_name: mkdir %s fail\n",tmp);
				r=1;
				goto Exit_Pro;
			}
		}
	}
	else
	{
		memset(tmp_path,0,sizeof(tmp_path));
		p_str=strchr(remote_file_name,'.');
		if(p_str==NULL)
		{
			err_log("get_backup_name: get tmp_path from %s fail\n",remote_file_name);
			r=1;
			goto Exit_Pro;
		}
		
		len=strlen(remote_file_name);
		if(&remote_file_name[len-1]-p_str-1>=0)
		{
			if(&remote_file_name[len-1]-p_str>10)
				strncpy(tmp_path,p_str+1,10);
			else
				strncpy(tmp_path,p_str+1,&remote_file_name[len-1]-p_str);
		}
		else
		{
			err_log("get_backup_name: get tmp_path from %s fail\n",remote_file_name);
			r=1;
			goto Exit_Pro;
		}
		
		
		
		//??
		sprintf(backup_name,"%s/%s/%s",p_collect_conf->backup_path,tmp_path,remote_file_name);
		sprintf(tmp,"%s/%s",p_collect_conf->backup_path,tmp_path);
		tmp[sizeof(tmp)-1]='\0';
		if(access(tmp,F_OK)==-1)
		{
			if(mkdir(tmp,0755)!=0)
			{
				err_log("get_backup_name: mkdir %s fail\n",tmp);
				r=1;
				goto Exit_Pro;
			}
		}
	}
	
Exit_Pro:
	return r;
}

/*
*backup_file()
*              return 0 success, 1 fail
*
*/
int backup_file(collect_conf * p_collect_conf,char * remote_path,char * remote_file_name)
{
	int  ret;
	
	FILE *fp_i=NULL;
	FILE *fp_o=NULL;
	
	char backup_name[512];
	
	char tmp_file_name[512];
	
	int  r_len;
	int  w_len;
	int  buff_len;
	char buff[8192];
	
	ret=0;
	
	/*判断是否备份*/ 
	if(p_collect_conf->is_backup)
	{ 
		sprintf(tmp_file_name,"%s/%03d_%s",WORK,p_collect_conf->current_process_number,remote_file_name);
		
		memset(backup_name,0,sizeof(backup_name));
		if(get_backup_name(p_collect_conf,remote_path,remote_file_name,backup_name)!=0)
		{
			err_log("backup_file: get_backup_name fail\n");
			ret=1;
			goto Exit_Pro;
		}	
		backup_name[sizeof(backup_name)-1]='\0';
		
		fp_i=fopen(tmp_file_name,"r");
		if(fp_i==NULL)
		{
			err_log("backup_file: fopen %s fail\n",tmp_file_name);
			ret=1;
			goto Exit_Pro;
		}
		
		fp_o=fopen(backup_name,"w+");
		if(fp_o==NULL)
		{
			err_log("backup_file: fopen %s fail\n",backup_name);
			ret=1;
			goto Exit_Pro;
		}
		
		buff_len=sizeof(buff);
		while(1)
		{
			r_len=fread(buff,1,buff_len,fp_i);
			if(r_len>0)
			{
				w_len=fwrite(buff,1,r_len,fp_o);
				if(w_len<r_len)
				{
					err_log("backup_file: fwrite %s fail\n",backup_name);
					ret=1;
					goto Exit_Pro;
				}
			}
			if(r_len<buff_len)
			{
				if( ferror(fp_i) )
				{
					err_log("backup_file: fread %s fail\n",tmp_file_name);
					ret=1;
					goto Exit_Pro;
				}
				break;
			}
		}
	} 	
Exit_Pro:
	if(fp_i!=NULL) fclose(fp_i);
	if(fp_o!=NULL) fclose(fp_o);
	
	return ret;
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
			//ftp get文件
			printf("Try to Execute get_remote_file.\n");
			if(get_remote_file(&oMyCollectConf,szRemotePath,szRemoteFileName,&lFileSize)!=0)
			{
				err_log("point_collect: nConfLineNo=%d,szRemoteFileName=%s,get_remote_file FAIL\n",nConfLineNo,szRemoteFileName);
				return 1;
			}
			//备份文件
			if(backup_file(&oMyCollectConf,szRemotePath,szRemoteFileName)!=0)
			{
				err_log("point_collect: nConfLineNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nConfLineNo,szRemoteFileName);
				return 1;
			}
			
			get_time(cEnd);
			
			//提交文件
			if(commit_file(&oMyCollectConf,szRemotePath,szRemoteFileName,cBegin,cEnd,lFileSize)!=0)
			{
				err_log("point_collect: nConfLineNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nConfLineNo,szRemoteFileName);
				return 1;
			}
			
			continue;
		}
		
		err_log("point_collect: nConfLineNo=%d,parse_collect_conf return UNKNOW\n",nConfLineNo);
		return 1;
	}
	
	
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
