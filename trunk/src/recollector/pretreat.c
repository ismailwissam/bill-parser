/****************************************************************************/
/* pretreat.c                                                               */
/*                                                                          */ 
/* ����Ԥ�������                                                           */
/*                                                                          */
/* create by wangxiaohui at 2010.5.13                                       */
/*                                                                          */
/* modify by wangxioahui at 2010.5.15                                       */
/*        �����޸���Ҫ�ǵ����ӿڲ������ӿڵĵڶ�������ԭ�������������      */
/*        �ṩ�������CSV�ļ��������ڱ��Ϊ�����������������ʱ�ṩ����     */
/*        CSV�ļ���·�������ʱ���������ɵ�CSV�ļ���׷����·�����淵�أ�    */
/*        װ���ʽΪ��CSV�ļ�����·��:CSV�ļ���1;CSV�ļ���2[;...]           */
/*                                                                          */
/* modify by wangxiaohui at 2010.5.19                                       */
/*        �����޸���Ҫ�ǵ�������̵ĵ����㷨��������Բɼ��������̣�����  */
/*         ÿ�����̿��Դ������⻰���ļ����������ļ�ǰ�������@��־��������*/
/*        ����                                                            */
/*                                                                          */
/* modify by wangxiaohui at 2010.5.23                                       */
/*        ����������ļ�������ʽ����֤�� valid_in_file_name                 */
/*        ����Ļ����ļ�������ʽ�����ǣ�                                    */
/*               �ɼ�����_����_�˾�_��������ʱ��(YYYYMMDDhhmmss)_ԭ�ļ��� */
/*                                                                          */
/* modify by ���� at 2010.6.11                                              */
/*        �µ�commitfile:��ȡ����,�ж��ļ��д��ڷ�,���ļ���,link��ȥ.       */
/*        ��ʱ��վɵ��ļ���(�ػ����̴���):									*/
/*        �賿2��(��ʱ2Сʱ�ǲ������)��ʱ��ɨ�������ļ��к�׺,             */
/*        ����Ŀ��ļ���ͨͨɾ��,											*/
/*        ��Ϊ�յ��ӵ�error�ļ��в���¼��־.                                */
/*                                                                          */
/* modify by wangxiaohui at 2010.6.22                                       */
/*        �Ѷ�data/pretreatĿ¼���м����ݵ���������ȡ����������ר�ŵ����� */
/*        ����clean_data_pretreat�������������                           */
/*                                                                          */
/* modify by wangxiaohui at 2010.7.19                                       */
/*        ���ɳ���汾��                                                    */
/*        �ع�����ṹ��                                                    */
/*                                                                          */
/* modify by wangxiaohui at 2010.8                                          */
/*        ����csv���ݹ��ܣ��������Ŀ¼csv�ļ��Ѿ����ڣ��򸲸Ǿɵ��ļ���    */
/****************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <link.h>
#include <signal.h>
#include "common.h"
#include "semctl.h"
#include "pretreat.h"

/* micro definition */
#define      MAX_MODULE             128
#define      MAX_OUTFILE_NUM        10
#define      MAX_FILE_ELEM_NUM      10
#define      MAX_FILE_ELEM          64
#define      PREFIX_RUN_LOG_FILE    "./log/pretreat_run"
#define      DELAY_TIMES            180

/* type definition */
typedef int funHandler(char *, char *, int *);  //in_file_name, out_file_name, rec_num

typedef struct {
        int            isload;         //       0 no     , 1 yes
        funHandler   * module_fun;     //return 0 success, 1 fail
        void         * handler_dlopen;
} t_module;

/* global variant definition */
t_module     module[MAX_MODULE];
BOOL         is_collect_finished = FALSE;

/* funtion definition */
static int          pretreat_task(void);
static int          commit_file(const char * in_file_name, const char * out_file_name, const char * begin_time, const char * end_time, long in_file_size, long rec_num);
static int          pretreat(const char * in_file_name);
static parse_code_e get_in_file_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_file_name);
static void         process_pretreat(void);
static int          load_module(void);
static int          get_relative_module_no(const char * in_file_name, int * module_no);
static int          valid_in_file_name(const char * in_file_name);
static int          get_commit_filename(const char * out_file_name, char * ret_commit_filename);
static int          get_backup_filename(const char * out_file_name, char * ret_backup_filename);
static int          get_ne_name(const char * original_file_name, char * ret_ne_name);
static void         sigusr1_handler(int signum);

int start_pretreat_task(void)
{
    int pid;

    pid = fork();
    if(pid == 0)
    {
        if(pretreat_task() != 0)
        {
            err_log("start_pretreat_task: pretreat_task fail\n");
            exit(1);
        }
        exit(0);
    }
    return pid;
}

static int pretreat_task(void)
{
    int        ret = 0;
	int        i;
	pid_t      childpid;
	pid_t      r_waitpid;
    t_child_process_status child_process_status[MAX_CHILD_PROCESS];
    BOOL       is_all_child_process_idle;
    int        delay_times = 0;

    /* ע��SIGUSR1�ź��� */
    if(signal(SIGUSR1, &sigusr1_handler) == SIG_ERR)
    {
		err_log("main: signal SIGUSR1 fail\n");
        ret = 1;
        goto Exit_Pro;
    }

	/* װ��Ԥ����ģ�� */
    if(load_module() != 0)
    {
		err_log("main: load module fail\n");
        ret = 1;
        goto Exit_Pro;
    }
		
	/* У���ӽ���������Ŀ */
	if(pretreat_parallel_num <= 0)
    {
        pretreat_parallel_num = 1;
    }
	if(pretreat_parallel_num > MAX_CHILD_PROCESS) 
    {
        pretreat_parallel_num = MAX_CHILD_PROCESS;
    }
    if(pretreat_parallel_num > collect_point_num)
    {
        pretreat_parallel_num = collect_point_num;
    }
        
	/* ��ʼ�� �ӽ��̽ṹ���� */
    for(i = 0; i<MAX_CHILD_PROCESS; i++)
    {
        memset(&child_process_status[i], 0, sizeof(t_child_process_status));
    }
   
	/* ѭ����������̽��н������� */
	while(1)
	{
		/* �����ӽ��� */
		for(i = 0; i<pretreat_parallel_num; i++)
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
                    child_process_status[i].sleep_time = PROCESS_SLEEP_TIME;
                }
                else if(childpid == 0)    /* child process */
                {
                    process_pretreat();
                    exit(0);
                }
            }
		}
	
		/* �����ӽ��� */
		for(i = 0; i<pretreat_parallel_num; i++)
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
		
        /* �ж��Ƿ����н������̶��Ѿ����� */
        is_all_child_process_idle = TRUE;
		for(i = 0; i<pretreat_parallel_num; i++)
		{
			if(child_process_status[i].pid > 0)
			{
                is_all_child_process_idle = FALSE;
                delay_times = 0;   //reset to 0
                break;
			}
		}

        /* ����ɼ������Ѿ��������������н������̶��Ѿ����У��˳��������� */
        if(is_collect_finished && is_all_child_process_idle)
        {
            delay_times++;
        }

        if(delay_times == DELAY_TIMES)
        {
            break;
        }

		sleep(1);
		
		/* ÿ��ݼ�sleep_time */
		for(i = 0; i<pretreat_parallel_num; i++)
		{
            if(child_process_status[i].pid == 0 && child_process_status[i].sleep_time > 0)
            {
                child_process_status[i].sleep_time--;
            }
		}
	}
    	
Exit_Pro:
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
	char  commit_file_name[MAX_FILENAME];
	char  backup_file_name[MAX_FILENAME];
    char  work_dir[MAX_FILENAME];
    char  out_files[MAX_OUTFILE_NUM][MAX_FILENAME];
    const char  *ptr_front = NULL, *ptr_back = NULL;
    char  ne_name[MAX_FILE_ELEM];
    int   count, i;

    /* ��ʼ�� */
    memset(work_dir, 0, sizeof(work_dir));
    memset(tmp_file_name, 0, sizeof(tmp_file_name));
    memset(commit_file_name, 0, sizeof(commit_file_name));
    memset(backup_file_name, 0, sizeof(backup_file_name));
    memset(out_files, 0, sizeof(out_files));

    /* ��ȡ����Ŀ¼ */
    ptr_front = out_file_name;
    ptr_back = strchr(ptr_front, ':');
    if(ptr_back == NULL)
    {
        err_log("commit_file: out file string %s incorrect\n", out_file_name);
        ret = 1;
        goto Exit_Pro;
    }
    strncpy(work_dir, ptr_front, ptr_back - ptr_front);

    /* ��������CSV�ļ��� */
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

    /* ѭ������ÿһ�������CSV�ļ� */
    for(i = 0; i < count; ++i)
    {
        /* csv�ļ���ʱ����Ŀ¼ */
        sprintf(tmp_file_name, "%s/%s", work_dir, out_files[i]);

		/* ��ȡ�ύĿ¼ */
		if (0 != get_commit_filename(out_files[i], commit_file_name))
		{
			err_log("commit_file: get_commit_filename fail\n");
			ret = 1;
			goto Exit_Pro;
		}

        /* ���Ŀ���ļ��Ѿ����ڣ���ɾ�� */
        if(access(commit_file_name, F_OK) == 0)
        {
            if(unlink(commit_file_name) != 0)
            {
                err_log("commit_file: unlink old file %s fail\n", commit_file_name);
                ret = 1;
                goto Exit_Pro;
            }
        }

        /* ���ƹ���Ŀ¼�ļ����ύĿ¼ */
        if(link(tmp_file_name, commit_file_name) != 0)
        {
            err_log("commit_file: link %s to %s fail\n", tmp_file_name, commit_file_name);
            ret = 1;
            goto Exit_Pro;
        }
        
        /* ��������˱���Ŀ¼�����csv�ļ����б��� */
        if(csv_backup_dir != NULL)
        {
            /* ��ȡ����Ŀ¼ */
            if (0 != get_backup_filename(out_files[i], backup_file_name))
            {
                err_log("commit_file: get_backup_filename fail\n");
                ret = 1;
                goto Exit_Pro;
            }

            /* ���Ŀ���ļ��Ѿ����ڣ���ɾ�� */
            if(access(backup_file_name, F_OK) == 0)
            {
                if(unlink(backup_file_name) != 0)
                {
                    err_log("commit_file: unlink old file %s fail\n", commit_file_name);
                    ret = 1;
                    goto Exit_Pro;
                }
            }

            /* ���ƹ���Ŀ¼�ļ�������Ŀ¼ */
            if(link(tmp_file_name, backup_file_name) != 0)
            {
                err_log("commit_file: link %s to %s fail\n", tmp_file_name, backup_file_name);
                ret = 1;
                goto Exit_Pro;
            }
        }

        /* ɾ������Ŀ¼����Ӧ����ʱ�ļ� */
		if ( unlink(tmp_file_name) != 0 )
		{
			err_log("commit_file: unlink file %s fail\n", tmp_file_name);
			ret = 1;
			goto Exit_Pro;
		}
    }

    /* ɾ�������ļ�in_file_name */
    sprintf(tmp_file_name, "%s/%s", collect_dir, in_file_name);
    if ( unlink(tmp_file_name) != 0 )
    {
        err_log("commit_file: unlink file %s fail\n", tmp_file_name);
        ret = 1;
        goto Exit_Pro;
    }

    /* ȡ����Ԫ���� */
    memset(ne_name, 0, sizeof(ne_name));
    if(get_ne_name(in_file_name, ne_name) != 0)
    {
        err_log("commit_file: get ne name fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    /* ��¼������Ϣ��pretreat_run.YYYYMMDD�ļ� */
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

    /* ��֤�����ļ��Ƿ���������淶 */
    if(valid_in_file_name(in_file_name) != 0)
    {
		err_log("pretreat: bill file name format incorrect\n", in_file_name);
		ret = 1;
		goto Exit_Pro;
    }

	/* ȡ�ļ���Ӧ����ģ��� */
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
	
    /* �ж϶�Ӧ�Ľ���ģ���Ƿ���� */
	if( module[num_module-1].isload != 1 )
	{
		err_log("pretreat: file %s, module %d not load\n", in_file_name, num_module);
		ret = 1;
		goto Exit_Pro;
	}

	
    /* ���ý���ģ��ĵ��ò����������ļ�,����ļ�·��,���ػ�����Ŀ */
	sprintf(tmp_in_file_name, "%s/%s", collect_dir, in_file_name);
	strcpy(tmp_out_file_name, WORK_DIR);
	rec_num = 0;

	get_time(begin_time);

	/* ���ö�Ӧģ��Ĵ����� */
	if( module[num_module-1].module_fun(tmp_in_file_name, tmp_out_file_name, &rec_num) != 0 )
	{
		err_log("pretreat: file %s, module %d module_fun fail\n", in_file_name, num_module);
		ret = 1;
		goto Exit_Pro;
	}

	get_time(end_time);

	/* ȡ�����ļ���С */
	if(stat(tmp_in_file_name, &stat_buff) == -1)
	{
		err_log("pretreat: stat %s fail\n", tmp_in_file_name);
		ret = 1;
		goto Exit_Pro;
	}
	in_file_size = stat_buff.st_size;

    /* �ύ�ļ� */
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
     * ��Ϊ��Ŀ¼��ֻ�ǽ����˲�ѯ���������и����ļ����ƵĲ�����
     * ����Ϊ�˷�ֹ�������ͬʱ��ͬһ���ļ����о�������������
     * �˶���̼以�� 
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
		/* ǰ��׺ƥ���� */
		if( pre_suf_check(pdirent->d_name, prefix, suffix) != PARSE_MATCH )
			continue;

        ptr = pdirent->d_name;
        ptr++;

		/* ��ȥǰ�������־@��Ҫ���ļ���ǰ6���ֽ������� */
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

        /* �ļ���� */
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
static void process_pretreat(void)
{
	char    file_name[MAX_FILENAME];

	memset(file_name, 0, sizeof(file_name));
	while(get_in_file_name(collect_dir, "@", NULL, file_name) == PARSE_MATCH)
	{
		/* �ļ�Ԥ���� */
		if(pretreat(file_name) != 0)
		{
			err_log("process_pretreat: pretreat fail\n");
			break;
		}
		
		memset(file_name, 0, sizeof(file_name));
    }
	return;
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

	sprintf(file_name, "%s/%s", CONF_DIR, "module.conf");

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
            
            /* װ�ض�̬�� */
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
	
	sprintf(tmp_file_name, "%s/pretreat.conf", CONF_DIR);
	fp = fopen(tmp_file_name, "r");
	if(fp == NULL)
	{
		err_log("get_relative_module_no: fopen %s fail\n", tmp_file_name);
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
                err_log("get_relative_module_no: pretreat.conf line %d incorrect\n", r);
                ret = 1;
                goto Exit_Pro;
            }
            
            len = strlen(content[0]);
            if(len != 8 || content[0][0] != '<' || content[0][len-1] != '>')
            {
                err_log("get_relative_module_no: pretreat.conf line %d incorrect\n", r);
                ret = 1;
                goto Exit_Pro;
            }
                
            content[0][len-1] = '\0';
            if(atoi(&content[0][1]) != r)
            {
                err_log("get_relative_module_no: pretreat.conf line %d incorrect\n", r);
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

    /* ��ʼ���������� */
    for(i = 0; i < MAX_FILE_ELEM_NUM; ++i)
    {
        memset(elems[i], 0, sizeof(elems[i]));
    }

    /* �ֽ������ļ������Ԫ�� */
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
 
    /* �����ļ���'_'�����Ԫ��������5�� */
    if(count < 5)
    {
		err_log("valid_in_file_name: in file name format incorrect\n");
        return 1;
    }

    /* 
     * ��֤�ֽ������ǰ�ĸ�����Ԫ�� 
     * elems[0]: �ɼ����� 
     * elems[1]: ���̱��(hw,nsn)
     * elems[2]: ��Ԫ����
     * elems[3]: ��������ʱ��(YYYYMMDDhhmmss)
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

    /* ��һ��Ԫ��elems[0]�ǲɼ����ţ�6λ���ֱ��
     * ��Ϊ��֮ǰ��λҪ�����ļ�ʱ�Ѿ�������֤������
     * ���ﲻ��Ҫ����֤
     */

    /* �ڶ���Ԫ��elems[1]�ǳ�����д��Ŀǰֻ������hw��nsn */
    if((strcmp(elems[1], "hw") != 0) && (strcmp(elems[1], "nsn") != 0))
    {
		err_log("valid_in_file_name: in file name format incorrect\n");
        return 1;
    }
    
    /* 
     * ������Ԫ��elems[2]����Ԫ���ƣ����е���Ԫ����Ҳ��ȷ���ģ���Ϊ��
     * ִ��Ч�ʣ�Ŀǰ�ݲ���֤��Ԫ���� 
     */

    /* ���ĸ�Ԫ��elems[3]�����ڸ�ʽ��YYYYMMDDhhsssmm */
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

static int get_commit_filename(const char * out_file_name, char * ret_commit_filename)
{ 
    char            elems[MAX_FILE_ELEM_NUM][MAX_FILE_ELEM];
    const char      *ptr_front = NULL, *ptr_back = NULL;
    char            table_date[MAX_DATE];
	char			path_name[MAX_FILENAME];
    int             count, len, i;

    /* ��ʼ���������� */
    for(i = 0; i < MAX_FILE_ELEM_NUM; ++i)
    {
        memset(elems[i], 0, sizeof(elems[i]));
    }

    /* �ֽ������ļ��ı���Ԫ�� */
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
 
    /* CSV�ļ���'_'�����Ԫ��������6�� */
    if(count < 6)
    {
		err_log("get_commit_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    }
 
    /* 
     * ��֤�ֽ������ǰ���Ԫ�� 
     * elems[0]: �ɼ����� 
     * elems[1]: ���̱��(hw,nsn)
     * elems[2]: ��Ԫ����
     * elems[3]: ��������
     * elems[4]: ��������ʱ��(YYYYMMDDhhmmss)
     * */
    if(strlen(elems[0]) == 0 
       || strlen(elems[1]) == 0
       || strlen(elems[2]) == 0 
       || strlen(elems[3]) == 0
       || strlen(elems[4]) == 0
       || strlen(elems[5]) == 0)
    {
		err_log("get_commit_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    } 

    /* ��һ��Ԫ��elems[0]�ǲɼ����ţ�6λ���ֱ��     */

    /* �ڶ���Ԫ��elems[1]�ǳ�����д��Ŀǰֻ������hw��nsn */
    if((strcmp(elems[1], "hw") != 0) && (strcmp(elems[1], "nsn") != 0))
    {
		err_log("get_commit_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    }
    
    /* 
     * ������Ԫ��elems[2]����Ԫ���ƣ����е���Ԫ����Ҳ��ȷ���ģ���Ϊ��
     * ִ��Ч�ʣ�Ŀǰ�ݲ���֤��Ԫ���� 
     */

    /* 
     * ���ĸ�Ԫ��elems[3]�ǻ������ͣ�Ҫ�����Ļ�������Ҳ��ȷ���ģ���Ϊ��
     * ִ��Ч�ʣ�Ŀǰ�ݲ���֤�������� 
     */

    /* �����Ԫ��elems[4]�����ڸ�ʽ��YYYYMMDDhhsssmm */
    len = strlen(elems[4]);
    if(len != 14)
    {
		err_log("get_commit_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    }

    for(i = 0; i < len; ++i)
    {
        if(!isdigit(elems[4][i]))
        {
            err_log("get_commit_filename: file name format incorrect: %s\n", out_file_name);
            return 1;
        }
    }

    /* ����Ҫ��ʱ����ȡ���� 
     * ����elems[4]�д�ŵ�ʱ���ʽ�ǣ�YYYYMMDDhhmmss
     * �����е�MMDD��ȡ������Ϊ������һ����
     */
    memset(table_date, 0, sizeof(table_date));
    strncpy(table_date, &(elems[4][4]), 4);

    /* ������Ҫ���ļ������� */
    if(strcmp(elems[1], "hw") == 0)
    {
        if(strcmp(elems[3], "bi") == 0)
        {
            /* ������ op_bill_����_�˾�_ʱ��(MMDD) */
            sprintf(path_name, "%s/op_bill_%s_%s_%s", pretreat_dir, elems[1], elems[2], table_date);
        }
        else
        {
            /* ������ op_bill_����_��������_�˾�_ʱ��(MMDD) */
            sprintf(path_name, "%s/op_bill_%s_%s_%s_%s", pretreat_dir, elems[1], elems[3], elems[2], table_date);
        }
    }
    else if(strcmp(elems[1], "nsn") == 0)
    {
        /* ������ op_bill_����_��������_�˾�_ʱ��(MMDD) */
        sprintf(path_name, "%s/op_bill_%s_%s_%s_%s", pretreat_dir, elems[1], elems[3], elems[2], table_date);
    }
    else
    {
        err_log("get_commit_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    }

	//ȷ��Ŀ¼�Ƿ����,�������򴴽�
	P();
	if(access(path_name,F_OK) == -1)
	{
		if(mkdir(path_name,0755)!=0)
		{
			err_log("get_commit_filename: mkdir incorrect: %s\n", path_name);
			V();
			return 1;
		}
	}
	V();

    /* �����ύ�ļ����� */
    sprintf(ret_commit_filename, "%s/%s", path_name, out_file_name);

    return 0;
}

static int get_backup_filename(const char * out_file_name, char * ret_backup_filename)
{ 
    char            elems[MAX_FILE_ELEM_NUM][MAX_FILE_ELEM];
    const char      *ptr_front = NULL, *ptr_back = NULL;
    char            table_date[MAX_DATE];
	char			path_name[MAX_FILENAME];
    char            cmd[MAX_BUFFER];
    int             count, len, i;

    /* ��ʼ���������� */
    for(i = 0; i < MAX_FILE_ELEM_NUM; ++i)
    {
        memset(elems[i], 0, sizeof(elems[i]));
    }

    /* �ֽ������ļ��ı���Ԫ�� */
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
 
    /* CSV�ļ���'_'�����Ԫ��������6�� */
    if(count < 6)
    {
		err_log("get_backup_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    }
 
    /* 
     * ��֤�ֽ������ǰ���Ԫ�� 
     * elems[0]: �ɼ����� 
     * elems[1]: ���̱��(hw,nsn)
     * elems[2]: ��Ԫ����
     * elems[3]: ��������
     * elems[4]: ��������ʱ��(YYYYMMDDhhmmss)
     * */
    if(strlen(elems[0]) == 0 
       || strlen(elems[1]) == 0
       || strlen(elems[2]) == 0 
       || strlen(elems[3]) == 0
       || strlen(elems[4]) == 0
       || strlen(elems[5]) == 0)
    {
		err_log("get_backup_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    } 

    /* ��һ��Ԫ��elems[0]�ǲɼ����ţ�6λ���ֱ��     */

    /* �ڶ���Ԫ��elems[1]�ǳ�����д��Ŀǰֻ������hw��nsn */
    if((strcmp(elems[1], "hw") != 0) && (strcmp(elems[1], "nsn") != 0))
    {
		err_log("get_backup_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    }
    
    /* 
     * ������Ԫ��elems[2]����Ԫ���ƣ����е���Ԫ����Ҳ��ȷ���ģ���Ϊ��
     * ִ��Ч�ʣ�Ŀǰ�ݲ���֤��Ԫ���� 
     */

    /* 
     * ���ĸ�Ԫ��elems[3]�ǻ������ͣ�Ҫ�����Ļ�������Ҳ��ȷ���ģ���Ϊ��
     * ִ��Ч�ʣ�Ŀǰ�ݲ���֤�������� 
     */

    /* �����Ԫ��elems[4]�����ڸ�ʽ��YYYYMMDDhhsssmm */
    len = strlen(elems[4]);
    if(len != 14)
    {
		err_log("get_backup_filename: file name format incorrect: %s\n", out_file_name);
        return 1;
    }

    for(i = 0; i < len; ++i)
    {
        if(!isdigit(elems[4][i]))
        {
            err_log("get_backup_filename: file name format incorrect: %s\n", out_file_name);
            return 1;
        }
    }

    /* ����Ҫ��ʱ����ȡ���� 
     * ����elems[4]�д�ŵ�ʱ���ʽ�ǣ�YYYYMMDDhhmmss
     * �����е�MMDD��ȡ������Ϊ������һ����
     */
    memset(table_date, 0, sizeof(table_date));
    strncpy(table_date, &(elems[4][4]), 4);

    /* ������Ҫ���ļ������� */
    sprintf(path_name, "%s/%s/%s/%s", csv_backup_dir, elems[1], elems[3], table_date);

	/* ȷ��Ŀ¼�Ƿ����,�������򴴽� */
	P();
	if(access(path_name,F_OK) == -1)
	{
        sprintf(cmd, "mkdir -p %s", path_name);
        if(system(cmd) == -1)
        {
			err_log("get_backup_filename: mkdir incorrect: %s\n", path_name);
			V();
			return 1;
        }
   	}
	V();

    /* ���ر����ļ����� */
    sprintf(ret_backup_filename, "%s/%s", path_name, out_file_name);

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

static void sigusr1_handler(int signum)
{
    if(signum == SIGUSR1)
    {
        printf("get SIGUSR1 signal\n");
        is_collect_finished = TRUE;
    }
}

