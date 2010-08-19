/************************************************************************/
/* huawei_a1.c                                                          */
/*                                                                      */
/* 华为MSOFTX3000端局ASN1话单解析模块                                   */
/*                                                                      */
/* create by wangxiaohui at 2010.5.19                                   */
/*                                                                      */
/* modify by wangxiaohui at 2010.5.23                                   */
/*           本次修改主要是因为在数据库中第一列都是网元名称，而原先的输 */
/*           中是不包含网元名称字段的，现在加上这个信息                 */
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "interface.hpp"

/* micro difinition */
#define      ERR_LOG_FILE        "./log/pretreat_huawei_a1_err"
#define      MAX_FILENAME        256
#define      MAX_BUFFER          64
#define      MAX_OUTPUT_CSV      4
#define      TRUE                1
#define      FALSE               0

/* type difinition */
typedef unsigned char  BOOL;

typedef struct {
    FILE *fp;
    char file_name[MAX_FILENAME];
} t_output_file;

enum enum_HW_A1_BILL_TYPE
{
    INVALID_BILL_TYPE = -1,
    HW_BT_A1_MOC = 0,    //asn.1 format bill
    HW_BT_A1_MTC,
    HW_BT_A1_CFW,
    HW_BT_A1_TRANSIT
};

const char * bill_types[MAX_OUTPUT_CSV] = 
{
    "moc",
    "mtc",
    "cfw",
    "tran"
};

/* internal function difinition */
static int err_log(const char * format,...);
static int get_commit_file_name(const char * original_file_name, const char * bill_type, char * ret_file_name);
static int get_ne_name(const char * original_file_name, char * ret_ne_name);

/*
 *huawei_a1()
 *          return 0 success, 1 fail
 */
int huawei_a1(char * in_file_name, char * out_file_name, int * rec_num)
{
    int               ret = 0;
    int               record_num;
    int               record_type;
    t_output_file     csv_files[MAX_OUTPUT_CSV];
    char              tmp_file_name[MAX_FILENAME];
    char              ne_name[MAX_BUFFER];
    int               item_index;
    const char        *item_name;
    const char        *item_value;
    BOOL              first_item;
    int               i;

    /* 初始化输出文件结构 */
    for(i = 0; i < MAX_OUTPUT_CSV; ++i)
    {
        csv_files[i].fp = NULL;
        memset(csv_files[i].file_name, 0, sizeof(csv_files[i].file_name));
    }

    /* 获取网元名称 */
    memset(ne_name, 0, sizeof(ne_name));
    if(get_ne_name(in_file_name, ne_name) != 0)
    {
        err_log("huawei_a1: get ne name fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    /* 打开话单文件并进行预处理 */
    if(!ParseFile(in_file_name))
    {
        err_log("huawei_a1: parse %s fail\n", in_file_name);
        ret = 1;
        goto Exit_Pro;
    }

    /* 循环获取每一条话单记录并进行处理 */
    record_num = 0;
    while(FetchNextBill())
    {
        /* 获取当前话单记录类型 */
        record_type = CurBillType();
        if (record_type == INVALID_BILL_TYPE)
        {
            /* 
             * 如果不是主叫、被叫、前转、汇接话单
             * 不做处理
             */
            continue;
        }

        record_num++;

        /* 判断对应的输出文件是否打开，如果没有打开就打开它 */
        if(csv_files[record_type].fp == NULL)
        {
            if(get_commit_file_name(in_file_name, bill_types[record_type], csv_files[record_type].file_name) != 0)
            {
                err_log("huawei_a1: generate output csv file name fail\n");
                ret = 1;
                goto Exit_Pro;
            }

            sprintf(tmp_file_name, "%s/%s", out_file_name, csv_files[record_type].file_name);

            csv_files[record_type].fp = fopen(tmp_file_name, "w+");
            if(csv_files[record_type].fp == NULL)
            {
                err_log("huawei_a1: fopen %s fail\n", tmp_file_name);
                ret = 1;
                goto Exit_Pro;
            }
        }

        /* 首先把网元名称输出到csv文件中作为第一列 */
        fprintf(csv_files[record_type].fp, "%s,", ne_name);

        /* 把当前话单记录写入输出文件 */
        item_index = 0;
        first_item = TRUE;
        while(1)
        {
            item_name = GetItemName(record_type, item_index);
            if(item_name == NULL)
            {
                fprintf(csv_files[record_type].fp, "\n");
                break;
            }

            if(first_item)
            {
                first_item = FALSE;
            }
            else
            {
                fprintf(csv_files[record_type].fp, ",");
            }

            item_value = GetItemData(item_name);
            
            if(item_value == NULL)
            {
                fprintf(csv_files[record_type].fp, "");
            }
            else
            {
                /* 
                 * 以下字段解析出的数据超长，造成
                 * 入库时发生错误，而且目前这些字段没有被使用，所以
                 * 暂时忽略这些字段的值
                 */
                if(strcmp(item_name, "supplServicesUsed") == 0
                   || strcmp(item_name, "freeFormatData") == 0)
                {
                    fprintf(csv_files[record_type].fp, "");
                }
                else
                {
                    fprintf(csv_files[record_type].fp, "%s", item_value);
                }
            }

            ++item_index;
        }
    }

    /* 返回生成的CSV文件名 */
    strcat(out_file_name, ":");
    for(i = 0; i < MAX_OUTPUT_CSV; ++i)
    {
        if(csv_files[i].fp != NULL)
        {
            strcat(out_file_name, csv_files[i].file_name);
            strcat(out_file_name, ";");
        }
    }

    /* 返回话单记录总数 */
    (*rec_num) = record_num;

Exit_Pro:
    for(i = 0; i < MAX_OUTPUT_CSV; ++i)
    {
        if(csv_files[i].fp != NULL)
        {
            fclose(csv_files[i].fp);
        }
    }
    return ret;
}

//------------------------------------------------------------------------------------------------
static int err_log(const char * format,...)
{
	time_t t;
	struct tm *systime;
	FILE *fp;
	
	va_list ap;

  	va_start(ap, format);
  	vprintf(format,ap);
  	va_end(ap);
  	printf("\n");

	fp=fopen(ERR_LOG_FILE,"a+");

	if(fp==NULL)
	{
		return 1;
	}
	
	t=time(NULL);
	systime=localtime(&t);
	fprintf(fp,"------------------------------------------------------------\n");
	fprintf(fp,"%s", asctime(systime));

	va_start(ap, format);
	vfprintf(fp, format, ap);
	va_end(ap);
	fprintf(fp, "\n");
  
	fclose(fp);
	
	return 0;
}

static int get_commit_file_name(const char * original_file_name, const char * bill_type, char * ret_file_name)
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

    /* 采集点编号 */
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorret\n");
        return 1;
    }
    strncpy(ret_file_name, ptr_front, ++ptr_back - ptr_front);

    /* 厂家 */
    ptr_front = ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorret\n");
        return 1;
    }
    strncat(ret_file_name, ptr_front, ++ptr_back - ptr_front);

    /* 端局 */
    ptr_front = ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorret\n");
        return 1;
    }
    strncat(ret_file_name, ptr_front, ++ptr_back - ptr_front);

    /* 话单类型 */
    if (bill_type != NULL)
    {
        strcat(ret_file_name, bill_type);
        strcat(ret_file_name, "_");
    }

    /* 附加后面的标题串 */
    ptr_front = ptr_back;
    ptr_back = strchr(ptr_front, '.');
    if(ptr_back == NULL)
    {
        strcat(ret_file_name, ptr_front);
    }
    else
    {
        strncat(ret_file_name, ptr_front, ptr_back - ptr_front);
    }

    /* 附加扩展名 */
    strcat(ret_file_name, ".csv");

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

    /* 采集点编号 */
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }

    /* 厂家 */
    ptr_front = ++ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }

    /* 端局 */
    ptr_front = ++ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }
    strncpy(ret_ne_name, ptr_front, ptr_back - ptr_front);

    return 0;
}

