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
/*                                                                         */
/* modify by wangxiaohui at 2010.7.19                                      */
/*           ���ɳ���汾��                                                */
/*           �ع����롣                                                    */
/***************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
#include "common.h"
#include "insert.h"

/* micor definition */
#define PREFIX_RUN_LOG_FILE     "./log/insert_run"
#define TABLE_LIST_CONFIG_FILE  "./conf/table.conf"
#define MAX_PARALLEL_TABLE      10
#define MAX_LDR_FILE_NUM        1000

/* global variant defintion */
int          sleep_time             = 5;
int          table_list_num         = 0;
BOOL         is_pretreat_finished   = FALSE;

/* static functions definitions */
static int          insert_task(void);
static int          commit_file(const char * in_file_name,  const char* in_dir_name, const char * begin_time, const char * end_time);
static int          insert(const char * in_dir_name);
static parse_code_e get_in_dir_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_dir_list);
static void         process_insert(int current_number, int parallel_number);
static int          get_ne_name(const char * original_file_name, char * ret_ne_name);
static int          get_table_num(int * ret_table_num);
static int          get_table_name(int index, char * ret_table_name);
static void         sigusr1_handler(int signum);

int start_insert_task(void)
{
    int pid;

    pid = fork();
    if(pid == 0)
    {
        if(insert_task() != 0)
        {
            err_log("start_insert_task: insert_task fail\n");
            exit(1);
        }
        exit(0);
    }
    return pid;
}

static int insert_task(void)
{
    int     ret = 0;
 	int		circle_second = 300;
    t_child_process_status child_process_status[MAX_CHILD_PROCESS];
	pid_t   childpid;
	pid_t   r_waitpid;
	int     i;
    BOOL    is_all_child_process_idle;

    /* ע��SIGUSR1�ź� */
    if(signal(SIGUSR1, &sigusr1_handler) == SIG_ERR)
    {
		err_log("main: signal SIGUSR1 fail\n");
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
	if(insert_parallel_num <= 0)
    {
        insert_parallel_num = 1;
    }
	if(insert_parallel_num > MAX_CHILD_PROCESS) 
    {
        insert_parallel_num = MAX_CHILD_PROCESS;
    }
    if(insert_parallel_num > table_list_num)
    {
        insert_parallel_num = table_list_num;
    }
	
	// ����sleep_time
 	sleep_time = circle_second * insert_parallel_num / table_list_num;
 	if (sleep_time < 5)
 	{
		sleep_time = 5;
 	}
        
	/* ��ʼ���ӽ��̽ṹ���� */
    for(i = 0; i < MAX_CHILD_PROCESS; i++)
    {
        memset(&child_process_status[i], 0, sizeof(t_child_process_status));
    }
   
	/* ѭ����������̽�����⴦�� */
	while(1)
	{
		/*�����ӽ���*/
		for(i = 0; i < insert_parallel_num; i++)
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
                    child_process_status[i].sleep_time = PROCESS_SLEEP_TIME;
                }
                else if(childpid == 0)    /*child process*/
                {
                    process_insert(i + 1, insert_parallel_num);
                    exit(0);
                }
            }
        }
	
		/*�����ӽ���*/
		for(i = 0; i < insert_parallel_num; i++)
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

        /* �ж��Ƿ����������̶��Ѿ������� */
        is_all_child_process_idle = TRUE;
        for(i = 0; i < insert_parallel_num; i++)
		{
			if(child_process_status[i].pid > 0)
			{
                is_all_child_process_idle = FALSE;
                break;
			}
		}
	
        /* ������������Ѿ��������������е������̶��Ѿ�����,�˳�������� */
        if(is_pretreat_finished && is_all_child_process_idle)
        {
            break;
        }

		sleep(1);
		
		/*ÿ��ݼ�sleep_time*/
		for(i = 0; i < insert_parallel_num; i++)
		{
            if(child_process_status[i].pid == 0 && child_process_status[i].sleep_time > 0)
            {
                child_process_status[i].sleep_time--;
            }
		}
	}
	
Exit_Pro:
   	return ret;
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
	fprintf(fp, "%s %s %s %s %s\n", in_file_name, in_dir_name, tmp_file_name, begin_time, end_time);
	fclose(fp);
	fp = NULL;	

    /* ɾ�������ļ�in_file_name */
    sprintf(tmp_file_name, "%s/%s/%s", pretreat_dir, in_dir_name, in_file_name);
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
	char            cmd[MAX_LONG_BUFFER];
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
    int             read_len;

    /* �������Ŀ¼����Ч�� */
	if( NULL == in_dir_name )
	{
		err_log("insert: dir_name is NULL\n");
		ret = 1;
		goto Exit_Pro;	
	}

	/* ��Ŀ¼ */
	sprintf(tmp,"%s/%s",pretreat_dir,in_dir_name);
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
        /* �����ʱ������������Ѿ���������ô���
         * ��ʱ���csv�ļ����ļ��о�û��Ҫ������ɾ��֮ */
        if(is_pretreat_finished)
        {
            if(rmdir(tmp) != 0)
            {
                err_log("insert: rmdir %s fail\n", tmp);
                ret = 1;
                goto Exit_Pro;
            }
        }
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
	sprintf(control_template, "%s/%s.ctl.template", TEMPLATE_DIR, table_prefix);

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
			read_len = fread(tmp,1,9,ctl_template);
            if(read_len != 9)
            {
                err_log("insert: ctl_template file error: %s\n", control_template);
                ret = 1;
                goto Exit_Pro;
            }
			tmp[9] = '\0';
			if (0 == strcmp("DATA_FILE", tmp))
			{
				for(i = 0; (strlen(ldr_file_list[i]) > 0) && (i < MAX_LDR_FILE_NUM) ; ++i)
				{
					sprintf(infile_one_line,"infile \"%s/%s/%s\"\n",pretreat_dir,in_dir_name,ldr_file_list[i]);
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
 *  process_insert()
 */
static void process_insert(int current_number, int parallel_number)
{
	char    dir_list[MAX_PARALLEL_TABLE][MAX_FILENAME];
	char	dir_list_string[MAX_LONG_BUFFER];
	char*	ptr_hea = NULL;
	char*	ptr_tra = NULL;
    char    table_name[MAX_TABLENAME];
	char    temp_table_pre[MAX_TABLENAME];
    int     skip_num, begin_no, end_no;
    int     i, j, count;

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
            break;
        }
		sprintf(temp_table_pre,"%s_",table_name);
		// ��ȡĿ¼�Ĵ������ļ����б�, ����dir_listĿ¼�б�
        if(get_in_dir_name(pretreat_dir, temp_table_pre, NULL, dir_list_string) == PARSE_MATCH)
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
 					err_log("process_insert: insert fail: %s\n", dir_list[j]);
 					continue;
 				}
            }
        }
        else
        {
            continue;
        }

        /* �ȴ�һ����ʱ���� */
        sleep(sleep_time);
    }

    return;
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

static void sigusr1_handler(int signum)
{
    if(signum == SIGUSR1)
    {
        printf("get SIGUSR1 signal\n");
        is_pretreat_finished = TRUE;
    }
}

