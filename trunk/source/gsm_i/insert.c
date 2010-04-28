/****************************************************/
/*                                                  */
/* insert.c                                         */
/*                                                  */ 
/*          话单入库程序 v1.0 2005.06 haojianting   */
/****************************************************/

#include <ctpublic.h>
#include "insert.h"
#include "exutils.h"

/*-----------------------------------------------*/
/*用于每个子进程设置其编号*/
int          curr_process_number    = 0;

char *       progname               = NULL;
char *       file_in_dir            = "./in" ;
char *       run_dir                = "./";
int          debug                  = 0;
int          parallel_child_process = 1;

char *       db_user                = "";
char *       db_password            = "";
char *       db_server              = "";

CS_CONTEXT  *Ex_context             = NULL;
/*-----------------------------------------------*/


/*
 *  Display the syntax for starting this program.
 */
static void usage(int status)
{
        FILE *output = status?stderr:stdout;
        
        fprintf(output,"Usage: %s [-i file from path] [-r run path] [-p parallel child number] [-u db_user] [-w db_password] [-s db_server] [-d]\n",progname);
        fprintf(output,"\nOptions:\n");
        fprintf(output,"        -i changes the file from directory to that specified in path, default is ./in\n");
        fprintf(output,"        -r changes the run directory to that specified in path, default is ./\n");
        fprintf(output,"        -p changes the parallel child number, default is 1\n");
        fprintf(output,"        -u specify db_user\n");
        fprintf(output,"        -w specify db_password\n");
        fprintf(output,"        -s specify db_server\n");
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

void strtoint(u_8 **in, int *out)
{
	u_8 * p_str=NULL;

	p_str=strchr(*in,SEPARATE_CHR);
	if(p_str!=NULL)
	{
		*p_str='\0';
		sscanf(*in,"%d",out);
		p_str++;
		*in=p_str;
	}
	else
	{
		p_str=strchr(*in,'\n');
		if(p_str!=NULL)
		{
			*p_str='\0';
			sscanf(*in,"%d",out);
			*in=p_str;
		}
	}
}

void strtostr(u_8 **in, u_8 *out)
{
	u_8 * p_str=NULL;

	p_str=strchr(*in,SEPARATE_CHR);
	if(p_str!=NULL)
	{
		*p_str='\0';
		strcpy(out,*in);
		p_str++;
		*in=p_str;
	}
	else
	{
		p_str=strchr(*in,'\n');
		if(p_str!=NULL)
		{
			*p_str='\0';
			strcpy(out,*in);
			*in=p_str;
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
	
    if ((progname = strrchr(argv[0], '/')) == NULL)
            progname = argv[0];
    else
            progname++;
	
 	/*  Process the options.  */
    while ((argval = getopt(argc, argv, "i:r:p:u:w:s:d")) != EOF) {
        switch(argval) {
            case 'i':
                file_in_dir = strdup(optarg);
                    break;	
            case 'r':
                run_dir = strdup(optarg);
                    break;	
            case 'p':
                parallel_child_process = atoi(optarg);
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


	/* Change dir */
	if(chdir(run_dir)==-1)
	{
		err_log("main: chdir to %s fail\n",run_dir);
		return 1;
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
		fprintf(stdout,"main: file_in_dir:%s#\n",file_in_dir);
		fprintf(stdout,"main: run_dir:%s#\n",run_dir);
		fprintf(stdout,"main: parallel_child_process:%d#\n",parallel_child_process);
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
                    process_insert(i+1,parallel_child_process);
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
 *commit_file()
 *              return 0 success, 1 fail
 *
 */
int commit_file(char * in_file_name,char * begin,char * end,long in_file_size,long rec_num,long db_rec_num)
{
	int   ret;
	
	FILE  * fp=NULL;
	
	char  run_log_time[15];
	char  tmp_file_name[512];
	char  tmp_file_name_2[512];

	ret=0;
        

        //记录insert_run.YYYYMMDD文件
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
	fprintf(fp,"%s %s %s %ld %ld %ld\n",in_file_name,begin,end,in_file_size,rec_num,db_rec_num);
	fclose(fp);
	fp=NULL;	
        V();  

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
 *insert()
 *           return 0 success, 1 fail
 *
 */
int insert(char * in_file_name,int idb_num)
{
	int             ret;
	
	FILE           *fp_i=NULL;
	struct          stat stat_buff;
	char            tmp_file_name[512];
	char            buff[2048];
	char           *p_tmp=NULL;
	int             rec_num;
	int             db_rec_num;
	char            begin[15];
	char            end[15];
	long            in_file_size;
	t_cdr           cdr;
	char            cmd[1024];
	
	CS_CONNECTION  *connection=NULL;
	CS_RETCODE	retcode;
	
	
	ret        = 0;
	rec_num    = 0;
	db_rec_num = 0;


	get_time(begin);

	sprintf(tmp_file_name,"%s/%s",file_in_dir,in_file_name);
	fp_i=fopen(tmp_file_name,"r");
	if(fp_i==NULL)
	{
        	err_log("insert: fopen %s fail\n",tmp_file_name);
        	ret=1;
        	goto Exit_Pro;
	}
	
	while(1)
	{
		memset(buff,0,sizeof(buff));

        	if(fgets(buff,sizeof(buff),fp_i)==NULL)
        		break;
        	if(buff[strlen(buff)-1]!='\n')
        		break;

	    /*取字段*/
	    if(rec_num==0)
	    {
		memset(&cdr,0,sizeof(cdr));
		p_tmp=buff;

                strtostr(&p_tmp, cdr.file_id);
                strtoint(&p_tmp,&cdr.serial_num);
                       
                strtostr(&p_tmp, cdr.add_cdr_switch);
                       
                strtostr(&p_tmp, cdr.add_hlr);
                       
                strtostr(&p_tmp, cdr.add_my_area_code);
                strtostr(&p_tmp, cdr.add_my_area_name);
                       
                strtostr(&p_tmp, cdr.add_my_scp);
                strtostr(&p_tmp, cdr.add_product_package);
                strtostr(&p_tmp, cdr.add_a_flag);
                strtostr(&p_tmp, cdr.add_a_name);
                strtostr(&p_tmp, cdr.add_u_type);
                strtostr(&p_tmp, cdr.add_u_group);
                strtostr(&p_tmp, cdr.add_gc);
                       
                strtoint(&p_tmp,&cdr.add_time_diff);
                       
                strtostr(&p_tmp, cdr.add_layout_time);
                strtostr(&p_tmp, cdr.add_call_begin_time);
                strtoint(&p_tmp,&cdr.add_occupy_duration);
                strtoint(&p_tmp,&cdr.add_talk_duration);
                       
                strtostr(&p_tmp, cdr.add_record_type);
                       
                strtostr(&p_tmp, cdr.add_call_type);
                       
                strtostr(&p_tmp, cdr.add_cf_type);
                       
                strtostr(&p_tmp, cdr.add_my_imsi);
                strtostr(&p_tmp, cdr.add_my_imei);
                       
                strtostr(&p_tmp, cdr.add_my_dn);
                strtostr(&p_tmp, cdr.add_other_dn);
                strtostr(&p_tmp, cdr.add_third_party);
                       
                strtostr(&p_tmp, cdr.add_my_msrn);
                strtostr(&p_tmp, cdr.add_other_msrn);
                       
                strtostr(&p_tmp, cdr.add_other_a_flag);
                strtostr(&p_tmp, cdr.add_other_a_name);
                       
                strtostr(&p_tmp, cdr.add_sub_my_dn);
                strtostr(&p_tmp, cdr.add_sub_other_dn);
                       
                strtostr(&p_tmp, cdr.add_other_direction);
                       
                strtoint(&p_tmp,&cdr.add_my_cell_num);
                strtoint(&p_tmp,&cdr.add_other_cell_num);
                       
                strtostr(&p_tmp, cdr.add_in_trunk_group_id);
                strtoint(&p_tmp,&cdr.add_in_cic_pcm);
                strtoint(&p_tmp,&cdr.add_in_cic_ts);
                       
                strtostr(&p_tmp, cdr.add_out_trunk_group_id);
                strtoint(&p_tmp,&cdr.add_out_cic_pcm);
                strtoint(&p_tmp,&cdr.add_out_cic_ts);
                       
                strtostr(&p_tmp, cdr.add_phone_manufacturer);
                strtostr(&p_tmp, cdr.add_phone_type);
                       
                strtostr(&p_tmp, cdr.charging_date);
                strtostr(&p_tmp, cdr.answer_time);
                strtostr(&p_tmp, cdr.disconnect_reason);
                strtostr(&p_tmp, cdr.billing_id);
                       
                strtoint(&p_tmp,&cdr.record_type);
                strtoint(&p_tmp,&cdr.call_type);
                strtostr(&p_tmp, cdr.reason_for_termination);
                strtostr(&p_tmp, cdr.served_dn);
                strtostr(&p_tmp, cdr.dialled_other_party);
                strtostr(&p_tmp, cdr.translated_other_party);
                strtostr(&p_tmp, cdr.third_party);
                strtostr(&p_tmp, cdr.service_centre_address);
                strtoint(&p_tmp,&cdr.call_hold_count);
                strtoint(&p_tmp,&cdr.call_wait_count);
                strtostr(&p_tmp, cdr.ss_code);
                strtostr(&p_tmp, cdr.exchange_id);
            }
	    rec_num++;
	}
	
	fclose(fp_i); 
	fp_i=NULL;

	/*调用bcp*/
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"bcp idb_bill_%02d in %s/%s -c -t , -Jcp936 -U%s -P%s -S%s -b 1000 -A8192", \
	         idb_num, file_in_dir, in_file_name, db_user, db_password, db_server);
	printf("cmd:%s#\n",cmd);
	if( system(cmd) == -1 )
	{
		err_log("insert: call bcp fail\n");
		ret=1;
		goto Exit_Pro;
	}
	
        /*调用存储过程完成insert detail, 提交接口表*/
	retcode = ex_init(&Ex_context);
	if (retcode != CS_SUCCEED)
	{
		err_log("insert: ex_init fail\n");
		ret=1;
		goto Exit_Pro;
	}

	retcode = ex_connect(Ex_context, &connection, NULL, db_user, db_password, db_server);
	if (retcode != CS_SUCCEED)
	{
		err_log("insert: ex_connect fail\n");
		ret=1;
		goto Exit_Pro;
	}

	retcode = ex_execute_cmd(connection,"set chained off");
	if (retcode != CS_SUCCEED)
	{
		err_log("insert: db set chained off fail\n");
		ret=1;
		goto Exit_Pro;
	}
	if( do_proc_after_insert(connection,idb_num,cdr.add_cdr_switch,cdr.file_id,&db_rec_num) != 0 )
	{
		err_log("insert: do_proc_after_insert fail\n");
		ret=1;
		goto Exit_Pro;
	}

	get_time(end);

	/*取输入文件大小*/
	sprintf(tmp_file_name,"%s/%s",file_in_dir,in_file_name);
	if(stat(tmp_file_name,&stat_buff)==-1)
	{
		err_log("insert: stat %s fail\n",tmp_file_name);
		ret=1;
		goto Exit_Pro;
	}
	in_file_size=stat_buff.st_size;
	

        /*提交文件*/
	if( commit_file(in_file_name,begin,end,in_file_size,rec_num,db_rec_num) != 0 )
	{
		err_log("insert: commit_file %s fail\n",in_file_name);
		ret=1;
		goto Exit_Pro;
	}
	
	
Exit_Pro:
	if (connection != NULL)
	{
		retcode = ex_con_cleanup(connection, retcode);
	}
	if (Ex_context != NULL)
	{
		retcode = ex_ctx_cleanup(Ex_context, retcode);
	}

	if(fp_i!=NULL) fclose(fp_i);
	
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
 *  zero_buff_2(char ***buff)
 */

void zero_buff_2(char *** buff)
{
	int i,j;

	for(i=0;i<200;i++)
	{
		for(j=0;j<5;j++)
			memset(buff[i][j],0,sizeof(buff[i][j]));
	}
}

/*
 *  get_idb_num()
 *                return 0 success, 1 fail
 *
 */
int get_idb_num(int parallel_number, int *o_idb_num)
{
	int              ret;

        CS_CONTEXT      *Ex_context    = NULL;
	CS_CONNECTION	*connection    = NULL;
	CS_RETCODE	 retcode;

        char          ***db_rec_buff_2 = NULL;
        int              db_rec_num_2;
        int              i,j,k;
	char             sql[256];
	
	ret = 0;


	*o_idb_num=0;
	
	retcode = ex_init(&Ex_context);
	if (retcode != CS_SUCCEED)
	{
		err_log("get_idb_num: ex_init fail\n");
		ret=1;
		goto Exit_Pro;
	}

	retcode = ex_connect(Ex_context, &connection, NULL, db_user, db_password, db_server);
	if (retcode != CS_SUCCEED)
	{
		err_log("get_idb_num: ex_connect fail\n");
		ret=1;
		goto Exit_Pro;
	}

	retcode = ex_execute_cmd(connection,"set chained off");
	if (retcode != CS_SUCCEED)
	{
		err_log("get_idb_num: db set chained off fail\n");
		ret=1;
		goto Exit_Pro;
	}

	/*----------------------------------------------------*/
	db_rec_buff_2 = (char ***)malloc(200 * sizeof(char **));
	if(db_rec_buff_2 == NULL)
	{
		err_log("get_idb_num: malloc fail\n");
		ret=1;
		goto Exit_Pro;
	}
	for(i=0;i<200;i++)
		db_rec_buff_2[i] = NULL;
		
	for(i=0;i<200;i++)
	{
		db_rec_buff_2[i] = (char **)malloc(5 * sizeof(char *));
		if(db_rec_buff_2[i] == NULL)
		{
			err_log("get_idb_num: malloc fail\n");
			ret=1;
			goto Exit_Pro;
		}
		for(j=0;j<5;j++)
			db_rec_buff_2[i][j]=NULL;
			
		for(j=0;j<5;j++)
		{
			db_rec_buff_2[i][j] = (char *)malloc( (MAX_CHAR_BUF+1) * sizeof(char) );
			if(db_rec_buff_2[i][j] == NULL)
			{
				err_log("get_idb_num: malloc fail\n");
				ret=1;
				goto Exit_Pro;
			}
		}
	}
	/*----------------------------------------------------*/
	zero_buff_2(db_rec_buff_2);
	retcode = ex_execute_query(connection,"select data_no, work_status, data_table \
	            from work_billdatastatus where work_status='F' order by data_no",  \
	            db_rec_buff_2, 3, 1, 200, &db_rec_num_2);
		
	if(retcode != CS_SUCCEED)
	{
		err_log("get_idb_num: ex_execute_query fail\n");
		ret=1;
		goto Exit_Pro;
	}

	for(j=0;j<db_rec_num_2;j++)
	{
		//for(k=0;k<3;k++)
		//	printf("%s,",db_rec_buff_2[j][k]);
		//printf("\n");
		
		if( curr_process_number == parallel_number )
		{
			if( 0 == atoi(db_rec_buff_2[j][0])%parallel_number )
			{
				*o_idb_num = atoi(db_rec_buff_2[j][0]);
				break;
			}
		}
		else
		{
			if( curr_process_number == atoi(db_rec_buff_2[j][0])%parallel_number )
			{
				*o_idb_num = atoi(db_rec_buff_2[j][0]);
				break;
			}
		}
	}

	if( *o_idb_num != 0 )
	{
		sprintf(sql,"truncate table idb_bill_%02d",*o_idb_num);	
		retcode = ex_execute_cmd(connection,sql);
		if (retcode != CS_SUCCEED)
		{
			err_log("get_idb_num: db set chained off fail\n");
			ret=1;
			goto Exit_Pro;
		}
	}
	
Exit_Pro:
	if (connection != NULL)
	{
		retcode = ex_con_cleanup(connection, retcode);
	}

	if (Ex_context != NULL)
	{
		retcode = ex_ctx_cleanup(Ex_context, retcode);
	}

	/*----------------------------------------------------*/
	if(db_rec_buff_2 != NULL)
	{
		for(i=0;i<200;i++)
		{
			if(db_rec_buff_2[i] != NULL)
			{
				for(j=0;j<5;j++)
				{
					if(db_rec_buff_2[i][j] != NULL)
						free(db_rec_buff_2[i][j]);
				}
				free(db_rec_buff_2[i]);
			}
		}
		free(db_rec_buff_2);
	}
	/*----------------------------------------------------*/

	return ret;
}

/*
 *  process_insert()
 *
 *
 */
void process_insert(int current_number,int parallel_number)
{
	char    file_name[256];
        int     idb_num;

	/*设置全局子进程编号*/
	curr_process_number=current_number;

	memset(file_name,0,sizeof(file_name));
	while(get_in_file_name(parallel_number,file_in_dir,NULL,".pre.m",file_name)==PARSE_MATCH)
	{
		/*判断是否有空闲接口表*/
	        if( get_idb_num(parallel_number,&idb_num)!=0 )
        	{
        		err_log("process_insert: get_idb_num fail\n");
        		break;
        	}
		if( idb_num == 0 )
		{
			err_log("process_insert: doesnot free idb_bill_xx table\n");
			break;
		}

		/*文件预处理*/
		if(insert(file_name,idb_num)!=0)
		{
			err_log("process_insert: insert fail\n");
			break;
		}
		
		memset(file_name,0,sizeof(file_name));
        }
	
	return;
}


