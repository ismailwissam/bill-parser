/************************************************************************/
/* huawei.c                                                             */
/*                                                                      */
/* huawei MSOFTX3000 gsm gateway binary bill parser                     */
/*                                                                      */
/* created by wangxiaohui at 2010.4.30                                  */
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>

/*-----------------------------------------------------------------------------------*/
/* micro difinition */
#define      ERR_LOG_FILE        "./log/pretreat.huawei.log"

//-------------------------------------------------------------------------------------
/* type difinition */
typedef unsigned char  u_8;
typedef unsigned short u_16;
typedef unsigned int   u_32;

/************************************************************/
/*                 ��Ϊ������ʽ˵��                         */
/* ��Ϊ�����ƻ�����ʽ����ͬ�ģ�����Ҫ�ֳɶ��ֻ�����������   */
// **********************************************************
// ����                                  ����      λ��
// **********************************************************
// ��ˮ��                                4         1-4       
// ģ������ˮ��                          4         5-8       
// ģ���                                1         9         
// ��������                              1         10        
// ͨ����ʼʱ��                          7         11-17     
// ���к���                              14        18-31     
// ���к���                              14        32-45     
// ����������                            14        46-59     
// �������κ���                          8         60-67     
// ���Ʒ��û�MSISDN                      8         68-75     
// ���Ʒ��ƶ��û�IMSI                    8         76-83     
// ���Ʒ��ƶ��û�IMEI                    8         84-91     
// ���м�Ⱥ��                            2         92-93     
// ���м�Ⱥ��                            2         94-95     
// ͨ��ʱ��                              4         96-99     
// �׻���������                          4         100-103   
// �м仰�����к�                        1         104       
// ��¼����                              3b        105       
// �û����                              5b        105       
// ͨ����ֹԭ��                          1         106       
// �����м仰��ԭ��                      1         107       
// ����MSC��                             8         108-115   
// ���Ʒ��û���ǰ����MSC��               8         116-123   
// �����û���ǰλ����                    2         124-125   
// �����û���ǰС��                      2         126-127   
// �����û���ʼλ����                    2         128-129   
// �����û���ʼС��                      2         130-131   
// �����û���ǰλ����                    2         132-133   
// �����û���ǰС��                      2         134-135   
// �����û���ʼλ����                    2         136-137   
// �����û���ʼС��                      2         138-139   
// ���Ʒ��ƶ��û����вο�                1         140       
// ����ģʽ                              4b        141       
// �绰ҵ������ҵ���־                4b        141       
// ��������                              1         142       
// �绰ҵ������ҵ����                  1         143       
// ҵ�����                              1         144       
// ����ҵ����1                           1         145        
// ����ҵ����2                           1         146         
// ����ҵ����3                           1         147         
// ����ҵ����4                           1         148         
// ���Ʒ��ƶ��û���ʼCLASSMARK           3         149-151   
// ���Ʒ��ƶ��û���ǰCLASSMARK           3         152-154   
// ͸����͸��ָʾ                        2b        155       
// �Ƿ�ʹ��DTMF                          2b        155       
// �Ʒ���ѱ�־                          4b        155       
// ���α�־                              1b        156       
// �ȼƷѱ�־                            1b        156       
// �������                              6b        156       
// ����ָʾ                              2         157-158   
// ����                                  2         159-160   
// ���ӷ�                                2         161-162   
// �ôκ���ռ�õ�B�ŵ���                 1         163       
// �ֽ���                                4         164-167   
// ����Ϣ���ĵ�ַ                        8         168-175   
// ÿ�����ݵ�Ԫ���ֽ���                  2         176-177   
// ҵ���                                4         178-181   
// SCFID                                 5         182-186   
// �����ֽ�                              1         187       
// FCI��Ϣ                               40        188-227   
// �ƶ����вο���                        8         228-235   
// ϵͳ��������                          1         236
// ����ָʾ                              1         237
// �����ֽ�                              5         238-242
// channel mode                          4b        243
// channel                               4b        243
// MAPByPassInd��ʶ                      1b        244
// Ro��·ȱʡ���д���                    2b        244
// voBB �û���ʶ                         1b        244
// ����bit                               4b        244
// ��ǿ�û����                          1         245
// cARPֵ                                1         246
// �û��߼��Ʒ�������                    3         247-249
// CMN��ʶ                               1b        250
// �ͷŷ�                                1         251
// ������                                1         252
// �����������                          14        253-266
// ��·�쳣��ʱ���                      7         267-273
// ���һ�γɹ�CCR������ʱ���           7         274-280
// ͨ��ʱ������ʱƫ��                    1         281
// ��·�쳣��ʱ������ʱƫ��              1         282
// ���һ�γɹ�CCR������ʱ������ʱƫ��   1         283
// ����λ                                17        284-300
// **********************************************************
typedef struct {
    char bill_sequence_in_msc[4];                  // ��ˮ��
    char bill_sequence_in_module[4]                // ģ������ˮ��
    char module_no;                                // ģ���
    char cdr_type;                                 // ��������
    char charge_start_time[7];                     // ͨ����ʼʱ��
    char calling_num[14];                          // ���к���
    char dialed_num[14];                           // ���к���
    char connected_num[14];                        // ����������
    char msrn[8];                                  // �������κ���
    char served_msisdn[8];                         // ���Ʒ��û�MSISDN
    char served_imsi[8];                           // ���Ʒ��ƶ��û�IMSI
    char served_imei[8];                           // ���Ʒ��ƶ��û�IMEI
    char incoming_trunk_group_id[2];               // ���м�Ⱥ��
    char outgoing_trunk_group_id[2];               // ���м�Ⱥ��
    char charge_duration[4];                       // ͨ��ʱ��������Ϊ��λ��
    char index_of_first_cdr[4];                    // �׻���������
    char sequence_of_intermediate_cdr;             // �м仰�����к�
    char record_user_type;                         // ��¼����:�û����
    char cause_for_call_termination;               // ͨ����ֹԭ��
    char cause_for_intermediate_record;            // �����м仰��ԭ��
    char local_msc_id[8];                          // ����MSC��
    char peer_msc_id[8];                           // ���Ʒ��û���ǰ����MSC��
    char current_lac_of_caller[2];                 // �����û���ǰλ����
    char current_ci_of_caller[2];                  // �����û���ǰС��
    char initial_lac_of_caller[2];                 // �����û���ʼλ����
    char initial_ci_of_caller[2];                  // �����û���ʼС��
    char current_lac_of_called[2];                 // �����û���ǰλ����
    char current_ci_of_called[2];                  // �����û���ǰС��
    char initial_lac_of_called[2];                 // �����û���ʼλ����
    char initial_ci_of_called[2];                  // �����û���ʼС��
    char call_reference;                           // ���Ʒ��ƶ��û����вο�
    char transmission_mode_tbs_flag;               // ����ģʽ:�绰ҵ������ҵ���־
    char bearer_capability;                        // ��������
    char service_code_of_tbs;                      // �绰ҵ������ҵ����
    char gsm_gsvn;                                 // ҵ�����
    char ss_code1;                                 // ����ҵ����1
    char ss_code2;                                 // ����ҵ����2
    char ss_code3;                                 // ����ҵ����3
    char ss_code4;                                 // ����ҵ����4
    char initial_classmark_of_served_ms[3];        // ���Ʒ��ƶ��û���ʼCLASSMARK
    char current_classmark_of_served_ms[3];        // ���Ʒ��ƶ��û���ǰCLASSMARK
    char transparency_dtmf__free_indicator;        // ͸����͸��ָʾ:�Ƿ�ʹ��DTMF:�Ʒ���ѱ�־
    char roam_hotbill_flag_and_action_result;      // ���α�־:�ȼƷѱ�־:�������
    char charging_case[2];                         // ����ָʾ
    char money_per_count[2];                       // ���ʣ�������ҵķ�Ϊ��λ��
    char add_fee[2];                               // ���ӷѣ�������ҵķ�Ϊ��λ��
    char number_of_b_channels_occupied;            // �ôκ���ռ�õ�B�ŵ���
    char number_of_bytes[4];                       // ���ͻ���ܶ���Ϣ�ֽ���
    char smsc_address[8];                          // ����Ϣ���ĵ�ַ
    char bytes_per_packet[2];                      // ÿ�����ݵ�Ԫ���ֽ���
    char servicekey[4];                            // ҵ���
    char scfid[5];                                 // SCFID
    char reserve1;                                 // �����ֽ�
    char fcidata[40];                              // FCI��Ϣ
    char call_reference_num[8];                    // �ƶ����вο���
    char system_type;                              // ϵͳ��������
    char rate_indication;                          // ����ָʾ
    char reserve2[5];                              // �����ֽ�
    char channel_mode_and_channel;                 // channel mode; channel
    char mapbypassind_ro_link_default_call_vobb_user_flag;    // MAPByPassInd��ʶ; Ro��·ȱʡ���д���; voBB �û���ʶ 
    char e_category;                               // ��ǿ�û�����
    char carp;                                     // cARPֵ
    char charge_area_code[3];                      // �û��߼��Ʒ�������
    char cmn_flag;                                 // CMN��ʶ 
    char disconnect_party;                         // �ͷŷ�
    char tariff_code;                              // ������
    char translated_number[14];                    // �����������
    char link_exception_time[7];                   // ��·�쳣��ʱ���
    char latest_success_ccr_operation_time[7];     // ���һ�γɹ�CCR������ʱ���
    char dst_offset;                               // ͨ��ʱ������ʱƫ��
    char link_exception_dst_offset;                // ��·�쳣��ʱ������ʱƫ��
    char latest_success_ccr_operation_dst_offset;  // ���һ�γɹ�CCR����������ʱƫ��
    char reserve3[17];                             // �����ֽ�
} t_cdr;

//------------------------------------------------------------------------------------------------
/* internal function difinition */
static void err_log(char * format,...);
static void p_32_swap(u_32 *p_a);
static void p_16_swap(u_16 *p_a);
static void p_8_swap(u_8 *p_a);
static int  hextoint(u_8 * hex,int hex_num);
static void hextostr(u_8 * out_str,u_8 * hex,int hex_num);
static void type_hextostr(u_8 * out_str,u_8 * hex,int hex_num);
static void bcdhextostr(u_8 * out_str,u_8 * hex,int hex_num);
static void type_bcdhextostr(u_8 * out_str,u_8 * hex,int hex_num);

//------------------------------------------------------------------------------------------------
/* interface difinition */
int huawei(char * in_file_name, char * out_file_name, int * rec_num);

//------------------------------------------------------------------------------------------------
//--main for test
int main(int argc,char * argv[])
{
	int rec_num;
	int r;
	
	r=huawei(argv[1],argv[2],&rec_num);
	
	printf("huawei return %d,rec_num=%d\n",r,rec_num);
	
	return 0;
}

/*
 *huawei()
 *          return 0 success, 1 fail
 *
 */
int huawei(char * in_file_name, char * out_file_name, int * rec_num)
{
	int                    ret;

	FILE                   *fp_i=NULL;
	FILE                   *fp_o=NULL;

	t_cdr                  cdr;

	/*ÿ�ζ�100������, ÿ��300�ֽڶ���*/
        const int              cdr_len = 300;
        const int              cdr_num_once = 100;
	u_8                    buff[cdr_num_once][cdr_len];     
	u_8                    *ptr=NULL;

	int                    serial_num;
	int                    i, read_cdr_num;

        u_32                   u32_num;
        u_16                   u16_num;
        u_32                   year, month, day, hour, minute, second;

	ret          = 0;
	serial_num   = 0;
	(*rec_num)   = 0;

        /* open the input file for read */
	fp_i = fopen(in_file_name, "r");
	if(fp_i == NULL)
	{
        	err_log("huawei: fopen %s fail\n",in_file_name);
        	ret = 1;
        	goto Exit_Pro;
	}
	
        /* open the output file for write */
	fp_o = fopen(out_file_name, "w+");
	if(fp_o == NULL)
	{
        	err_log("huawei: fopen %s fail\n",out_file_name);
        	ret = 1;
        	goto Exit_Pro;
	}

	while(1)
	{
		memset(buff, 0, sizeof(buff));
		read_cdr_num = fread(buff, cdr_len, cdr_num_once, fp_i);
		
		if(read_cdr_num > 0)
		{
			for(i = 0; i < read_cdr_num; i++)
			{
				serial_num++;

				/* ******************** ��ȡ������¼ **************************************/
				memset(&cdr, 0, sizeof(cdr));
                                memcpy(&cdr, &buff[i][0], sizeof(cdr));
	        		
				/* ********************* ���������¼���ļ� *******************************/
                                // ��ˮ��                                4         1-4       
                                memset(&u32_num, 0, sizeof(u32_num));
                                memcpy(&u32_num, &(cdr.bill_sequence_in_msc[0]), sizeof(u32_num));
                                p_32_swap(&u32_num);
	                        fprintf(fp_o, "%d,", u32_num);

                                // ģ������ˮ��                          4         5-8       
                                memset(&u32_num, 0, sizeof(u32_num));
                                memcpy(&u32_num, &(cdr.bill_sequence_in_module[0]), sizeof(u32_num));
                                p_32_swap(&u32_num);
	                        fprintf(fp_o, "%d,", u32_num);

                                // ģ���                                1         9         
                                u32_num = cdr.module_no;
                                fprintf(fp_o, "%d,", u32_num);

                                // ��������                              1         10        
                                u32_num = cdr.cdr_type;
                                fprintf(fp_o, "%d,", u32_num);

                                // ͨ����ʼʱ��                          7         11-17     
                                memset(&u16_num, 0, sizeof(u16_num));
                                memcpy(&u16_num, &(cdr.charge_start_time[0]), 2);
                                p_16_swap(&u16_num);
                                year = u16_num;
                                month = cdr.charge_start_time[2];
                                day = cdr.charge_start_time[3];
                                hour = cdr.charge_start_time[4];
                                minute = cdr.charge_start_time[5];
                                second = cdr.charge_start_time[6];
                                fprintf(fp_o, "'%d-%d-%d %d:%d:%d',", year, month, day, hour, minute, second);

                                // ���к���                              14        18-31     
                                // ���к���                              14        32-45     
                                // ����������                            14        46-59     
                                // �������κ���                          8         60-67     
                                // ���Ʒ��û�MSISDN                      8         68-75     
                                // ���Ʒ��ƶ��û�IMSI                    8         76-83     
                                // ���Ʒ��ƶ��û�IMEI                    8         84-91     
                                // ���м�Ⱥ��                            2         92-93     
                                // ���м�Ⱥ��                            2         94-95     
                                // ͨ��ʱ��                              4         96-99     
                                // �׻���������                          4         100-103   
                                // �м仰�����к�                        1         104       
                                // ��¼����                              3b        105       
                                // �û����                              5b        105       
                                // ͨ����ֹԭ��                          1         106       
                                // �����м仰��ԭ��                      1         107       
                                // ����MSC��                             8         108-115   
                                // ���Ʒ��û���ǰ����MSC��               8         116-123   
                                // �����û���ǰλ����                    2         124-125   
                                // �����û���ǰС��                      2         126-127   
                                // �����û���ʼλ����                    2         128-129   
                                // �����û���ʼС��                      2         130-131   
                                // �����û���ǰλ����                    2         132-133   
                                // �����û���ǰС��                      2         134-135   
                                // �����û���ʼλ����                    2         136-137   
                                // �����û���ʼС��                      2         138-139   
                                // ���Ʒ��ƶ��û����вο�                1         140       
                                // ����ģʽ                              4b        141       
                                // �绰ҵ������ҵ���־                4b        141       
                                // ��������                              1         142       
                                // �绰ҵ������ҵ����                  1         143       
                                // ҵ�����                              1         144       
                                // ����ҵ����1                           1         145        
                                // ����ҵ����2                           1         146         
                                // ����ҵ����3                           1         147         
                                // ����ҵ����4                           1         148         
                                // ���Ʒ��ƶ��û���ʼCLASSMARK           3         149-151   
                                // ���Ʒ��ƶ��û���ǰCLASSMARK           3         152-154   
                                // ͸����͸��ָʾ                        2b        155       
                                // �Ƿ�ʹ��DTMF                          2b        155       
                                // �Ʒ���ѱ�־                          4b        155       
                                // ���α�־                              1b        156       
                                // �ȼƷѱ�־                            1b        156       
                                // �������                              6b        156       
                                // ����ָʾ                              2         157-158   
                                // ����                                  2         159-160   
                                // ���ӷ�                                2         161-162   
                                // �ôκ���ռ�õ�B�ŵ���                 1         163       
                                // �ֽ���                                4         164-167   
                                // ����Ϣ���ĵ�ַ                        8         168-175   
                                // ÿ�����ݵ�Ԫ���ֽ���                  2         176-177   
                                // ҵ���                                4         178-181   
                                // SCFID                                 5         182-186   
                                // �����ֽ�                              1         187       
                                // FCI��Ϣ                               40        188-227   
                                // �ƶ����вο���                        8         228-235   
                                // ϵͳ��������                          1         236
                                // ����ָʾ                              1         237
                                // �����ֽ�                              5         238-242
                                // channel mode                          4b        243
                                // channel                               4b        243
                                // MAPByPassInd��ʶ                      1b        244
                                // Ro��·ȱʡ���д���                    2b        244
                                // voBB �û���ʶ                         1b        244
                                // ����bit                               4b        244
                                // ��ǿ�û����                          1         245
                                // cARPֵ                                1         246
                                // �û��߼��Ʒ�������                    3         247-249
                                // CMN��ʶ                               1b        250
                                // �ͷŷ�                                1         251
                                // ������                                1         252
                                // �����������                          14        253-266
                                // ��·�쳣��ʱ���                      7         267-273
                                // ���һ�γɹ�CCR������ʱ���           7         274-280
                                // ͨ��ʱ������ʱƫ��                    1         281
                                // ��·�쳣��ʱ������ʱƫ��              1         282
                                // ���һ�γɹ�CCR������ʱ������ʱƫ��   1         283
                                // ����λ                                17        284-300
                                
	                        fprintf(fp_o,"%s\n" ,cdr.exchange_id);
			}
		}

		if(read_cdr_num < cdr_num_once)
		{
			if(ferror(fp_i))
			{
				err_log("huawei: fread %s fail\n",in_file_name);
				ret = 1;
				goto Exit_Pro;
			}
			break;
		}
	}

Exit_Pro:
        /* close the input file and output file */
	if(fp_i != NULL)
        {
            fclose(fp_i);
            fp_i = NULL;
        }
	if(fp_o != NULL)
        {
            fclose(fp_o);
            fp_o = NULL;
        }

	(*rec_num) = serial_num;

	return ret;
}

//------------------------------------------------------------------------------------------------
static void err_log(char * format,...)
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

static void p_32_swap(u_32 *p_a)
{
	u_32 _tmp;

	memcpy(&_tmp,p_a,sizeof(u_32));

        ((u_8 *)p_a)[0] = ((u_8 *)&_tmp)[3];
        ((u_8 *)p_a)[1] = ((u_8 *)&_tmp)[2];
        ((u_8 *)p_a)[2] = ((u_8 *)&_tmp)[1];
        ((u_8 *)p_a)[3] = ((u_8 *)&_tmp)[0];
}

static void p_16_swap(u_16 *p_a)
{
	u_16 _tmp;

	memcpy(&_tmp,p_a,sizeof(u_16));

        ((u_8 *)p_a)[0] = ((u_8 *)&_tmp)[1];
        ((u_8 *)p_a)[1] = ((u_8 *)&_tmp)[0];
}

static void p_8_swap(u_8 *p_a)
{
	u_8 _tmp;
	
	memcpy(&_tmp,p_a,sizeof(u_8));
	
	(*p_a) = ((_tmp&0xf)<<4)+((_tmp>>4)&0xf);
}

/*
 *hextoint()
 */
static int hextoint(u_8 * hex,int hex_num)
{
	int i;
	int r;
	
	r=0;
	
	for(i=0;i<hex_num;i++)
	{
		r=r*256+hex[i];
	}
	return r;
}

/*
 *hextostr()
 */
static void hextostr(u_8 * out_str,u_8 * hex,int hex_num)
{
	int i;
	for(i=0;i<hex_num;i++)
	{
		sprintf(out_str,"%02X",*hex);
		out_str+=2;
		hex++;
	}	
}

/*
 *type_hextostr()
 */
static void type_hextostr(u_8 * out_str,u_8 * hex,int hex_num)
{
	hextostr(out_str,hex+2,hex_num-2);
}

/*
 *bcdhextostr()
 */
static void bcdhextostr(u_8 * out_str,u_8 * hex,int hex_num)
{
	int i;
	u_8 _tmp;
	
	for(i=0;i<hex_num;i++)
	{
		_tmp=(*hex);
		p_8_swap(&_tmp);
		sprintf(out_str,"%02X",_tmp);
		out_str+=2;
		hex++;
	}	
}

/*
 *type_bcdhextostr()
 */
static void type_bcdhextostr(u_8 * out_str,u_8 * hex,int hex_num)
{
	int type;
	
	type = (*hex);
	type = type>>4;
	
	if( type==9 )
	{
		if( *(hex+1)==0 || *(hex+1)==0xff )
		{
			bcdhextostr(out_str,hex+1,hex_num-1);
		}
		else
		{
			strcpy(out_str,"00");
			bcdhextostr(out_str+2,hex+1,hex_num-1);
		}
	}
	else
	{
		bcdhextostr(out_str,hex+1,hex_num-1);
	}
}

