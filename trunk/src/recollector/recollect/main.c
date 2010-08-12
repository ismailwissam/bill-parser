/****************************************************************************/
/* main.c                                                                   */
/*                                                                          */ 
/* 话单补采程序                                                             */
/*                                                                          */
/* create by wangxiaohui at 2010.7.19                                       */
/****************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <dirent.h>
#include <signal.h>
#include "common.h"
#include "semctl.h"
#include "collect.h"
#include "pretreat.h"
#include "insert.h"

/* global variant definition */
char*               progname = NULL;

/* funtion definition */
static void         usage(int status);

/*
 *  Display the syntax for starting this program.
 */
static void usage(int status)
{
    FILE *output = status?stderr:stdout;
    
    fprintf(output, "Usage: %s [-c collect_path] [-p pretreat_path] [-r run_path] \
                               [-x collect_process_num] [-y pretreat_process_num] [-z insert_process_num] \
                               [-s db_server] [-u db_user] [-w db_password] [-d]\n", progname);
    fprintf(output, "\nOptions:\n");
    fprintf(output, "        -c collect path, default is ./data/collect\n");
    fprintf(output, "        -p pretreat path, default is ./data/pretreat\n");
    fprintf(output, "        -r run path, default is ./\n");
    fprintf(output, "        -x collect child process number.\n");
    fprintf(output, "        -y pretreat child process number.\n");
    fprintf(output, "        -z insert child process number.\n");
    fprintf(output, "        -s db server.\n");
    fprintf(output, "        -u db user.\n");
    fprintf(output, "        -w db password.\n");
    fprintf(output, "        -d debug flag\n");

	exit(status);
}

int main(int argc, char * argv[])
{
    int        ret = 0;
	int        argval;
	struct     stat stat_buff;
    pid_t      collect_pid, pretreat_pid, insert_pid, wait_pid;
    BOOL       collect_task_finish, pretreat_task_finish, insert_task_finish;

    printf("begin the main process...\n");

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
    while ((argval = getopt(argc, argv, "c:p:r:x:y:z:s:u:w:d")) != EOF) 
    {
        switch(argval) {
            case 'c':
                collect_dir = strdup(optarg);
                break;	
            case 'p':
                pretreat_dir = strdup(optarg);
                break;	
            case 'r':
                run_dir = strdup(optarg);
                break;	
            case 'x':
                collect_parallel_num = atoi(optarg);
                break;
            case 'y':
                pretreat_parallel_num = atoi(optarg);
                break;
            case 'z':
                insert_parallel_num = atoi(optarg);
                break;
            case 's':
                db_server = strdup(optarg);
                break;
            case 'u':
                db_user = strdup(optarg);
                break;
            case 'w':
                db_password = strdup(optarg);
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
    if(collect_dir == NULL)
    {
        collect_dir = (char *)malloc(MAX_FILENAME);
        if(collect_dir == NULL)
        {
            err_log("main: malloc fail\n");
            ret = 1;
            goto Exit_Pro;
        }
        strcpy(collect_dir, DEFAULT_COLLECT_DIR);
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

	/* 切换到运行目录 */
	if(chdir(run_dir) == -1)
	{
		err_log("main: chdir to %s fail\n", run_dir);
        ret = 1;
        goto Exit_Pro;
	}

	/* 检查临时工作目录 */
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

	/* 检查配置目录 */
	if(stat(CONF_DIR, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", CONF_DIR);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", CONF_DIR);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* 检查模板目录 */
	if(stat(TEMPLATE_DIR, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", TEMPLATE_DIR);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", TEMPLATE_DIR);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* 检查模块目录 */
	if(stat(MODULE_DIR, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", MODULE_DIR);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", MODULE_DIR);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* 检查采集目录 */
	if(stat(collect_dir, &stat_buff) == -1)
	{
		err_log("main: stat %s fail\n", collect_dir);
        ret = 1;
        goto Exit_Pro;
	}
	else
	{
        if(!S_ISDIR(stat_buff.st_mode))	
        {
            err_log("main: %s isn't a dir\n", collect_dir);
            ret = 1;
            goto Exit_Pro;
        }
	}

	/* 检查解析目录 */
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

	/* 清理work目录下文件 */
    if(clear_dir_file(WORK_DIR, NULL, NULL) != 0)
    {
		err_log("main: clear %s fail\n", WORK_DIR);
        ret = 1;
        goto Exit_Pro;
    }
   	
    /* 初始化信号量对象 */
    if(sem_init() != 0)
    {
		err_log("main: sem_init fail\n");
        ret = 1;
        goto Exit_Pro;
    }

	/* 验证采集配置 */
	if(verify_collect_conf(&collect_point_num) != 0)
	{
		err_log("main: verify_collect_conf fail\n");
        ret = 1;
        goto Exit_Pro;
	}

	if(collect_point_num <= 0 )
	{
		err_log("main: collect point num is incorrect: %d\n", collect_point_num);
        ret = 1;
        goto Exit_Pro;
	}

    printf("begin the collect task process...\n");

    /* 启动采集任务 */
    if((collect_pid = start_collect_task()) < 0)
    {
        err_log("main: start collect task fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    printf("begin the pretreat task process...\n");

    /* 启动解析任务 */
    if((pretreat_pid = start_pretreat_task()) < 0)
    {
        err_log("main: start pretreat task fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    printf("begin the insert task process...\n");

    /* 启动入库任务 */
    if((insert_pid = start_insert_task()) < 0)
    {
        err_log("main: start insert task fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    /* 等待三个任务进程退出 */
    collect_task_finish = FALSE;
    pretreat_task_finish = FALSE;
    insert_task_finish = FALSE;
    while(1)
    {
        /* 等待采集任务退出 */
        if(collect_pid > 0)
        {
            wait_pid = waitpid(collect_pid, NULL, WNOHANG);
            if(wait_pid > 0)
            {
                collect_pid = 0;
                collect_task_finish = TRUE;

                printf("finish the collect task process successfully!\n");

                /* 如果采集任务完成，则向解析任务进程发送一个通知信号 */
                if(pretreat_pid > 0)
                {
                    kill(pretreat_pid, SIGUSR1);
                }
            }
            if(wait_pid < 0)
            {
                err_log("main: waitpid fail\n");
                ret = 1;
                goto Exit_Pro;
            }
        }

        /* 等待解析任务进程退出 */
        if(pretreat_pid > 0)
        {
            wait_pid = waitpid(pretreat_pid, NULL, WNOHANG);
            if(wait_pid > 0)
            {
                pretreat_pid = 0;
                pretreat_task_finish = TRUE;

                printf("finish the pretreat task process successfully!\n");

                /* 如果解析任务完成，则向入库任务进程发送一个通知信号 */
                if(insert_pid > 0)
                {
                    kill(insert_pid, SIGUSR1);
                }
            }
            if(wait_pid < 0)
            {
                err_log("main: waitpid fail\n");
                ret = 1;
                goto Exit_Pro;
            }
        }

        /* 等待入库任务进程退出 */
        if(insert_pid > 0)
        {
            wait_pid = waitpid(insert_pid, NULL, WNOHANG);
            if(wait_pid > 0)
            {
                insert_pid = 0;
                insert_task_finish = TRUE;

                printf("finish the insert task process successfully!\n");
            }
            if(wait_pid < 0)
            {
                err_log("main: waitpid fail\n");
                ret = 1;
                goto Exit_Pro;
            }
        }

        /* 如果三个任务进程都已经完成，则退出主进程 */
        if(collect_task_finish && pretreat_task_finish && insert_task_finish)
        {
            break;
        }

        sleep(60);
    }
    
    /* 处理采集配置 
     * 把当前处理过的采集配置文件添加时间后缀进行备份 */
    if(archive_collect_conf() != 0)
    {
        err_log("archive the collect conf fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    printf("finish the main process successfully!\n");

Exit_Pro:
    if(collect_dir != NULL)
    {
        free(collect_dir);
    }
    if(pretreat_dir != NULL)
    {
        free(pretreat_dir);
    }
    if(run_dir != NULL)
    {
        free(run_dir);
    }
    if(db_server != NULL)
    {
        free(db_server);
    }
    if(db_user != NULL)
    {
        free(db_user);
    }
    if(db_password != NULL)
    {
        free(db_password);
    }
	return ret;
}

