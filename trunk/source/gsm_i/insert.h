
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
/*����ÿ���ӽ�����������*/
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
	char file_id[16];                   //�����ļ�id; 20050805_3366
	int  serial_num;	            //�������

	char add_cdr_switch[11];            //����������Դ, pretreatΪ�ɼ�����;
	                                    //mark��Ϊ����������MSC1, GMSC1, TMSC1��

	char add_hlr[11];                   //���˺������hlr
	
	char add_my_area_code[11];          //����cell�������
	char add_my_area_name[31];          //����cell��������
	
	char add_my_scp[11];                //���˺���SCP
	char add_product_package[21];       //���˺����Ʒ ��'���鿨', '���к�һ','80�ײ�'��
	char add_a_flag[2];	            //���˺��������ʶ  L-���ء�N-���ڡ�I-���ʡ�U-δ֪
	char add_a_name[31];                //���˺�����������  д����; �������Ϻ���
	char add_u_type[11];                //���˺������� ΪPPC��VPN��IN��NORMAL��
	char add_u_group[11];               //���˺���ָ���û�Ⱥ��ʶ VIP-��ͻ���
	char add_gc[2];                     //���˺���G&C��ʶ 1-��, 0-����

	int  add_time_diff;                 //���п�ʼʱ��ͻ����ļ�ʱ��λ��
	
	char add_layout_time[18];           //����ʱ��yyyymmddhhmi; miȡ00��15��30��45
	char add_call_begin_time[18];       //���п�ʼʱ��yyyymmddhhmiss
	int  add_occupy_duration;	    //ռ��ʱ����
	int  add_talk_duration;	            //ͨ��ʱ����
	
	char add_record_type[3];	    //��¼���� 00 Single, 01 First, 02 Intermediate, 03 Last

	char add_call_type[3];	            //��������
	                                    //S1(MOC)��S2(MTC)��
	                                    //S3(SMS MO)��S4(SMS MT)��
	                                    //S5(ROAM)��S6(TRANSIT)��
	                                    //S7(sSRegistration,sSErasure��)��S8(����) 

	char add_cf_type[3];	            //��ת���� 
	                                    //00�޺�ת��
	                                    //21 Call forwarding unconditional��
	                                    //29 Call forwarding on mobile subscriber busy
	                                    //2A Call forwarding on no answer
	                                    //2B CF_not_reachable

	char add_my_imsi[17];               //����IMSI
	char add_my_imei[17];               //����IMEI

	char add_my_dn[33];                 //���˺���, TRANSIT����������
	char add_other_dn[33];              //�Զ˺���, TRANSIT�����ű���
	char add_third_party[33];           //����������

	char add_my_msrn[33];               //���˶�̬���κ�
	char add_other_msrn[33];            //�Զ˶�̬���κ�

	char add_other_a_flag[2];	    //�Զ˺��������ʶ  L-���ء�N-���ڡ�I-���ʡ�U-δ֪
	char add_other_a_name[31];          //�Զ˺�����������  д����; �������Ϻ���

	char add_sub_my_dn[17];             //����hlr�Ŷ�
	char add_sub_other_dn[17];          //�Զ��ж�����(��������)

	char add_other_direction[21];       //�Զ���������

	int  add_my_cell_num;    	    //����cell
	int  add_other_cell_num;	    //�Զ�cell

	char add_in_trunk_group_id[17];     //���м�Ⱥ��ʶ
	int  add_in_cic_pcm;	            //���м�2M(PCM)
	int  add_in_cic_ts;	            //���м�2M(PCM)��ʱ϶

	char add_out_trunk_group_id[17];    //���м�Ⱥ��ʶ
	int  add_out_cic_pcm;   	    //���м�2M(PCM)
	int  add_out_cic_ts;	            //���м�2M(PCM)��ʱ϶

	char add_phone_manufacturer[31];    //�ֻ���������
	char add_phone_type[31];            //�ֻ��ͺ�
	
	char charging_date[9];	            //��������ʱ��yyyymmdd
	char answer_time[9];	            //Ӧ��ʼʱ��hh:mi:ss 55:55:55��ʾδӦ��
	char disconnect_reason[3];	    //���н���ԭ�� 10������11������
	char billing_id[9];	            //����������, ��һ�κ��в������Ż���ʱ, ����һ�κ��еĶ��Ż���

	int  record_type;	            //��¼����(ԭʼ)
        				    //4, Single Billing Record
                                            //5, First Intermediate Billing Record
                                            //6, Intermediate Billing Record
                                            //7, Last Billing Record
	int  call_type; 	            //��������(ԭʼ)
	char reason_for_termination[3];     //���н���ԭ��(ԭʼ)
	char served_dn[33];                 //���˺���(ԭʼ),TRANSIT����������
	char dialled_other_party[33];       //�Զ˺���[�������](ԭʼ),TRANSIT�����ű���,SMS MO�ŶԶ˺���
	char translated_other_party[33];    //�Զ˺���[�������](ԭʼ)
	char third_party[33];               //����������(ԭʼ)
	char service_centre_address[33];    //����Ϣ���ĺ���
	int  call_hold_count;               //���б��ִ���
	int  call_wait_count;               //���еȴ�����
	char ss_code[3];                    //����ҵ�����
	char exchange_id[17];               //������id
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
