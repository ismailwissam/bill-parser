
#ifndef __200506_insert.h_
#define __200506_insert.h_

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

extern int   errno;


#define      SEPARATE_CHR      ','

#define      SEMKEY            781

#define      MAX_CHILD_PROCESS 128
#define      SLEEP_TIME        60

#define      LOG      "./log"

#define      ERR_LOG_FILE        "./log/insert.log"
#define      PREFIX_RUN_LOG_FILE "./log/insert_run"

/*-----------------------------------------------*/
/*用于每个子进程设置其编号*/
extern int          curr_process_number;

extern char *       progname;
extern char *       file_in_dir;
extern char *       run_dir;
extern int          debug;
extern int          parallel_child_process;

extern char *       db_user;
extern char *       db_password;
extern char *       db_server;

extern CS_CONTEXT  *Ex_context;
/*-----------------------------------------------*/

typedef unsigned char  u_8;
typedef unsigned short u_16;
typedef unsigned int   u_32;

typedef struct {
	int  pid;
	int  sleep_time;
} t_child_process_status;

typedef struct {
	char file_id[16];                   //话单文件id; 20050805_3366
	int  serial_num;	            //话单序号

	char add_cdr_switch[11];            //话单数据来源, pretreat为采集点编号;
	                                    //mark改为交换机名称MSC1, GMSC1, TMSC1等

	char add_hlr[11];                   //本端号码归属hlr
	
	char add_my_area_code[11];          //本端cell区域编码
	char add_my_area_name[31];          //本端cell区域名称
	
	char add_my_scp[11];                //本端号码SCP
	char add_product_package[21];       //本端号码产品 如'乡情卡', '长市合一','80套餐'等
	char add_a_flag[2];	            //本端号码区域标识  L-本地、N-国内、I-国际、U-未知
	char add_a_name[31];                //本端号码区域名称  写汉字; 北京、上海等
	char add_u_type[11];                //本端号码类型 为PPC、VPN、IN、NORMAL等
	char add_u_group[11];               //本端号码指定用户群标识 VIP-大客户等
	char add_gc[2];                     //本端号码G&C标识 1-是, 0-不是

	int  add_time_diff;                 //呼叫开始时间和话单文件时间差单位天
	
	char add_layout_time[18];           //规整时间yyyymmddhhmi; mi取00、15、30、45
	char add_call_begin_time[18];       //呼叫开始时间yyyymmddhhmiss
	int  add_occupy_duration;	    //占用时长秒
	int  add_talk_duration;	            //通话时长秒
	
	char add_record_type[3];	    //记录类型 00 Single, 01 First, 02 Intermediate, 03 Last

	char add_call_type[3];	            //呼叫类型
	                                    //S1(MOC)、S2(MTC)、
	                                    //S3(SMS MO)、S4(SMS MT)、
	                                    //S5(ROAM)、S6(TRANSIT)、
	                                    //S7(sSRegistration,sSErasure等)、S8(其它) 

	char add_cf_type[3];	            //呼转类型 
	                                    //00无呼转、
	                                    //21 Call forwarding unconditional、
	                                    //29 Call forwarding on mobile subscriber busy
	                                    //2A Call forwarding on no answer
	                                    //2B CF_not_reachable

	char add_my_imsi[17];               //本端IMSI
	char add_my_imei[17];               //本端IMEI

	char add_my_dn[33];                 //本端号码, TRANSIT话单放主叫
	char add_other_dn[33];              //对端号码, TRANSIT话单放被叫
	char add_third_party[33];           //第三方号码

	char add_my_msrn[33];               //本端动态漫游号
	char add_other_msrn[33];            //对端动态漫游号

	char add_other_a_flag[2];	    //对端号码区域标识  L-本地、N-国内、I-国际、U-未知
	char add_other_a_name[31];          //对端号码区域名称  写汉字; 北京、上海等

	char add_sub_my_dn[17];             //本端hlr号段
	char add_sub_other_dn[17];          //对端判断依据(流量方向)

	char add_other_direction[21];       //对端流量方向

	int  add_my_cell_num;    	    //本端cell
	int  add_other_cell_num;	    //对端cell

	char add_in_trunk_group_id[17];     //入中继群标识
	int  add_in_cic_pcm;	            //入中继2M(PCM)
	int  add_in_cic_ts;	            //入中继2M(PCM)的时隙

	char add_out_trunk_group_id[17];    //出中继群标识
	int  add_out_cic_pcm;   	    //出中继2M(PCM)
	int  add_out_cic_ts;	            //出中继2M(PCM)的时隙

	char add_phone_manufacturer[31];    //手机生产厂商
	char add_phone_type[31];            //手机型号
	
	char charging_date[9];	            //话单产生时间yyyymmdd
	char answer_time[9];	            //应答开始时间hh:mi:ss 55:55:55表示未应答
	char disconnect_reason[3];	    //呼叫结束原因 10正常、11非正常
	char billing_id[9];	            //话单索引号, 当一次呼叫产生多张话单时, 关联一次呼叫的多张话单

	int  record_type;	            //记录类型(原始)
        				    //4, Single Billing Record
                                            //5, First Intermediate Billing Record
                                            //6, Intermediate Billing Record
                                            //7, Last Billing Record
	int  call_type; 	            //呼叫类型(原始)
	char reason_for_termination[3];     //呼叫结束原因(原始)
	char served_dn[33];                 //本端号码(原始),TRANSIT话单放主叫
	char dialled_other_party[33];       //对端号码[拨打号码](原始),TRANSIT话单放被叫,SMS MO放对端号码
	char translated_other_party[33];    //对端号码[处理号码](原始)
	char third_party[33];               //第三方号码(原始)
	char service_centre_address[33];    //短消息中心号码
	int  call_hold_count;               //呼叫保持次数
	int  call_wait_count;               //呼叫等待次数
	char ss_code[3];                    //补充业务代码
	char exchange_id[17];               //交换机id
} t_cdr;

typedef enum parse_code {
	PARSE_FAIL 	= -1,
	PARSE_MATCH	= 1,
	PARSE_UNMATCH	= 2,
} parse_code_e;


void         usage(int status);
int          get_time(char * par);
void         err_log(char * format,...);
void         daemon_start(void);
void         P(void);
void         V(void);
void         strtoint(u_8 **in, int *out);
void         strtostr(u_8 **in, u_8 *out);

parse_code_e pre_suf_check(char * name,char * prefix,char * suffix);
int          commit_file(char * in_file_name,char * begin,char * end,long in_file_size,long rec_num,long db_rec_num);
int          insert(char * in_file_name,int idb_num);
parse_code_e get_in_file_name(int parallel_number,char * dir_name,char * prefix,char * suffix,char * ret_file_name);
void         zero_buff_2(char *** buff);
int          get_idb_num(int parallel_number, int *o_idb_num);
void         process_insert(int current_number,int parallel_number);

CS_RETCODE CS_PUBLIC ex_execute_query(CS_CONNECTION *connection,CS_CHAR *cmdbuff,char ***buff, \
                                        int col,int begin,int num,int * o_num);
CS_RETCODE CS_PUBLIC ex_fetch_query(CS_COMMAND *cmd, char ***buff, int col, int begin, int num, int *o_num);
int do_proc_after_insert(CS_CONNECTION *connection, int idb_num, char *par_cdr_switch, \
                          char *par_file_id, int *o_db_rec_num);


#endif
