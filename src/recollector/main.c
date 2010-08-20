/****************************************************************************/
/* main.c                                                                   */
/*                                                                          */ 
/* 话单补采程序                                                             */
/*                                                                          */
/* create by wangxiaohui at 2010.7.19                                       */
/*                                                                          */
/* modify by wangxiaohui at 2010.8.19                                       */
/*        之前版本补采程序的入库操作和正常采集的入库操作产生冲突，造成调用  */
/*        sqlldr失败，所以现在修改为补采程序共享正常采集的入库操作，前面的  */
/*        采集过程和解析过程完全独立，在解析阶段把生成的csv文件放到正常入库 */
/*        目录下，让正常采集的入库程序完成补采数据的入库操作                */
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
                               [-x collect_process_max_num] [-y pretreat_process_max_num] \
                               [-b csv_backup_path] [-f] [-d]\n", progname);
    fprintf(output, "\nOptions:\n");
    fprintf(output, "        -c collect_path: collect path to save the bill files, default is ./data/collect\n");
    fprintf(output, "        -p pretreat_path: pretreat path to save the csv files, default is ./data/pretreat\n");
    fprintf(output, "        -b csv_backup_path: csv backup path, no backup if not set\n");
    fprintf(output, "        -r run_path: program run path, default is ./\n");
    fprintf(output, "        -x collect_process_max_num: collect child process max number, default is 1\n");
    fprintf(output, "        -y pretreat_process_max_num: pretreat child process max number, default is 1\n");
    fprintf(output, "        -f : enable force download flag\n");
    fprintf(output, "        -d : enable debug flag\n");

	exit(status);
}

int main(int argc, char * argv[])
{
    int        ret = 0;
	int        argval;
	struct     stat stat_buff;
    pid_t      collect_pid, pretreat_pid, wait_pid;
    BOOL       collect_task_finish, pretreat_task_finish;

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
    while ((argval = getopt(argc, argv, "c:p:b:r:x:y:fd")) != EOF) 
    {
        switch(argval) {
            case 'c':
                collect_dir = strdup(optarg);
                break;	
            case 'p':
                pretreat_dir = strdup(optarg);
                break;	
            case 'b':
                csv_backup_dir = strdup(optarg);
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
            case 'f':
                force_update = 1;
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

	/* 检查备份目录 */
    if(csv_backup_dir != NULL)
    {
        if(stat(csv_backup_dir, &stat_buff) == -1)
        {
            err_log("main: stat %s fail\n", csv_backup_dir);
            ret = 1;
            goto Exit_Pro;
        }
        else
        {
            if(!S_ISDIR(stat_buff.st_mode))	
            {
                err_log("main: %s isn't a dir\n", csv_backup_dir);
                ret = 1;
                goto Exit_Pro;
            }
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

    /* 等待三个任务进程退出 */
    collect_task_finish = FALSE;
    pretreat_task_finish = FALSE;
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
            }
            if(wait_pid < 0)
            {
                err_log("main: waitpid fail\n");
                ret = 1;
                goto Exit_Pro;
            }
        }

        /* 如果采集和解析进程都已经完成，则退出主进程 */
        if(collect_task_finish && pretreat_task_finish)
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
    if(csv_backup_dir != NULL)
    {
        free(csv_backup_dir);
    }
    if(run_dir != NULL)
    {
        free(run_dir);
    }
	return ret;
}

