/*****************************************************************/
/*                                                               */
/* pretreat_v20.c                                                */
/*                                                               */ 
/*                       话单预处理程序 v2.0 2005.07 haojianting */
/*****************************************************************/

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

//----------------------------------------------------------------------
extern int   errno;

//----------------------------------------------------------------------
#define      SEMKEY            878

#define      MAX_MODULE        128

#define      MAX_CHILD_PROCESS 128
#define      SLEEP_TIME        60

#define      WORK     "./work"
#define      LOG      "./log"
#define      CONF     "./conf"

#define      ERR_LOG_FILE        "./log/pretreat.log"
#define      PREFIX_RUN_LOG_FILE "./log/pretreat_run"

//----------------------------------------------------------------------
typedef unsigned char  u_1;
typedef unsigned short u_2;
typedef unsigned int   u_4;

typedef int funHandler(char *, char *, int *);  //tmp_in_file_name, tmp_out_file_name, rec_num

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

//----------------------------------------------------------------------
/*用于每个子进程设置其编号*/
int          curr_process_number    = 0;

char *       progname               = NULL;
char *       file_out_dir           = "./out";
char *       file_in_dir            = "./in" ;
char *       run_dir                = "./";
int          debug                  = 0;
int          parallel_child_process = 1;

t_module     module[MAX_MODULE];

//----------------------------------------------------------------------
static void  usage(int status);
int          get_time(char * par);
void         err_log(char * format,...);
void         daemon_start(void);
void         P(void);
void         V(void);

parse_code_e pre_suf_check(char * name,char * prefix,char * suffix);
parse_code_e get_orig_file_name(char * dir_name,char * prefix,char * suffix,char * ret_file_name);

int          get_commit_name(char * in_file_name,char * commit_name);
int          commit_file(char * in_file_name,char * begin,char * end,long in_file_size,long out_file_size,long rec_num);

int          pretreat(char * in_file_name);

parse_code_e get_in_file_name(int parallel_number,char * dir_name,char * prefix,char * suffix,char * ret_file_name);
void         process_pretreat(int current_number,int parallel_number);

//----------------------------------------------------------------------


/*
 *  Display the syntax for starting this program.
 */
static void usage(int status)
{
        FILE *output = status?stderr:stdout;
        
        fprintf(output,"Usage: %s [-o file commit path] [-i file from path] [-r run path] [-p parallel child number] [-d]\n",progname);
        fprintf(output,"\nOptions:\n");
        fprintf(output,"        -o changes the file commit directory to that specified in path, default is ./out\n");
        fprintf(output,"        -i changes the file from directory to that specified in path, default is ./in\n");
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


void err_log(char * format,...)
{
	time_t t;
	struct tm *systime;
	FILE *fp;
	
	char   tmp[256];

	va_list ap;

	if(debug)
	{
  		va_start(ap, format);
  		vprintf(format,ap);
  		va_end(ap);
  		printf("\n");
	}

        if(curr_process_number!=0)
        {
        	sprintf(tmp,"%s.%03d",ERR_LOG_FILE,curr_process_number);
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

void daemon_start(void)
{
     int childpid;

     umask(022);

     if(getppid() == 1 ) {  return; }

     signal(SIGTTOU,SIG_IGN);
     signal(SIGTTIN,SIG_IGN);
     signal(SIGTSTP,SIG_IGN);
     signal(SIGUSR1,SIG_IGN);

     if(setpgrp()==-1)
            err_log("daemon_start: can't change process group\n");
     signal(SIGHUP,SIG_IGN);

     
     if((childpid=fork())<0) {
            err_log("daemon_start: fork error\n");
            exit(1);
     }
     else if(childpid>0)
             exit(0);
}

void P(void)
{
	int           sem_id;
    	struct sembuf sem_buff;
	
	sem_id=semget(SEMKEY,1,0);
	if(sem_id==-1)
	{
		err_log("P: semget SEMKEY %d fail\n",SEMKEY);
		exit(1);
	}
    /*-----------------------------------------------*/
    /*  semphore p oprate                            */
    /*-----------------------------------------------*/
	sem_buff.sem_num = 0;
	sem_buff.sem_op  = -1;
	sem_buff.sem_flg = SEM_UNDO;
	
	while(semop(sem_id,&sem_buff,1)==-1)
	{
		if(errno==EINTR||errno==EAGAIN)
		{
			err_log("P: semop SEMKEY %d EINTR or EAGAIN errno=%d\n",SEMKEY,errno);
		}
		else
		{
			err_log("P: semop SEMKEY %d fail errno=%d\n",SEMKEY,errno);
			exit(1);
		}
	}

}

void V(void)
{
	int           sem_id;
    	struct sembuf sem_buff;
	
	sem_id=semget(SEMKEY,1,0);
	if(sem_id==-1)
	{
		err_log("V: semget SEMKEY %d fail\n",SEMKEY);
		exit(1);
	}
    /*-----------------------------------------------*/
    /*  semphore v oprate                            */
    /*-----------------------------------------------*/
	sem_buff.sem_num = 0;
	sem_buff.sem_op  = 1;
	sem_buff.sem_flg = SEM_UNDO;

	while(semop(sem_id,&sem_buff,1)==-1)
	{
		if(errno==EINTR||errno==EAGAIN)
		{
			err_log("V: semop SEMKEY %d EINTR or EAGAIN errno=%d\n",SEMKEY,errno);
		}
		else
		{
			err_log("V: semop SEMKEY %d fail errno=%d\n",SEMKEY,errno);
			exit(1);
		}
	}
	
}


int main(int argc,char * argv[])
{
	int     argval;
	struct  stat stat_buff;
	char    file_name[256];
	char    tmp_file_name_2[512];
	int     i;

        t_child_process_status child_process_status[MAX_CHILD_PROCESS];

	pid_t   childpid;
	pid_t   r_waitpid;
	
	int     sem_id;
	int     sem_in;

	FILE       * fp=NULL;
	char	     buff[2048];
	char         content[3][256];
	int          r;
        void       * handler_dlopen = NULL;
        const char * dlError        = NULL;

	
        if ((progname = strrchr(argv[0], '/')) == NULL)
                progname = argv[0];
        else
                progname++;
	
 	/*  Process the options.  */
        while ((argval = getopt(argc, argv, "o:i:r:p:d")) != EOF) {

                switch(argval) {

                        case 'o':
                        	file_out_dir = strdup(optarg);
                                break;	
                        case 'i':
                        	file_in_dir = strdup(optarg);
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


	/* Change dir */
	if(chdir(run_dir)==-1)
	{
		err_log("main: chdir to %s fail\n",run_dir);
		return 1;
	}

	/* Check dir WORK*/
	if(stat(WORK,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",WORK);
		return 1;
	}
	else
	{
	        if(!S_ISDIR(stat_buff.st_mode))	
	        {
	        	err_log("main: %s isn't a dir\n",WORK);
	        	return 1;
	        }
	}

	/* Check dir LOG*/
	if(stat(LOG,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",LOG);
		return 1;
	}
	else
	{
	        if(!S_ISDIR(stat_buff.st_mode))	
	        {
	        	err_log("main: %s isn't a dir\n",LOG);
	        	return 1;
	        }
	}

	/* Check dir CONF*/
	if(stat(CONF,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",CONF);
		return 1;
	}
	else
	{
	        if(!S_ISDIR(stat_buff.st_mode))	
	        {
	        	err_log("main: %s isn't a dir\n",CONF);
	        	return 1;
	        }
	}

	/* Check dir file_out_dir*/
	if(stat(file_out_dir,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",file_out_dir);
		return 1;
	}
	else
	{
	        if(!S_ISDIR(stat_buff.st_mode))	
	        {
	        	err_log("main: %s isn't a dir\n",file_out_dir);
	        	return 1;
	        }
	}

	/* Check dir file_in_dir*/
	if(stat(file_in_dir,&stat_buff)==-1)
	{
		err_log("main: stat %s fail\n",file_in_dir);
		return 1;
	}
	else
	{
	        if(!S_ISDIR(stat_buff.st_mode))	
	        {
	        	err_log("main: %s isn't a dir\n",file_in_dir);
	        	return 1;
	        }
	}

	
	/*verify parallel_child_process*/
	if(parallel_child_process<=0)                parallel_child_process=1;
	if(parallel_child_process>MAX_CHILD_PROCESS) parallel_child_process=MAX_CHILD_PROCESS;
	
	if(debug)
	{
		fprintf(stdout,"main: file_out_dir:%s#\n",file_out_dir);
		fprintf(stdout,"main: file_in_dir:%s#\n",file_in_dir);
		fprintf(stdout,"main: run_dir:%s#\n",run_dir);
		fprintf(stdout,"main: parallel_child_process:%d#\n",parallel_child_process);
	}

	/*清理work目录下文件*/
	memset(file_name,0,sizeof(file_name));
	while(get_orig_file_name(WORK,NULL,NULL,file_name)==PARSE_MATCH)
	{
                sprintf(tmp_file_name_2,"%s/%s",WORK,file_name);

 		err_log("main: clear file:%s#\n",tmp_file_name_2);
		if ( unlink(tmp_file_name_2)!=0 )
		{
			err_log("main: unlink file %s fail\n",tmp_file_name_2);
			return 1;
		}
		memset(file_name,0,sizeof(file_name));
        }

	/*semphore init*/
	sem_id=semget(SEMKEY,1,0666|IPC_CREAT);
	if(sem_id==-1) {
		err_log("main: semget SEMKEY %d fail\n",SEMKEY);
		exit(1);
	}
	sem_in=1;
	/*Linux*/
	//if(semctl(sem_id,0,SETVAL,sem_in)==-1) {
	//	err_log("main: semctl SEMKEY %d fail\n",SEMKEY);
	//	exit(1);
	//}
	/*Solaris*/
	if(semctl(sem_id,0,SETVAL,&sem_in)==-1) {
		err_log("main: semctl SEMKEY %d fail\n",SEMKEY);
		exit(1);
	}

	/*装载 预处理模块(动态库)*/
	memset(module,0,sizeof(module));
	sprintf(file_name,"%s/%s",CONF,"module.conf");

	fp=fopen(file_name,"r");
	if(fp==NULL)
	{
		err_log("main: fopen %s fail\n",file_name);
		exit(1);
	}

	r=0;

	while(1)
	{
	    memset(buff,0,sizeof(buff));
	    memset(content,0,sizeof(content));
       	    if(fgets(buff,sizeof(buff),fp)==NULL)
       		break;
       		
       	    r++;
	    
	    if( r>MAX_MODULE )
	    {
	    	err_log("main: module.conf line>%d\n",MAX_MODULE);
	    	exit(1);
	    }
	    
       	    if(buff[0]!='#')
       	    {     	
       		if( sscanf(buff,"%s%s%s",content[0],content[1],content[2]) != 3 )
       		{
       			err_log("main: module.conf line %d incorrect\n",r);
       			exit(1);
       		}
       		
	       	if( atoi(content[0]) != r )
	       	{
       			err_log("main: module.conf line %d incorrect\n",r);
       			exit(1);
	       	}
	       	
	       	//装载动态库
		//----------------------------------------------------
    		handler_dlopen = dlopen(content[1],RTLD_LAZY);
    		if( handler_dlopen == NULL)
    		{
    			dlError=dlerror();
       			err_log("main: dlopen %s fail, %s\n",content[1],dlError);
    			goto Exit_Pro;
    		}
		
		module[r-1].handler_dlopen = handler_dlopen;
		
    		module[r-1].module_fun=dlsym( handler_dlopen, content[2]);
    		if( module[r-1].module_fun == NULL )
    		{
    			dlError=dlerror();
    			printf("dlsym fail, %s\n",dlError);
       			err_log("main: dlsym module:%s, fun:%s fail, %s\n",content[1],content[2],dlError);
       			goto Exit_Pro;
    		}
    		
    		module[r-1].isload=1;
    		
    		err_log("main: successful load module:%s, fun:%s\n",content[1],content[2]);
		//----------------------------------------------------
       	    }
	}
	
	fclose(fp);
	fp=NULL;
	        
	/*初始化 子进程结构数组*/
        for(i=0;i<MAX_CHILD_PROCESS;i++)
        	memset(&child_process_status[i],0,sizeof(t_child_process_status));
        
        if(debug==0)
        	daemon_start();
        
	/*预处理*/
	while(1)
	{
		/*创建子进程*/
		for(i=0;i<parallel_child_process;i++)
		{
			if(child_process_status[i].pid==0&&child_process_status[i].sleep_time<=0)
			{
				if((childpid=fork())<0) /*fork error*/
				{
            				err_log("main: fork error\n");
            				exit(1);
     				}
     				else if(childpid>0)     /**/ 
     				{
     					child_process_status[i].pid        = childpid;
     					child_process_status[i].sleep_time = SLEEP_TIME;
     				}
     				else if(childpid==0)    /*child process*/
     				{
					process_pretreat(i+1,parallel_child_process);
     					exit(0);
     				}
             		}
		}
	
		/*回收子进程*/
		for(i=0;i<parallel_child_process;i++)
		{
			if(child_process_status[i].pid>0)
			{
				r_waitpid=waitpid(child_process_status[i].pid,NULL,WNOHANG);
				if(r_waitpid>0)
				{
					child_process_status[i].pid=0;
				}
				if(r_waitpid<0)
				{
					err_log("main: waitpid fail\n");
					exit(1);
				}
			}
		}
		
		sleep(1);
		
		/*每秒递减sleep_time*/
		for(i=0;i<parallel_child_process;i++)
		{
        		if(child_process_status[i].pid==0&&child_process_status[i].sleep_time>0)
        			child_process_status[i].sleep_time--;
		}
	}
    	
Exit_Pro:
	if( fp != NULL )
		fclose(fp);
	for( i=0; i<MAX_MODULE; i++ )
	{
    		if( module[i].handler_dlopen != NULL )
    			dlclose(module[i].handler_dlopen);
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
		if( pre_suf_check(pdirent->d_name,prefix,suffix) != PARSE_MATCH )
			continue;
		
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


/*
 *get_commit_name()
 *             return 0 success, 1 fail
 *
 */
int get_commit_name(char * in_file_name,char * commit_name)
{
	int    r;

	r=0;
	
	sprintf(commit_name,"%s.pre",in_file_name);
	
	return r;
}

/*
 *commit_file()
 *              return 0 success, 1 fail
 *
 */
int commit_file(char * in_file_name,char * begin,char * end,long in_file_size,long out_file_size,long rec_num)
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
        

        //link工作目录文件至提交目录
        memset(commit_name,0,sizeof(commit_name));
	if(get_commit_name(in_file_name,commit_name)!=0)
	{
		err_log("commit_file: get_commit_name fail\n");
		ret=1;
		goto Exit_Pro;
	}
	commit_name[sizeof(commit_name)-1]='\0';
	
	sprintf(tmp_file_name,"%s/%03d_%s",WORK,curr_process_number,in_file_name);
	sprintf(tmp_file_name_2,"%s/%s",file_out_dir,commit_name);
	
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

        //记录pretreat_run.YYYYMMDD文件
	get_time(run_log_time);
	run_log_time[8]='\0';
	
	sprintf(tmp_file_name,"%s.%s",PREFIX_RUN_LOG_FILE,run_log_time);

        P();		
	fp=fopen(tmp_file_name,"a+");
	if(fp==NULL)
	{
	        V();
		err_log("commit_file: fopen %s fail\n",tmp_file_name);
		ret=1;
		goto Exit_Pro;
	}
	fprintf(fp,"%s %s %s %ld %ld %ld\n",in_file_name,begin,end,in_file_size,out_file_size,rec_num);
	fclose(fp);
	fp=NULL;	
        V();  

        //清理work目录下,xxx_ 开头文件,xxx为子进程编号
	memset(file_name,0,sizeof(file_name));
	sprintf(pre,"%03d",curr_process_number);
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

        //unlink 输入文件in_file_name
        sprintf(tmp_file_name_2,"%s/%s",file_in_dir,in_file_name);

	if ( unlink(tmp_file_name_2)!=0 )
	{
		err_log("commit_file: unlink file %s fail\n",tmp_file_name_2);
		ret=1;
		goto Exit_Pro;
	}
	
	
Exit_Pro:
	if(fp!=NULL) fclose(fp);
	
	return ret;
}

/*
 *pretreat()
 *           return 0 success, 1 fail
 *
 */
int pretreat(char * in_file_name)
{
	int    ret;

	struct stat stat_buff;
	
	char   tmp_file_name[512];
	char   tmp_in_file_name[512];
	char   tmp_out_file_name[512];

	long   in_file_size;
	long   out_file_size;

	int    rec_num;
	char   begin[15];
	char   end[15];
	
	int    num_module;
	int    point;
	char   tmp[7];
	
	int    r,len;
	char   buff[2048];
	char   content[2][256];
	FILE * fp=NULL;

	ret     = 0;
	rec_num = 0;

	
	sprintf(tmp_in_file_name,"%s/%s",file_in_dir,in_file_name);
	sprintf(tmp_out_file_name,"%s/%03d_%s",WORK,curr_process_number,in_file_name);
	
	get_time(begin);

	/*取文件对应模块号*/
	num_module=0;
	r=0;
	
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp,in_file_name,6);
	point=atoi(tmp);
	
	sprintf(tmp_file_name,"%s/pretreat.conf",CONF);
	fp=fopen(tmp_file_name,"r");
	if(fp==NULL)
	{
		err_log("pretreat: fopen %s fail\n",tmp_file_name);
		ret=1;
		goto Exit_Pro;
	}
	
	while(1)
	{
	    memset(buff,0,sizeof(buff));
	    memset(content,0,sizeof(content));
       	    if(fgets(buff,sizeof(buff),fp)==NULL)
       		break;
       		
       	    r++;

       	    if(buff[0]!='#')
       	    {     	
       		if( sscanf(buff,"%s%s",content[0],content[1] ) != 2 )
       		{
       			err_log("pretreat: pretreat.conf line %d incorrect\n",r);
       			ret=1;
       			goto Exit_Pro;
       		}
       		
	       	len=strlen(content[0]);
	       	if(len!=8 || content[0][0]!='<' || content[0][len-1]!='>')
	       	{
	       		err_log("pretreat: pretreat.conf line %d incorrect\n",r);
       			ret=1;
       			goto Exit_Pro;
	       	}
	       		
	       	content[0][len-1]='\0';
	       	if(atoi(&content[0][1])!=r)
	       	{
       			err_log("pretreat: pretreat.conf line %d incorrect\n",r);
       			ret=1;
       			goto Exit_Pro;
	       	}
	       	
	       	if( r == point )
	       		num_module=atoi(content[1]);
       	    }
	}
	
	fclose(fp);
	fp=NULL;
	
	if( num_module < 1 )
	{
		err_log("pretreat: file %s, pretreat.conf no this point\n",in_file_name);
		ret=1;
		goto Exit_Pro;
	}
	
	/*调用对应模块的处理函数*/
	if( module[num_module-1].isload != 1 )
	{
		err_log("pretreat: file %s, module %d not load\n",in_file_name,num_module);
		ret=1;
		goto Exit_Pro;
	}

	if( module[num_module-1].module_fun(tmp_in_file_name,tmp_out_file_name,&rec_num) != 0 )
	{
		err_log("pretreat: file %s, module %d module_fun fail\n",in_file_name,num_module);
		ret=1;
		goto Exit_Pro;
	}

	get_time(end);

	/*取输入文件大小*/
	sprintf(tmp_file_name,"%s/%s",file_in_dir,in_file_name);
	if(stat(tmp_file_name,&stat_buff)==-1)
	{
		err_log("pretreat: stat %s fail\n",tmp_file_name);
		ret=1;
		goto Exit_Pro;
	}
	in_file_size=stat_buff.st_size;
	
	/*取输出文件大小*/
	sprintf(tmp_file_name,"%s/%03d_%s",WORK,curr_process_number,in_file_name);
	if(stat(tmp_file_name,&stat_buff)==-1)
	{
		err_log("pretreat: stat %s fail\n",tmp_file_name);
		ret=1;
		goto Exit_Pro;
	}
	out_file_size=stat_buff.st_size;

        /*提交文件*/
	if( commit_file(in_file_name,begin,end,in_file_size,out_file_size,rec_num) != 0 )
	{
		err_log("pretreat: commit_file %s fail\n",in_file_name);
		ret=1;
		goto Exit_Pro;
	}
	
	
Exit_Pro:
	if( fp != NULL )
		fclose(fp);
	return ret;
}


/*
 *  get_in_file_name()
 */
parse_code_e get_in_file_name(int parallel_number,char * dir_name,char * prefix,char * suffix,char * ret_file_name)
{
	int              ret;
	DIR *            pdir=NULL;
	struct dirent *	 pdirent;
	struct stat stat_buff;
	char             tmp_file_name[512];
	
	char             name_pre[7];
	
	ret=PARSE_UNMATCH;

	if(dir_name==NULL)
	{
		err_log("get_in_file_name: dir_name is NULL\n");
		ret=PARSE_FAIL;
		goto Exit_Pro;	
	}
	
	pdir=opendir(dir_name);
		
	if(pdir==NULL)
	{
		err_log("get_in_file_name: opendir fail\n");
		ret=PARSE_FAIL;
		goto Exit_Pro;	
	}
	while( (pdirent=readdir(pdir))!=NULL )
	{
		/*前后缀匹配检查*/
		if( pre_suf_check(pdirent->d_name,prefix,suffix) != PARSE_MATCH )
			continue;
		
		/*要求文件名前6个字节是数字*/
		if(strlen(pdirent->d_name)<6)
			continue;
		if(!isdigit(pdirent->d_name[0]))
			continue;
		if(!isdigit(pdirent->d_name[1]))
			continue;
		if(!isdigit(pdirent->d_name[2]))
			continue;
		if(!isdigit(pdirent->d_name[3]))
			continue;
		if(!isdigit(pdirent->d_name[4]))
			continue;
		if(!isdigit(pdirent->d_name[5]))
			continue;

		/* 要求 采集点编号%子进程并发数=当前子进程编号 */
		strncpy(name_pre,pdirent->d_name,6);
		name_pre[6]='\0';
		if( curr_process_number == parallel_number )
		{
			if( 0!=atoi(name_pre)%parallel_number)
				continue;
		}
		else
		{
			if( curr_process_number!=atoi(name_pre)%parallel_number)
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
			err_log("get_in_file_name: stat %s fail\n",pdirent->d_name);
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
	if(debug)
	{
		P();
		printf("get_in_file_name:  ret_file_name=%s#, curr_process_number=%d\n",ret_file_name,curr_process_number);
		V();
	}
	return ret;
}


/*
 *process_pretreat()
 *
 *
 */
void process_pretreat(int current_number,int parallel_number)
{
	char    file_name[256];

	/*设置全局子进程编号*/
	curr_process_number=current_number;


	memset(file_name,0,sizeof(file_name));
	while(get_in_file_name(parallel_number,file_in_dir,NULL,NULL,file_name)==PARSE_MATCH)
	{
		/*文件预处理*/
		if(pretreat(file_name)!=0)
		{
			err_log("process_pretreat: pretreat fail\n");
			break;
		}
		
		memset(file_name,0,sizeof(file_name));
        }
	
	return;
}

