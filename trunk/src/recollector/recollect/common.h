#ifndef _COMMON_H_
#define _COMMON_H_

/* micro definition */
#define      MAX_FILENAME           256
#define      MAX_LONG_FILENAME      512
#define      MAX_BUFFER             1024
#define      MAX_LONG_BUFFER        2048
#define      MAX_TIME               15
#define      MAX_DATE               9
#define      MAX_TABLENAME          64
#define      MAX_CHILD_PROCESS      128
#define      PROCESS_SLEEP_TIME     60
#define      DEFAULT_COLLECT_DIR    "./data/collect"
#define      DEFAULT_PRETREAT_DIR   "./data/pretreat"
#define      DEFAULT_RUN_DIR        "./"
#define      DEFAULT_DB_USER        "obirawdb"
#define      DEFAULT_DB_PASSWORD    "obirawdb\\$zj2010"
#define      DEFAULT_DB_SERVER      "obidb3_132"
#define      WORK_DIR               "./work"
#define      LOG_DIR                "./log"
#define      CONF_DIR               "./conf"
#define      TEMPLATE_DIR           "./template"
#define      MODULE_DIR             "./module"
#define      ERR_LOG_FILE           "./log/recollect_err"
#define      COLLECT_CONF           "collect.conf"
#define      ARCHIVE_DIR            "./conf/archive"
#define      FALSE                  0
#define      TRUE                   1

/* type definition */
typedef enum parse_code {
	PARSE_FAIL 	= -1,
	PARSE_MATCH	= 1,
	PARSE_UNMATCH	= 2,
} parse_code_e;

typedef struct 
{
	int pid;
	int sleep_time;
} t_child_process_status;

typedef unsigned int BOOL;

/* global variant definition */
extern char* collect_dir;
extern char* pretreat_dir;
extern char* run_dir;
extern char* db_user;
extern char* db_password;
extern char* db_server;
extern int collect_parallel_num;
extern int pretreat_parallel_num;
extern int insert_parallel_num;
extern int debug;
extern int collect_point_num;

/* common function definition */
int err_log(const char * format, ...);
int get_time(char * par);
parse_code_e pre_suf_check(const char * name, const char * prefix, const char * suffix);
parse_code_e get_orig_file_name(const char * dir_name, const char * prefix, const char * suffix, char * ret_file_name);
int clear_dir_file(const char * dir_name, const char * prefix, const char * suffix);

#endif
