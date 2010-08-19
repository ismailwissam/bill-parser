#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common.h"

char* collect_dir = NULL;
char* pretreat_dir = NULL;
char* run_dir = NULL;
int collect_parallel_num = 1;
int pretreat_parallel_num = 1;
int force_update = 0;
int debug = 0;
int collect_point_num = 0;

int err_log(const char * format, ...)
{
	time_t t;
	struct tm *systime;
	FILE *fp;
	va_list ap;

	if(debug)
	{
  		va_start(ap, format);
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
	fprintf(fp, "------------------------------------------------------------\n");
	fprintf(fp, "%s", asctime(systime));

	va_start(ap, format);
	vfprintf(fp, format, ap);
	va_end(ap);
	fprintf(fp, "\n");
  
	fclose(fp);
	
	return 0;
}

int get_time(char * par)
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
 *  pre_suf_check()
 */
parse_code_e pre_suf_check(const char * name, const char * prefix, const char * suffix)
{
	if(name == NULL)
		return PARSE_UNMATCH;
		
	/*前缀匹配检查 */
	if(prefix != NULL)
	{
		if(strstr(name, prefix) != name)
			return PARSE_UNMATCH;
	}
	/* 后缀匹配检查 */
	if(suffix != NULL)
	{
		if(strlen(name)<strlen(suffix))
			return PARSE_UNMATCH;
		if(strcmp( (name+strlen(name)-strlen(suffix)), suffix) != 0 )
			return PARSE_UNMATCH;
	}
	
	return PARSE_MATCH;
}

/* 
 *  get_orig_file_name()
 */
parse_code_e get_orig_file_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_file_name)
{
	int              ret;
	DIR *            pdir = NULL;
	struct dirent *	 pdirent;
	struct stat stat_buff;
	char             tmp_file_name[MAX_FILENAME];
	
	ret = PARSE_UNMATCH;

	if(dir_name == NULL)
    {
		err_log("get_orig_file_name: dir name is null\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
    }

    pdir = opendir(dir_name);
	if(pdir == NULL)
	{
		err_log("get_orig_file_name: opendir fail\n");
		ret = PARSE_FAIL;
		goto Exit_Pro;	
	}

	while( (pdirent = readdir(pdir)) != NULL )
	{
		/* 前后缀匹配检查 */
		if( pre_suf_check(pdirent->d_name, prefix, suffix) != PARSE_MATCH )
			continue;

        /* 文件检查 */
        strcpy(tmp_file_name, dir_name);
        strcat(tmp_file_name, "/");
        strcat(tmp_file_name, pdirent->d_name);

		if(stat(tmp_file_name, &stat_buff) == -1)
		{
			err_log("get_orig_file_name: stat %s fail\n", pdirent->d_name);
			ret = PARSE_FAIL;
			break;
		}
        if(S_ISREG(stat_buff.st_mode))
        {
            strcpy(ret_file_name, pdirent->d_name);
            ret = PARSE_MATCH;
            break;	
        }	
	}

Exit_Pro:
	if(pdir != NULL)
        	closedir(pdir);
	return ret;
}

/*
 * clear_work_dir()
 *
 * return 0 success, 1 fail.
 */
int clear_dir_file(const char * dir_name, const char * prefix, const char * suffix)
{
    char    file_name[MAX_FILENAME];
    char    tmp_file_name[MAX_FILENAME];

	memset(file_name, 0, sizeof(file_name));
	while(get_orig_file_name(dir_name, prefix, suffix, file_name) == PARSE_MATCH)
	{
        sprintf(tmp_file_name, "%s/%s", dir_name, file_name);
		if(unlink(tmp_file_name) != 0)
		{
			err_log("clear_dir: unlink file %s fail\n", tmp_file_name);
			return 1;
		}
		memset(file_name, 0, sizeof(file_name));
    }

    return 0;
}

