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
/*                 华为话单格式说明                         */
/* 华为二进制话单格式是相同的，不需要分成多种话单解析程序   */
// **********************************************************
// 名称                                  长度      位置
// **********************************************************
// 流水号                                4         1-4       
// 模块内流水号                          4         5-8       
// 模块号                                1         9         
// 话单类型                              1         10        
// 通话起始时间                          7         11-17     
// 主叫号码                              14        18-31     
// 被叫号码                              14        32-45     
// 第三方号码                            14        46-59     
// 被叫漫游号码                          8         60-67     
// 被计费用户MSISDN                      8         68-75     
// 被计费移动用户IMSI                    8         76-83     
// 被计费移动用户IMEI                    8         84-91     
// 入中继群号                            2         92-93     
// 出中继群号                            2         94-95     
// 通话时长                              4         96-99     
// 首话单索引号                          4         100-103   
// 中间话单序列号                        1         104       
// 记录类型                              3b        105       
// 用户类别                              5b        105       
// 通话终止原因                          1         106       
// 产生中间话单原因                      1         107       
// 本局MSC号                             8         108-115   
// 被计费用户当前所在MSC号               8         116-123   
// 主叫用户当前位置区                    2         124-125   
// 主叫用户当前小区                      2         126-127   
// 主叫用户初始位置区                    2         128-129   
// 主叫用户初始小区                      2         130-131   
// 被叫用户当前位置区                    2         132-133   
// 被叫用户当前小区                      2         134-135   
// 被叫用户初始位置区                    2         136-137   
// 被叫用户初始小区                      2         138-139   
// 被计费移动用户呼叫参考                1         140       
// 传输模式                              4b        141       
// 电话业务或承载业务标志                4b        141       
// 承载能力                              1         142       
// 电话业务或承载业务码                  1         143       
// 业务类别                              1         144       
// 补充业务码1                           1         145        
// 补充业务码2                           1         146         
// 补充业务码3                           1         147         
// 补充业务码4                           1         148         
// 被计费移动用户初始CLASSMARK           3         149-151   
// 被计费移动用户当前CLASSMARK           3         152-154   
// 透明非透明指示                        2b        155       
// 是否使用DTMF                          2b        155       
// 计费免费标志                          4b        155       
// 漫游标志                              1b        156       
// 热计费标志                            1b        156       
// 操作结果                              6b        156       
// 费率指示                              2         157-158   
// 费率                                  2         159-160   
// 附加费                                2         161-162   
// 该次呼叫占用的B信道数                 1         163       
// 字节数                                4         164-167   
// 短消息中心地址                        8         168-175   
// 每个数据单元的字节数                  2         176-177   
// 业务键                                4         178-181   
// SCFID                                 5         182-186   
// 保留字节                              1         187       
// FCI信息                               40        188-227   
// 移动呼叫参考号                        8         228-235   
// 系统接入类型                          1         236
// 速率指示                              1         237
// 保留字节                              5         238-242
// channel mode                          4b        243
// channel                               4b        243
// MAPByPassInd标识                      1b        244
// Ro链路缺省呼叫处理                    2b        244
// voBB 用户标识                         1b        244
// 保留bit                               4b        244
// 增强用户类别                          1         245
// cARP值                                1         246
// 用户逻辑计费区编码                    3         247-249
// CMN标识                               1b        250
// 释放方                                1         251
// 费率码                                1         252
// 网络接续号码                          14        253-266
// 链路异常的时间戳                      7         267-273
// 最近一次成功CCR操作的时间戳           7         274-280
// 通话时间夏令时偏移                    1         281
// 链路异常的时间夏令时偏移              1         282
// 最近一次成功CCR操作的时间夏令时偏移   1         283
// 保留位                                17        284-300
// **********************************************************
typedef struct {
    char bill_sequence_in_msc[4];                  // 流水号
    char bill_sequence_in_module[4]                // 模块内流水号
    char module_no;                                // 模块号
    char cdr_type;                                 // 话单类型
    char charge_start_time[7];                     // 通话起始时间
    char calling_num[14];                          // 主叫号码
    char dialed_num[14];                           // 被叫号码
    char connected_num[14];                        // 第三方号码
    char msrn[8];                                  // 被叫漫游号码
    char served_msisdn[8];                         // 被计费用户MSISDN
    char served_imsi[8];                           // 被计费移动用户IMSI
    char served_imei[8];                           // 被计费移动用户IMEI
    char incoming_trunk_group_id[2];               // 入中继群号
    char outgoing_trunk_group_id[2];               // 出中继群号
    char charge_duration[4];                       // 通话时长（以秒为单位）
    char index_of_first_cdr[4];                    // 首话单索引号
    char sequence_of_intermediate_cdr;             // 中间话单序列号
    char record_user_type;                         // 记录类型:用户类别
    char cause_for_call_termination;               // 通话终止原因
    char cause_for_intermediate_record;            // 产生中间话单原因
    char local_msc_id[8];                          // 本局MSC号
    char peer_msc_id[8];                           // 被计费用户当前所在MSC号
    char current_lac_of_caller[2];                 // 主叫用户当前位置区
    char current_ci_of_caller[2];                  // 主叫用户当前小区
    char initial_lac_of_caller[2];                 // 主叫用户初始位置区
    char initial_ci_of_caller[2];                  // 主叫用户初始小区
    char current_lac_of_called[2];                 // 被叫用户当前位置区
    char current_ci_of_called[2];                  // 被叫用户当前小区
    char initial_lac_of_called[2];                 // 被叫用户初始位置区
    char initial_ci_of_called[2];                  // 被叫用户初始小区
    char call_reference;                           // 被计费移动用户呼叫参考
    char transmission_mode_tbs_flag;               // 传输模式:电话业务或承载业务标志
    char bearer_capability;                        // 承载能力
    char service_code_of_tbs;                      // 电话业务或承载业务码
    char gsm_gsvn;                                 // 业务类别
    char ss_code1;                                 // 补充业务码1
    char ss_code2;                                 // 补充业务码2
    char ss_code3;                                 // 补充业务码3
    char ss_code4;                                 // 补充业务码4
    char initial_classmark_of_served_ms[3];        // 被计费移动用户初始CLASSMARK
    char current_classmark_of_served_ms[3];        // 被计费移动用户当前CLASSMARK
    char transparency_dtmf__free_indicator;        // 透明非透明指示:是否使用DTMF:计费免费标志
    char roam_hotbill_flag_and_action_result;      // 漫游标志:热计费标志:操作结果
    char charging_case[2];                         // 费率指示
    char money_per_count[2];                       // 费率（以人民币的分为单位）
    char add_fee[2];                               // 附加费（以人民币的分为单位）
    char number_of_b_channels_occupied;            // 该次呼叫占用的B信道数
    char number_of_bytes[4];                       // 发送或接受短消息字节数
    char smsc_address[8];                          // 短消息中心地址
    char bytes_per_packet[2];                      // 每个数据单元的字节数
    char servicekey[4];                            // 业务键
    char scfid[5];                                 // SCFID
    char reserve1;                                 // 保留字节
    char fcidata[40];                              // FCI信息
    char call_reference_num[8];                    // 移动呼叫参考号
    char system_type;                              // 系统接入类型
    char rate_indication;                          // 速率指示
    char reserve2[5];                              // 保留字节
    char channel_mode_and_channel;                 // channel mode; channel
    char mapbypassind_ro_link_default_call_vobb_user_flag;    // MAPByPassInd标识; Ro链路缺省呼叫处理; voBB 用户标识 
    char e_category;                               // 增强用户类型
    char carp;                                     // cARP值
    char charge_area_code[3];                      // 用户逻辑计费区编码
    char cmn_flag;                                 // CMN标识 
    char disconnect_party;                         // 释放方
    char tariff_code;                              // 费率码
    char translated_number[14];                    // 网络接续号码
    char link_exception_time[7];                   // 链路异常的时间戳
    char latest_success_ccr_operation_time[7];     // 最近一次成功CCR操作的时间戳
    char dst_offset;                               // 通话时间夏令时偏移
    char link_exception_dst_offset;                // 链路异常的时间夏令时偏移
    char latest_success_ccr_operation_dst_offset;  // 最近一次成功CCR操作的夏令时偏移
    char reserve3[17];                             // 保留字节
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

	/*每次读100条话单, 每条300字节定长*/
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

				/* ******************** 提取话单记录 **************************************/
				memset(&cdr, 0, sizeof(cdr));
                                memcpy(&cdr, &buff[i][0], sizeof(cdr));
	        		
				/* ********************* 输出话单记录到文件 *******************************/
                                // 流水号                                4         1-4       
                                memset(&u32_num, 0, sizeof(u32_num));
                                memcpy(&u32_num, &(cdr.bill_sequence_in_msc[0]), sizeof(u32_num));
                                p_32_swap(&u32_num);
	                        fprintf(fp_o, "%d,", u32_num);

                                // 模块内流水号                          4         5-8       
                                memset(&u32_num, 0, sizeof(u32_num));
                                memcpy(&u32_num, &(cdr.bill_sequence_in_module[0]), sizeof(u32_num));
                                p_32_swap(&u32_num);
	                        fprintf(fp_o, "%d,", u32_num);

                                // 模块号                                1         9         
                                u32_num = cdr.module_no;
                                fprintf(fp_o, "%d,", u32_num);

                                // 话单类型                              1         10        
                                u32_num = cdr.cdr_type;
                                fprintf(fp_o, "%d,", u32_num);

                                // 通话起始时间                          7         11-17     
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

                                // 主叫号码                              14        18-31     
                                // 被叫号码                              14        32-45     
                                // 第三方号码                            14        46-59     
                                // 被叫漫游号码                          8         60-67     
                                // 被计费用户MSISDN                      8         68-75     
                                // 被计费移动用户IMSI                    8         76-83     
                                // 被计费移动用户IMEI                    8         84-91     
                                // 入中继群号                            2         92-93     
                                // 出中继群号                            2         94-95     
                                // 通话时长                              4         96-99     
                                // 首话单索引号                          4         100-103   
                                // 中间话单序列号                        1         104       
                                // 记录类型                              3b        105       
                                // 用户类别                              5b        105       
                                // 通话终止原因                          1         106       
                                // 产生中间话单原因                      1         107       
                                // 本局MSC号                             8         108-115   
                                // 被计费用户当前所在MSC号               8         116-123   
                                // 主叫用户当前位置区                    2         124-125   
                                // 主叫用户当前小区                      2         126-127   
                                // 主叫用户初始位置区                    2         128-129   
                                // 主叫用户初始小区                      2         130-131   
                                // 被叫用户当前位置区                    2         132-133   
                                // 被叫用户当前小区                      2         134-135   
                                // 被叫用户初始位置区                    2         136-137   
                                // 被叫用户初始小区                      2         138-139   
                                // 被计费移动用户呼叫参考                1         140       
                                // 传输模式                              4b        141       
                                // 电话业务或承载业务标志                4b        141       
                                // 承载能力                              1         142       
                                // 电话业务或承载业务码                  1         143       
                                // 业务类别                              1         144       
                                // 补充业务码1                           1         145        
                                // 补充业务码2                           1         146         
                                // 补充业务码3                           1         147         
                                // 补充业务码4                           1         148         
                                // 被计费移动用户初始CLASSMARK           3         149-151   
                                // 被计费移动用户当前CLASSMARK           3         152-154   
                                // 透明非透明指示                        2b        155       
                                // 是否使用DTMF                          2b        155       
                                // 计费免费标志                          4b        155       
                                // 漫游标志                              1b        156       
                                // 热计费标志                            1b        156       
                                // 操作结果                              6b        156       
                                // 费率指示                              2         157-158   
                                // 费率                                  2         159-160   
                                // 附加费                                2         161-162   
                                // 该次呼叫占用的B信道数                 1         163       
                                // 字节数                                4         164-167   
                                // 短消息中心地址                        8         168-175   
                                // 每个数据单元的字节数                  2         176-177   
                                // 业务键                                4         178-181   
                                // SCFID                                 5         182-186   
                                // 保留字节                              1         187       
                                // FCI信息                               40        188-227   
                                // 移动呼叫参考号                        8         228-235   
                                // 系统接入类型                          1         236
                                // 速率指示                              1         237
                                // 保留字节                              5         238-242
                                // channel mode                          4b        243
                                // channel                               4b        243
                                // MAPByPassInd标识                      1b        244
                                // Ro链路缺省呼叫处理                    2b        244
                                // voBB 用户标识                         1b        244
                                // 保留bit                               4b        244
                                // 增强用户类别                          1         245
                                // cARP值                                1         246
                                // 用户逻辑计费区编码                    3         247-249
                                // CMN标识                               1b        250
                                // 释放方                                1         251
                                // 费率码                                1         252
                                // 网络接续号码                          14        253-266
                                // 链路异常的时间戳                      7         267-273
                                // 最近一次成功CCR操作的时间戳           7         274-280
                                // 通话时间夏令时偏移                    1         281
                                // 链路异常的时间夏令时偏移              1         282
                                // 最近一次成功CCR操作的时间夏令时偏移   1         283
                                // 保留位                                17        284-300
                                
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

