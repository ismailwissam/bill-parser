/**********************************************************************************/
/* huawei.c                                                                       */
/*                                                                                */
/* 华为MSOFTX3000关口局二进制话单解析模块                                         */
/*                                                                                */
/* create by wangxiaohui at 2010.4.30                                             */
/*                                                                                */
/* modify by wangxiaohui at 2010.5.15                                             */
/*        本次修改主要是调整接口参数，接口的第二个参数原本是输入参数，            */
/*        提供解析后的CSV文件名；现在变更为输入输出参数，输入时提供保存           */
/*        CSV文件的路径，输出时把所有生成的CSV文件名追加在路径后面返回，          */
/*        装配格式为：CSV文件保存路径:CSV文件名1;CSV文件名2[;...]                 */
/*                                                                                */
/* modify by wangxioahui at 2010.5.19                                             */
/*        本次修改主要是为了适应新的文件命名格式，原先话单的生成时间是            */
/*        从话单内容中解析出来的，现在改为直接在Ftp下载文件时就获取文件           */
/*        的生成时间，并把这个时间拼装在文件名中，现在该解析模块读取的            */
/*        话单文件格式为：                                                        */
/*        采集点编号_厂家_端局_话单生成时间(YYYYMMDDhhmmss)_原文件名              */
/*        实例：000001_hw_hzgs2_20100520123522_xxxxxx.dat                         */
/*        而生成的CSV文件是在原先文件名的基础上增加话单类型标示，输出的           */
/*        CSV文件格式为：                                                         */
/*        采集点编号_厂家_端局_话单类型_话单生成时间(YYYYMMDDhhmmss)_原文件名.csv */
/*        实例：000002_hw_gzgs4_moc_20100520123522_xxxxxx.csv                     */
/*                                                                                */
/* modify by wangxiaohui at 2010.5.22                                             */
/*        本次修改是主要是在输出的csv文件中增加一列数据：网元名称(NE_NAME),因为在 */
/*        目标数据库中表的第一列都是网元名称(NE_NAME),为了使用sqlldr加载数据时可以*/
/*        正确加载，现在增加这一列输出数据，放在第一列                            */
/**********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>

/*-----------------------------------------------------------------------------------*/
/* micro difinition */
#define      ERR_LOG_FILE        "./log/pretreat_huawei_err"
#define      MAX_FILENAME        256
#define      MAX_BUFFER          64
#define      TRUE                1
#define      FALSE               0

//-------------------------------------------------------------------------------------
/* type difinition */
typedef unsigned char  u_8;
typedef unsigned short u_16;
typedef unsigned int   u_32;
typedef unsigned char  bool;

//-------------------------------------------------------------------------------------
/* constant difinition */
const char* BCDSTR = "0123456789ABCDEF";

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
typedef struct _tag_t_cdr_raw {
    char bill_sequence_in_msc[4];                  // 流水号
    char bill_sequence_in_module[4];               // 模块内流水号
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
} t_cdr_raw;

typedef struct _tag_t_cdr {
    u_32 bill_sequence_in_msc;                     // 流水号
    u_32 bill_sequence_in_module;                  // 模块内流水号
    u_8 module_no;                                 // 模块号
    u_8 cdr_type;                                  // 话单类型
    u_16 charge_start_time_year;                   // 通话起始时间 - 年
    u_8 charge_start_time_month;                   // 通话起始时间 - 月
    u_8 charge_start_time_day;                     // 通话起始时间 - 日
    u_8 charge_start_time_hour;                    // 通话起始时间 - 时
    u_8 charge_start_time_minute;                  // 通话起始时间 - 分
    u_8 charge_start_time_second;                  // 通话起始时间 - 秒
    u_8 calling_num_plan;                          // 主叫号码计划
    u_8 calling_num_type;                          // 主叫号码类型
    u_8 calling_num_length;                        // 主叫号码长度
    char calling_num[25];                          // 主叫号码
    u_8 dialed_num_plan;                           // 被叫号码计划
    u_8 dialed_num_type;                           // 被叫号码类型
    u_8 dialed_num_length;                         // 被叫号码长度
    char dialed_num[25];                           // 被叫号码
    u_8 connected_num_plan;                        // 第三方号码计划
    u_8 connected_num_type;                        // 第三方号码类型
    u_8 connected_num_length;                      // 第三方号码长度
    char connected_num[25];                        // 第三方号码
    char msrn[17];                                 // 被叫漫游号码
    char served_msisdn[17];                        // 被计费用户MSISDN
    char served_imsi[17];                          // 被计费移动用户IMSI
    char served_imei[17];                          // 被计费移动用户IMEI
    u_16 incoming_trunk_group_id;                  // 入中继群号
    u_16 outgoing_trunk_group_id;                  // 出中继群号
    u_32 charge_duration;                          // 通话时长（以秒为单位）
    u_32 index_of_first_cdr;                       // 首话单索引号
    u_8 sequence_of_intermediate_cdr;              // 中间话单序列号
    u_8 record_type;                               // 记录类型
    u_8 user_type;                                 // 用户类型
    u_8 cause_for_call_termination;                // 通话终止原因
    u_8 cause_for_intermediate_record;             // 产生中间话单原因
    char local_msc_id[17];                         // 本局MSC号
    char peer_msc_id[17];                          // 被计费用户当前所在MSC号
    char current_lac_of_caller[5];                 // 主叫用户当前位置区
    char current_ci_of_caller[5];                  // 主叫用户当前小区
    char initial_lac_of_caller[5];                 // 主叫用户初始位置区
    char initial_ci_of_caller[5];                  // 主叫用户初始小区
    char current_lac_of_called[5];                 // 被叫用户当前位置区
    char current_ci_of_called[5];                  // 被叫用户当前小区
    char initial_lac_of_called[5];                 // 被叫用户初始位置区
    char initial_ci_of_called[5];                  // 被叫用户初始小区
    u_8 call_reference;                            // 被计费移动用户呼叫参考
    u_8 transmission_mode;                         // 传输模式
    u_8 tbs_flag;                                  // 电话业务或承载业务标志
    u_8 bearer_capability;                         // 承载能力
    u_8 service_code_of_tbs;                       // 电话业务或承载业务码
    u_8 gsm_gsvn;                                  // 业务类别
    u_8 ss_code1;                                  // 补充业务码1
    u_8 ss_code2;                                  // 补充业务码2
    u_8 ss_code3;                                  // 补充业务码3
    u_8 ss_code4;                                  // 补充业务码4
    u_8 initial_classmark_rfc;                     // 被计费移动用户初始CLASSMARK的RFC值
    u_8 initial_classmark_a5_1;                    // 被计费移动用户初始CLASSMARK是否支持A5_1算法
    u_8 initial_classmark_es_ind;                  // 被计费移动用户初始CLASSMARK是否支持早发送
    u_8 initial_classmark_rev_level;               // 被计费移动用户初始CLASSMARK版本号
    u_8 initial_classmark_fc;                      // 被计费移动用户初始CLASSMARK是否支持FC
    u_8 initial_classmark_vbs;                     // 被计费移动用户初始CLASSMARK是否需要VBS指示
    u_8 initial_classmark_vgcs;                    // 被计费移动用户初始CLASSMARK是否需要VGCS指示
    u_8 initial_classmark_smc;                     // 被计费移动用户初始CLASSMARK是否支持点到点短信接收
    u_8 initial_classmark_ss_ind;                  // 被计费移动用户初始CLASSMARK屏蔽指示语
    u_8 initial_classmark_ps_cap;                  // 被计费移动用户初始CLASSMARK是否提供PS
    u_8 initial_classmark_a5_2;                    // 被计费移动用户初始CLASSMARK是否支持A5_2算法
    u_8 initial_classmark_a5_3;                    // 被计费移动用户初始CLASSMARK是否支持A5_3算法
    u_8 initial_classmark_cm_3;                    // 被计费移动用户初始CLASSMARK是否支持CM_3
    u_8 current_classmark_rfc;                     // 被计费移动用户当前CLASSMARK的RFC值
    u_8 current_classmark_a5_1;                    // 被计费移动用户当前CLASSMARK是否支持A5_1算法
    u_8 current_classmark_es_ind;                  // 被计费移动用户当前CLASSMARK是否支持早发送
    u_8 current_classmark_rev_level;               // 被计费移动用户当前CLASSMARK版本号
    u_8 current_classmark_fc;                      // 被计费移动用户当前CLASSMARK是否支持FC
    u_8 current_classmark_vbs;                     // 被计费移动用户当前CLASSMARK是否需要VBS指示
    u_8 current_classmark_vgcs;                    // 被计费移动用户当前CLASSMARK是否需要VGCS指示
    u_8 current_classmark_smc;                     // 被计费移动用户当前CLASSMARK是否支持点到点短信接收
    u_8 current_classmark_ss_ind;                  // 被计费移动用户当前CLASSMARK屏蔽指示语
    u_8 current_classmark_ps_cap;                  // 被计费移动用户当前CLASSMARK是否提供PS
    u_8 current_classmark_a5_2;                    // 被计费移动用户当前CLASSMARK是否支持A5_2算法
    u_8 current_classmark_a5_3;                    // 被计费移动用户当前CLASSMARK是否支持A5_3算法
    u_8 current_classmark_cm_3;                    // 被计费移动用户当前CLASSMARK是否支持CM_3
    u_8 transparency_indicator;                    // 透明非透明指示
    u_8 dtmf_flag;                                 // 是否使用DTMF
    u_8 free_indicator;                            // 计费免费指示
    u_8 roam_flag;                                 // 漫游用户标志
    u_8 hot_bill_flag;                             // 热计费标志
    u_8 action_result;                             // 操作结果
    u_16 charging_case;                            // 费率指示
    u_16 money_per_count;                          // 费率（以人民币的分为单位）
    u_16 add_fee;                                  // 附加费（以人民币的分为单位）
    u_8 number_of_b_channels_occupied;             // 该次呼叫占用的B信道数
    u_32 number_of_bytes;                          // 发送或接受短消息字节数
    char smsc_address[17];                         // 短消息中心地址
    u_16 bytes_per_packet;                         // 每个数据单元的字节数
    char servicekey[4];                            // 业务键
    char scfid[5];                                 // SCFID
    char fcidata[40];                              // FCI信息
    char call_reference_num[8];                    // 移动呼叫参考号
    u_8 system_type;                               // 系统接入类型
    u_8 rate_indication;                           // 速率指示
    u_8 channel_mode;                              // channel mode
    u_8 channel;                                   // channel
    u_8 map_by_pass_ind;                           // MAPByPassInd标识
    u_8 ro_link_default_call_proccess;             // Ro链路缺省呼叫处理
    u_8 vobb_user_flag;                            // voBB 用户标识 
    u_8 e_category;                                // 增强用户类型
    u_8 carp;                                      // cARP值
    char charge_area_code[3];                      // 用户逻辑计费区编码
    u_8 cmn_flag;                                  // CMN标识 
    u_8 disconnect_party;                          // 释放方
    u_8 tariff_code;                               // 费率码
    u_8 translated_number_plan;                    // 网络接续号码计划
    u_8 translated_number_type;                    // 网络接续号码类型
    u_8 translated_number_length;                  // 网络接续号码长度
    char translated_number[25];                    // 网络接续号码
    u_16 link_exception_time_year;                 // 链路异常的时间戳 - 年
    u_8 link_exception_time_month;                 // 链路异常的时间戳 - 月
    u_8 link_exception_time_day;                   // 链路异常的时间戳 - 日
    u_8 link_exception_time_hour;                  // 链路异常的时间戳 - 时
    u_8 link_exception_time_minute;                // 链路异常的时间戳 - 分
    u_8 link_exception_time_second;                // 链路异常的时间戳- 秒
    u_16 latest_success_ccr_operation_time_year;   // 最近一次成功CCR操作的时间戳 - 年
    u_8 latest_success_ccr_operation_time_month;   // 最近一次成功CCR操作的时间戳 - 月
    u_8 latest_success_ccr_operation_time_day;     // 最近一次成功CCR操作的时间戳 - 日
    u_8 latest_success_ccr_operation_time_hour;    // 最近一次成功CCR操作的时间戳 - 时
    u_8 latest_success_ccr_operation_time_minute;  // 最近一次成功CCR操作的时间戳 - 分
    u_8 latest_success_ccr_operation_time_second;  // 最近一次成功CCR操作的时间戳 - 秒
    u_8 dst_offset;                                // 通话时间夏令时偏移
    u_8 link_exception_dst_offset;                 // 链路异常的时间夏令时偏移
    u_8 latest_success_ccr_operation_dst_offset;   // 最近一次成功CCR操作的夏令时偏移
} t_cdr;

//------------------------------------------------------------------------------------------------
/* internal function difinition */
static int err_log(const char * format,...);
static void p_32_swap(u_32 *p_a);
static void p_16_swap(u_16 *p_a);
static void hextostr(char * out_str, const char * hex,int hex_num);
static bool bcdhextostr(bool is_zipbcd, bool is_bigend, bool is_all_num, const char* data, int len, char * out_str);
static int get_commit_file_name(const char * original_file_name, const char * bill_type, char * ret_file_name);
static int get_ne_name(const char * original_file_name, char * ret_ne_name);

/* module interface */
int huawei(char * in_file_name, char * out_file_name, int * rec_num);

/*
 *huawei()
 *          return 0 success, 1 fail
 *
 */
int huawei(char * in_file_name, char * out_file_name, int * rec_num)
{
	int                    ret;

	FILE                   *fp_i = NULL;
	FILE                   *fp_o = NULL;

	t_cdr_raw              cdr_raw;
    t_cdr                  cdr;

	/* max length of each cdr bill is 300 bytes, but actual may not be. */
    const int              cdr_max_len = 300;
	char                   cdr_buf[cdr_max_len];     
    u_32                   cdr_len;
    u_32                   read_cdr_len;

    char                   tmp_buf[MAX_BUFFER];
    u_32                   mem_len, num_len, i;
	u_32                   serial_num;

    char                   csv_file_name[MAX_FILENAME];
    char                   tmp_file_name[MAX_FILENAME];

    char                   ne_name[MAX_BUFFER];
    bool                   first_bill;

	ret          = 0;
	serial_num   = 0;
	(*rec_num)   = 0;

    /* get ne name */
    memset(ne_name, 0, sizeof(ne_name));
    if(get_ne_name(in_file_name, ne_name) != 0)
    {
        err_log("huawei: get ne name fail\n");
        ret = 1;
        goto Exit_Pro;
    }

    /* open the input file for read */
	fp_i = fopen(in_file_name, "r");
	if(fp_i == NULL)
	{
        err_log("huawei: fopen %s fail\n",in_file_name);
        ret = 1;
        goto Exit_Pro;
	}

    /* skip the file header */
    if(fseek(fp_i, 32, SEEK_SET) != 0)
    {
        err_log("huawei: seek % failed.\n", in_file_name);
        ret = 1;
        goto Exit_Pro;
    }

    first_bill = TRUE;

	while(1)
	{
        /* read cdr length from in file */
        if(fread(&cdr_len, 1, sizeof(cdr_len), fp_i) < sizeof(cdr_len))
        {
            if(ferror(fp_i))
            {
                err_log("huawei: fread %s error!\n",in_file_name);
                ret = 1;
                goto Exit_Pro;
            }
            break;
        }
        p_32_swap(&cdr_len);

        /* if the cdr length is not 187, it is not a valid cdr record.
         * may be the file tail, exit the parse process.
         * */
        if (cdr_len != 187)
        {
            break;
        }

        /* read cdr data from in file */
		memset(cdr_buf, 0, sizeof(cdr_buf));
		read_cdr_len = fread(cdr_buf, 1, cdr_len, fp_i);
		
        if(feof(fp_i))
        {
            break;
        }

        if(ferror(fp_i))
        {
            err_log("huawei: fread %s error!\n",in_file_name);
            ret = 1;
            goto Exit_Pro;
        }

		if(read_cdr_len < cdr_len)
		{
            err_log("huawei: %s exception!\n",in_file_name);
            ret = 1;
			break;
		}

        /* parse the cdr data */
		if(read_cdr_len == cdr_len)
		{
            serial_num++;

            /* ******************** 提取话单记录 **************************************/
            memset(&cdr_raw, 0, sizeof(cdr_raw));
            memcpy(&cdr_raw, &cdr_buf[0], cdr_len);
                
            /* ******************** 话单记录预处理 **************************************/
            memset(&cdr, 0, sizeof(cdr));

            // 流水号                                4         1-4       
            memcpy(&(cdr.bill_sequence_in_msc), 
                   &(cdr_raw.bill_sequence_in_msc[0]), 
                   sizeof(cdr.bill_sequence_in_msc));
            p_32_swap(&(cdr.bill_sequence_in_msc));

            // 模块内流水号                          4         5-8       
            memcpy(&(cdr.bill_sequence_in_module), 
                   &(cdr_raw.bill_sequence_in_module[0]), 
                   sizeof(cdr.bill_sequence_in_module));
            p_32_swap(&(cdr.bill_sequence_in_module));

            // 模块号                                1         9         
            cdr.module_no = cdr_raw.module_no;

            // 话单类型                              1         10        
            cdr.cdr_type = cdr_raw.cdr_type;

            // 通话起始时间                          7         11-17     
            memcpy(&(cdr.charge_start_time_year), &(cdr_raw.charge_start_time[0]), 2);
            p_16_swap(&(cdr.charge_start_time_year));
            cdr.charge_start_time_month = cdr_raw.charge_start_time[2];
            cdr.charge_start_time_day = cdr_raw.charge_start_time[3];
            cdr.charge_start_time_hour = cdr_raw.charge_start_time[4];
            cdr.charge_start_time_minute = cdr_raw.charge_start_time[5];
            cdr.charge_start_time_second = cdr_raw.charge_start_time[6];

            if(first_bill)
            {
                /* generate the temp csv file name */
                memset(csv_file_name, 0, sizeof(csv_file_name));
                if(get_commit_file_name(in_file_name, "bi", csv_file_name) != 0)
                {
                    err_log("huawei: generate output csv file fail\n");
                    ret = 1;
                    goto Exit_Pro;
                }

                /* open the csv file for write */
                sprintf(tmp_file_name, "%s/%s", out_file_name, csv_file_name);
                fp_o = fopen(tmp_file_name, "w+");
                if(fp_o == NULL)
                {
                    err_log("huawei: fopen %s fail\n",tmp_file_name);
                    ret = 1;
                    goto Exit_Pro;
                }

                first_bill = FALSE;
            }

            // 主叫号码                              14        18-31     
            /* 号码计划 */
            cdr.calling_num_plan = cdr_raw.calling_num[0] & 0x0F;

            /* 号码类型 */
            cdr.calling_num_type = (cdr_raw.calling_num[0] >> 4) & 0x07;

            /* 号码长度 */
            cdr.calling_num_length = cdr_raw.calling_num[1] & 0x1F;

            /* 主叫号码 */
            mem_len = 12;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.calling_num[2]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf + i) = '\0';
                    break;
                }
            }
            strcpy(cdr.calling_num, tmp_buf);

            // 被叫号码                              14        32-45     
            /* 号码计划 */
            cdr.dialed_num_plan = cdr_raw.dialed_num[0] & 0x0F;

            /* 号码类型 */
            cdr.dialed_num_type = (cdr_raw.dialed_num[0] >> 4) & 0x07;

            /* 号码长度 */
            cdr.dialed_num_length = cdr_raw.dialed_num[1] & 0x1F;

            /* 被叫号码 */
            mem_len = 12;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.dialed_num[2]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.dialed_num, tmp_buf);

            // 第三方号码                            14        46-59     
            /* 号码计划 */
            cdr.connected_num_plan = cdr_raw.connected_num[0] & 0x0F;

            /* 号码类型 */
            cdr.connected_num_type = (cdr_raw.connected_num[0] >> 4) & 0x07;

            /* 号码长度 */
            cdr.connected_num_length = cdr_raw.connected_num[1] & 0x1F;

            /* 第三方号码 */
            mem_len = 12;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.connected_num[2]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.connected_num, tmp_buf);
       
            // 被叫漫游号码                          8         60-67     
            mem_len = 8;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.msrn[0]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.msrn, tmp_buf);
            
            // 被计费用户MSISDN                      8         68-75     
            mem_len = 8;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.served_msisdn[0]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.served_msisdn, tmp_buf);

            // 被计费移动用户IMSI                    8         76-83     
            mem_len = 8;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.served_imsi[0]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.served_imsi, tmp_buf);
            
            // 被计费移动用户IMEI                    8         84-91     
            mem_len = 8;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.served_imei[0]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.served_imei, tmp_buf);
            
            // 入中继群号                            2         92-93     
            memcpy(&(cdr.incoming_trunk_group_id), &(cdr_raw.incoming_trunk_group_id[0]), 2);
            p_16_swap(&(cdr.incoming_trunk_group_id));

            // 出中继群号                            2         94-95     
            memcpy(&(cdr.outgoing_trunk_group_id), &(cdr_raw.outgoing_trunk_group_id[0]), 2);
            p_16_swap(&(cdr.outgoing_trunk_group_id));
            
            // 通话时长                              4         96-99     
            memcpy(&(cdr.charge_duration), &(cdr_raw.charge_duration[0]), sizeof(cdr.charge_duration));
            p_32_swap(&(cdr.charge_duration));
            
            // 首话单索引号                          4         100-103   
            memcpy(&(cdr.index_of_first_cdr), &(cdr_raw.index_of_first_cdr[0]), sizeof(cdr.index_of_first_cdr));
            p_32_swap(&(cdr.index_of_first_cdr));
            
            // 中间话单序列号                        1         104       
            cdr.sequence_of_intermediate_cdr = cdr_raw.sequence_of_intermediate_cdr;
            
            // 记录类型                              3b        105       
            cdr.record_type = cdr_raw.record_user_type & 0x07;
            
            // 用户类别                              5b        105       
            cdr.user_type = (cdr_raw.record_user_type >> 3) & 0x1F;
            
            // 通话终止原因                          1         106       
            cdr.cause_for_call_termination = cdr_raw.cause_for_call_termination;
            
            // 产生中间话单原因                      1         107       
            cdr.cause_for_intermediate_record = cdr_raw.cause_for_intermediate_record;
           
            // 本局MSC号                             8         108-115   
            mem_len = 8;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.local_msc_id[0]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.local_msc_id, tmp_buf);
            
            // 被计费用户当前所在MSC号               8         116-123   
            mem_len = 8;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.peer_msc_id[0]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.peer_msc_id, tmp_buf);
            
            // 主叫用户当前位置区                    2         124-125   
            mem_len = 2;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.current_lac_of_caller[0]), mem_len, tmp_buf);
            strcpy(cdr.current_lac_of_caller, tmp_buf);
            
            // 主叫用户当前小区                      2         126-127   
            mem_len = 2;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.current_ci_of_caller[0]), mem_len, tmp_buf);
            strcpy(cdr.current_ci_of_caller, tmp_buf);
            
            // 主叫用户初始位置区                    2         128-129   
            mem_len = 2;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.initial_lac_of_caller[0]), mem_len, tmp_buf);
            strcpy(cdr.initial_lac_of_caller, tmp_buf);
            
            // 主叫用户初始小区                      2         130-131   
            mem_len = 2;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.initial_ci_of_caller[0]), mem_len, tmp_buf);
            strcpy(cdr.initial_ci_of_caller, tmp_buf);
            
            // 被叫用户当前位置区                    2         132-133   
            mem_len = 2;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.current_lac_of_called[0]), mem_len, tmp_buf);
            strcpy(cdr.current_lac_of_called, tmp_buf);
            
            // 被叫用户当前小区                      2         134-135   
            mem_len = 2;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.current_ci_of_called[0]), mem_len, tmp_buf);
            strcpy(cdr.current_ci_of_called, tmp_buf);
            
            // 被叫用户初始位置区                    2         136-137   
            mem_len = 2;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.initial_lac_of_called[0]), mem_len, tmp_buf);
            strcpy(cdr.initial_lac_of_called, tmp_buf);
            
            // 被叫用户初始小区                      2         138-139   
            mem_len = 2;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.initial_ci_of_called[0]), mem_len, tmp_buf);
            strcpy(cdr.initial_ci_of_called, tmp_buf);
            
            // 被计费移动用户呼叫参考                1         140       
            cdr.call_reference = cdr_raw.call_reference & 0x07;
            
            // 传输模式                              4b        141       
            cdr.transmission_mode = cdr_raw.transmission_mode_tbs_flag & 0x0F;
           
            // 电话业务或承载业务标志                4b        141       
            cdr.tbs_flag = (cdr_raw.transmission_mode_tbs_flag >> 4) & 0x0F;
          
            // 承载能力                              1         142       
            cdr.bearer_capability = cdr_raw.bearer_capability;
           
            // 电话业务或承载业务码                  1         143       
            cdr.service_code_of_tbs = cdr_raw.service_code_of_tbs;
           
            // 业务类别                              1         144       
            cdr.gsm_gsvn = cdr_raw.gsm_gsvn;
           
            // 补充业务码1                           1         145        
            cdr.ss_code1 = cdr_raw.ss_code1;
           
            // 补充业务码2                           1         146         
            cdr.ss_code2 = cdr_raw.ss_code2;
            
            // 补充业务码3                           1         147         
            cdr.ss_code3 = cdr_raw.ss_code3;
            
            // 补充业务码4                           1         148         
            cdr.ss_code4 = cdr_raw.ss_code4;
           
            // 被计费移动用户初始CLASSMARK           3         149-151   
            cdr.initial_classmark_rfc = cdr_raw.initial_classmark_of_served_ms[0] & 0x07;
            cdr.initial_classmark_a5_1 = (cdr_raw.initial_classmark_of_served_ms[0] & 0x08) >> 3;
            cdr.initial_classmark_es_ind = (cdr_raw.initial_classmark_of_served_ms[0] & 0x10) >> 4;
            cdr.initial_classmark_rev_level = (cdr_raw.initial_classmark_of_served_ms[0] &0xE0) >> 5;
            cdr.initial_classmark_fc = cdr_raw.initial_classmark_of_served_ms[1] & 0x01;
            cdr.initial_classmark_vgcs = (cdr_raw.initial_classmark_of_served_ms[1] & 0x02) >> 1;
            cdr.initial_classmark_vbs = (cdr_raw.initial_classmark_of_served_ms[1] & 0x04) >> 2;
            cdr.initial_classmark_smc = (cdr_raw.initial_classmark_of_served_ms[1] & 0x08) >> 3;
            cdr.initial_classmark_ss_ind = (cdr_raw.initial_classmark_of_served_ms[1] & 0x30) >> 4;
            cdr.initial_classmark_ps_cap = (cdr_raw.initial_classmark_of_served_ms[1] & 0x40) >> 6;
            cdr.initial_classmark_a5_2 = cdr_raw.initial_classmark_of_served_ms[2] & 0x01;
            cdr.initial_classmark_a5_3 = cdr_raw.initial_classmark_of_served_ms[2] & 0x02;
            cdr.initial_classmark_cm_3 = (cdr_raw.initial_classmark_of_served_ms[2] & 0x80) >> 7;
            
            // 被计费移动用户当前CLASSMARK           3         152-154   
            cdr.current_classmark_rfc = cdr_raw.current_classmark_of_served_ms[0] & 0x07;
            cdr.current_classmark_a5_1 = (cdr_raw.current_classmark_of_served_ms[0] & 0x08) >> 3;
            cdr.current_classmark_es_ind = (cdr_raw.current_classmark_of_served_ms[0] & 0x10) >> 4;
            cdr.current_classmark_rev_level = (cdr_raw.current_classmark_of_served_ms[0] &0xE0) >> 5;
            cdr.current_classmark_fc = cdr_raw.current_classmark_of_served_ms[1] & 0x01;
            cdr.current_classmark_vgcs = (cdr_raw.current_classmark_of_served_ms[1] & 0x02) >> 1;
            cdr.current_classmark_vbs = (cdr_raw.current_classmark_of_served_ms[1] & 0x04) >> 2;
            cdr.current_classmark_smc = (cdr_raw.current_classmark_of_served_ms[1] & 0x08) >> 3;
            cdr.current_classmark_ss_ind = (cdr_raw.current_classmark_of_served_ms[1] & 0x30) >> 4;
            cdr.current_classmark_ps_cap = (cdr_raw.current_classmark_of_served_ms[1] & 0x40) >> 6;
            cdr.current_classmark_a5_2 = cdr_raw.current_classmark_of_served_ms[2] & 0x01;
            cdr.current_classmark_a5_3 = cdr_raw.current_classmark_of_served_ms[2] & 0x02;
            cdr.current_classmark_cm_3 = (cdr_raw.current_classmark_of_served_ms[2] & 0x80) >> 7;
            
            // 透明非透明指示                        2b        155       
            cdr.transparency_indicator = cdr_raw.transparency_dtmf__free_indicator & 0x03;
            
            // 是否使用DTMF                          2b        155       
            cdr.dtmf_flag = (cdr_raw.transparency_dtmf__free_indicator & 0x0C) >> 2;
            
            // 计费免费标志                          4b        155       
            cdr.free_indicator = (cdr_raw.transparency_dtmf__free_indicator & 0xF0) >> 4;
            
            // 漫游标志                              1b        156       
            cdr.roam_flag = cdr_raw.roam_hotbill_flag_and_action_result & 0x01;
            
            // 热计费标志                            1b        156       
            cdr.hot_bill_flag = (cdr_raw.roam_hotbill_flag_and_action_result & 0x02) >> 1;
            
            // 操作结果                              6b        156       
            cdr.action_result = (cdr_raw.roam_hotbill_flag_and_action_result & 0xFC) >> 2;
            
            // 费率指示                              2         157-158   
            memcpy(&(cdr.charging_case), &(cdr_raw.charging_case[0]), sizeof(cdr.charging_case));
            p_16_swap(&(cdr.charging_case));
            
            // 费率                                  2         159-160   
            memcpy(&(cdr.money_per_count), &(cdr_raw.money_per_count[0]), sizeof(cdr.money_per_count));
            p_16_swap(&(cdr.money_per_count));
            
            // 附加费                                2         161-162   
            memcpy(&(cdr.add_fee), &(cdr_raw.add_fee[0]), sizeof(cdr.add_fee));
            p_16_swap(&(cdr.add_fee));
            
            // 该次呼叫占用的B信道数                 1         163       
            cdr.number_of_b_channels_occupied = cdr_raw.number_of_b_channels_occupied;
            
            // 字节数                                4         164-167   
            memcpy(&(cdr.number_of_bytes), &(cdr_raw.number_of_bytes[0]), sizeof(cdr.number_of_bytes));
            p_32_swap(&(cdr.number_of_bytes));
            
            // 短消息中心地址                        8         168-175   
            mem_len = 8;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.smsc_address[0]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.smsc_address, tmp_buf);

            // 每个数据单元的字节数                  2         176-177   
            memcpy(&(cdr.bytes_per_packet), &(cdr_raw.bytes_per_packet[0]), sizeof(cdr.bytes_per_packet));
            p_16_swap(&(cdr.bytes_per_packet));
            
            // 业务键                                4         178-181   
            memcpy(&(cdr.servicekey[0]), &(cdr_raw.servicekey[0]), sizeof(cdr.servicekey));
            
            // SCFID                                 5         182-186   
            memcpy(&(cdr.scfid[0]), &(cdr_raw.scfid[0]), sizeof(cdr.scfid));
            
            // 保留字节                              1         187       
            
            // FCI信息                               40        188-227   
            memcpy(&(cdr.fcidata[0]), &(cdr_raw.fcidata[0]), sizeof(cdr.fcidata));
            
            // 移动呼叫参考号                        8         228-235   
            memcpy(&(cdr.call_reference_num[0]), &(cdr_raw.call_reference_num[0]), sizeof(cdr.call_reference_num));
            
            // 系统接入类型                          1         236
            cdr.system_type = cdr_raw.system_type;
            
            // 速率指示                              1         237
            cdr.rate_indication = cdr_raw.rate_indication;
            
            // 保留字节                              5         238-242
            
            // channel mode                          4b        243
            cdr.channel_mode = cdr_raw.channel_mode_and_channel & 0x0F;
            
            // channel                               4b        243
            cdr.channel = (cdr_raw.channel_mode_and_channel & 0xF0) >> 4;
            
            // MAPByPassInd标识                      1b        244
            cdr.map_by_pass_ind = cdr_raw.mapbypassind_ro_link_default_call_vobb_user_flag & 0x01;

            // Ro链路缺省呼叫处理                    2b        244
            cdr.ro_link_default_call_proccess = (cdr_raw.mapbypassind_ro_link_default_call_vobb_user_flag & 0x06) >> 1;
            
            // voBB 用户标识                         1b        244
            cdr.vobb_user_flag = (cdr_raw.mapbypassind_ro_link_default_call_vobb_user_flag & 0x08) >> 3;
            
            // 保留bit                               4b        244
            
            // 增强用户类别                          1         245
            cdr.e_category = cdr_raw.e_category;
            
            // cARP值                                1         246
            cdr.carp = cdr_raw.carp;
            
            // 用户逻辑计费区编码                    3         247-249
            memcpy(&(cdr.charge_area_code[0]), &(cdr_raw.charge_area_code[0]), sizeof(cdr.charge_area_code));
            
            // CMN标识                               1b        250
            cdr.cmn_flag = cdr_raw.cmn_flag & 0x01;
            
            // 释放方                                1         251
            cdr.disconnect_party = cdr_raw.disconnect_party;

            // 费率码                                1         252
            cdr.tariff_code = cdr_raw.tariff_code;
            
            // 网络接续号码                          14        253-266
            /* 号码计划 */
            cdr.translated_number_plan = cdr_raw.translated_number[0] & 0x0F;

            /* 号码类型 */
            cdr.translated_number_type = (cdr_raw.translated_number[0] >> 4) & 0x07;

            /* 号码长度 */
            cdr.translated_number_length = cdr_raw.translated_number[1] & 0x1F;

            /* 网络接续号码 */
            mem_len = 12;
            num_len = mem_len * 2;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            bcdhextostr(TRUE, TRUE, TRUE, &(cdr_raw.translated_number[2]), mem_len, tmp_buf);
            for (i = 0; i < num_len; ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            strcpy(cdr.translated_number, tmp_buf);

            // 链路异常的时间戳                      7         267-273
            memcpy(&(cdr.link_exception_time_year), &(cdr_raw.link_exception_time[0]), 2);
            p_16_swap(&(cdr.link_exception_time_year));
            cdr.link_exception_time_month = cdr_raw.link_exception_time[2];
            cdr.link_exception_time_day = cdr_raw.link_exception_time[3];
            cdr.link_exception_time_hour = cdr_raw.link_exception_time[4];
            cdr.link_exception_time_minute = cdr_raw.link_exception_time[5];
            cdr.link_exception_time_second = cdr_raw.link_exception_time[6];

            // 最近一次成功CCR操作的时间戳           7         274-280
            memcpy(&(cdr.latest_success_ccr_operation_time_year), &(cdr_raw.latest_success_ccr_operation_time[0]), 2);
            p_16_swap(&(cdr.latest_success_ccr_operation_time_year));
            cdr.latest_success_ccr_operation_time_month = cdr_raw.latest_success_ccr_operation_time[2];
            cdr.latest_success_ccr_operation_time_day = cdr_raw.latest_success_ccr_operation_time[3];
            cdr.latest_success_ccr_operation_time_hour = cdr_raw.latest_success_ccr_operation_time[4];
            cdr.latest_success_ccr_operation_time_minute = cdr_raw.latest_success_ccr_operation_time[5];
            cdr.latest_success_ccr_operation_time_second = cdr_raw.latest_success_ccr_operation_time[6];
           
            // 通话时间夏令时偏移                    1         281
            cdr.dst_offset = cdr_raw.dst_offset;
            
            // 链路异常的时间夏令时偏移              1         282
            cdr.link_exception_dst_offset = cdr_raw.link_exception_dst_offset;
            
            // 最近一次成功CCR操作的时间夏令时偏移   1         283
            cdr.latest_success_ccr_operation_dst_offset = cdr_raw.latest_success_ccr_operation_dst_offset;

            // 保留位                                17        284-300

            /* ********************* 输出话单记录到文件 *******************************/
            //  网元名称
            fprintf(fp_o, "%s,", ne_name);

            // 流水号
            fprintf(fp_o, "%d,", cdr.bill_sequence_in_msc);

            // 模块内流水号
            fprintf(fp_o, "%d,", cdr.bill_sequence_in_module);

            // 模块号
            fprintf(fp_o, "%d,", cdr.module_no);

            // 话单类型
            fprintf(fp_o, "%d,", cdr.cdr_type);
        
            // 通话起始时间
            fprintf(fp_o, "'%d-%d-%d %d:%d:%d',", 
                    cdr.charge_start_time_year,
                    cdr.charge_start_time_month,
                    cdr.charge_start_time_day,
                    cdr.charge_start_time_hour,
                    cdr.charge_start_time_minute,
                    cdr.charge_start_time_second);

            // 主叫号码计划
            fprintf(fp_o, "%d,", cdr.calling_num_plan);

            // 主叫号码类型
            fprintf(fp_o, "%d,", cdr.calling_num_type);

            // 主叫号码长度
            fprintf(fp_o, "%d,", cdr.calling_num_length);

            // 主叫号码
            fprintf(fp_o, "%s,", cdr.calling_num);

            // 被叫号码计划
            fprintf(fp_o, "%d,", cdr.dialed_num_plan);

            // 被叫号码类型
            fprintf(fp_o, "%d,", cdr.dialed_num_type);

            // 被叫号码长度
            fprintf(fp_o, "%d,", cdr.dialed_num_length);

            // 被叫号码
            fprintf(fp_o, "%s,", cdr.dialed_num);

            // 第三方号码计划
            fprintf(fp_o, "%d,", cdr.connected_num_plan);

            // 第三方号码类型
            fprintf(fp_o, "%d,", cdr.connected_num_type);

            // 第三方号码长度
            fprintf(fp_o, "%d,", cdr.connected_num_length);

            // 第三方号码
            fprintf(fp_o, "%s,", cdr.connected_num);

            //被叫漫游号码
            fprintf(fp_o, "%s,", cdr.msrn);

            // 被计费用户MSISDN
            fprintf(fp_o, "%s,", cdr.served_msisdn);
            
            // 被计费移动用户IMSI
            fprintf(fp_o, "%s,", cdr.served_imsi);

            // 被计费移动用户IMEI
            fprintf(fp_o, "%s,", cdr.served_imei);

            // 入中继群号
            fprintf(fp_o, "%d,", cdr.incoming_trunk_group_id);

            // 出中继群号
            fprintf(fp_o, "%d,", cdr.outgoing_trunk_group_id);

            // 通话时长
            fprintf(fp_o, "%d,", cdr.charge_duration);

            // 首话单索引号
            fprintf(fp_o, "%d,", cdr.index_of_first_cdr);

            // 中间话单序列号
            fprintf(fp_o, "%d,", cdr.sequence_of_intermediate_cdr);

            // 记录类型
            fprintf(fp_o, "%d,", cdr.record_type);
      
            // 用户类型
            fprintf(fp_o, "%d,", cdr.user_type);

            // 通话终止原因
            fprintf(fp_o, "%d,", cdr.cause_for_call_termination);

            /* 产生中间话单原因 */
            fprintf(fp_o, "%d,", cdr.cause_for_intermediate_record);

            /* 本局MSC号 */
            fprintf(fp_o, "%s,", cdr.local_msc_id);

            /* 被计费移动用户当前所在MSC号 */
            fprintf(fp_o, "%s,", cdr.peer_msc_id);

            /* 主叫用户当前位置区 */
            fprintf(fp_o, "%s,", cdr.current_lac_of_caller);

            /* 主叫用户当前小区 */
            fprintf(fp_o, "%s,", cdr.current_ci_of_caller);

            /* 主叫用户初始位置区 */
            fprintf(fp_o, "%s,", cdr.initial_lac_of_caller);

            /* 主叫用户初始位小区 */
            fprintf(fp_o, "%s,", cdr.initial_ci_of_caller);

            /* 被叫用户当前位置区 */
            fprintf(fp_o, "%s,", cdr.current_lac_of_called);

            /* 被叫用户当前小区 */
            fprintf(fp_o, "%s,", cdr.current_ci_of_called);

            /* 被叫用户初始位置区 */
            fprintf(fp_o, "%s,", cdr.initial_lac_of_called);

            /* 被叫用户初始位小区 */
            fprintf(fp_o, "%s,", cdr.initial_ci_of_called);

            /* 被计费移动用户呼叫参考 */
            fprintf(fp_o, "%d,", cdr.call_reference);

            /* 传输模式 */
            fprintf(fp_o, "%d,", cdr.transmission_mode);

            /* 电话业务或承载业务标志 */
            fprintf(fp_o, "%d,", cdr.tbs_flag);

            /* 承载能力 */
            fprintf(fp_o, "%d,", cdr.bearer_capability);
 
            /* 电话业务或承载业务码 */
            fprintf(fp_o, "%d,", cdr.service_code_of_tbs);
 
            /* 业务类别 */
            fprintf(fp_o, "%d,", cdr.gsm_gsvn);
 
            /* 补充业务1 */
            fprintf(fp_o, "%d,", cdr.ss_code1);
 
            /* 补充业务2 */
            fprintf(fp_o, "%d,", cdr.ss_code2);
 
            /* 补充业务3 */
            fprintf(fp_o, "%d,", cdr.ss_code3);

            /* 补充业务4 */
            fprintf(fp_o, "%d,", cdr.ss_code4);

            /* 被计费移动用户初始CLASSMARK */
            /* rfc */
            fprintf(fp_o, "%d,", cdr.initial_classmark_rfc);

            /* a5_1 */
            fprintf(fp_o, "%d,", cdr.initial_classmark_a5_1);

            /* es ind */
            fprintf(fp_o, "%d,", cdr.initial_classmark_es_ind);

            /* rev_level */
            fprintf(fp_o, "%d,", cdr.initial_classmark_rev_level);

            /* fc */
            fprintf(fp_o, "%d,", cdr.initial_classmark_fc);

            /* vbs */
            fprintf(fp_o, "%d,", cdr.initial_classmark_vbs);

            /* vgcs */
            fprintf(fp_o, "%d,", cdr.initial_classmark_vgcs);

            /* smc */
            fprintf(fp_o, "%d,", cdr.initial_classmark_smc);

            /* ss ind */
            fprintf(fp_o, "%d,", cdr.initial_classmark_ss_ind);

            /* ps_cap */
            fprintf(fp_o, "%d,", cdr.initial_classmark_ps_cap);

            /* a5_2 */
            fprintf(fp_o, "%d,", cdr.initial_classmark_a5_2);

            /* a5_3 */
            fprintf(fp_o, "%d,", cdr.initial_classmark_a5_3);

            /* cm_3 */
            fprintf(fp_o, "%d,", cdr.initial_classmark_cm_3);

            /* 被计费移动用户当前CLASSMARK */
            /* rfc */
            fprintf(fp_o, "%d,", cdr.current_classmark_rfc);

            /* a5_1 */
            fprintf(fp_o, "%d,", cdr.current_classmark_a5_1);

            /* es ind */
            fprintf(fp_o, "%d,", cdr.current_classmark_es_ind);

            /* rev_level */
            fprintf(fp_o, "%d,", cdr.current_classmark_rev_level);

            /* fc */
            fprintf(fp_o, "%d,", cdr.current_classmark_fc);

            /* vbs */
            fprintf(fp_o, "%d,", cdr.current_classmark_vbs);

            /* vgcs */
            fprintf(fp_o, "%d,", cdr.current_classmark_vgcs);

            /* smc */
            fprintf(fp_o, "%d,", cdr.current_classmark_smc);

            /* ss ind */
            fprintf(fp_o, "%d,", cdr.current_classmark_ss_ind);

            /* ps_cap */
            fprintf(fp_o, "%d,", cdr.current_classmark_ps_cap);

            /* a5_2 */
            fprintf(fp_o, "%d,", cdr.current_classmark_a5_2);

            /* a5_3 */
            fprintf(fp_o, "%d,", cdr.current_classmark_a5_3);

            /* cm_3 */
            fprintf(fp_o, "%d,", cdr.current_classmark_cm_3);

            /* 透明非透明指示 */
            fprintf(fp_o, "%d,", cdr.transparency_indicator);

            /* 是否使用DTMF */
            fprintf(fp_o, "%d,", cdr.dtmf_flag);

            /* 计费免费指示 */
            fprintf(fp_o, "%d,", cdr.free_indicator);

            /* 漫游用户标志 */
            fprintf(fp_o, "%d,", cdr.roam_flag);

            /* 热计费标志 */
            fprintf(fp_o, "%d,", cdr.hot_bill_flag);

            /* 操作结果 */
            fprintf(fp_o, "%d,", cdr.action_result);

            /* 费率指示 */
            fprintf(fp_o, "%d,", cdr.charging_case);

            /* 费率 */
            fprintf(fp_o, "%d,", cdr.money_per_count);

            /* 附加费 */
            fprintf(fp_o, "%d,", cdr.add_fee);
            
            /* 该次呼叫占用的B信道数 */
            fprintf(fp_o, "%d,", cdr.number_of_b_channels_occupied);

            /* 字节数 */
            fprintf(fp_o, "%d,", cdr.number_of_bytes);

            /* 短消息中心地址 */
            fprintf(fp_o, "%s,", cdr.smsc_address);

            /* 每个数据单元的字节数 */
            fprintf(fp_o, "%d,", cdr.bytes_per_packet);

            /* 业务键 */
            memset(tmp_buf, 0, sizeof(tmp_buf));
            hextostr(tmp_buf, cdr.servicekey, sizeof(cdr.servicekey));
            for (i = 0; i < sizeof(tmp_buf); ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            fprintf(fp_o, "%s,", tmp_buf);

            /* SCFID */
            memset(tmp_buf, 0, sizeof(tmp_buf));
            hextostr(tmp_buf, cdr.scfid, sizeof(cdr.scfid));
            for (i = 0; i < sizeof(tmp_buf); ++i) 
            {
                if (*(tmp_buf + i) == 'f' || *(tmp_buf + i) == 'F')
                {
                    *(tmp_buf +i) = '\0';
                    break;
                }
            }
            fprintf(fp_o, "%s\n", tmp_buf);

            /* 保留字节 */
		}
	}

    /* 返回生成的CVS文件名 */
    strcat(out_file_name, ":");
    strcat(out_file_name, csv_file_name);

    /* 返回话单记录总数 */
	(*rec_num) = serial_num;

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
	vfprintf(fp,format,ap);
	va_end(ap);
	fprintf(fp,"\n");
  
	fclose(fp);
	
	return 0;
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

/*
 *hextostr()
 */
static void hextostr(char * out_str, const char * hex, int hex_num)
{
	int i;
	for(i=0;i<hex_num;i++)
	{
		sprintf(out_str,"%02X",*hex);
		out_str+=2;
		hex++;
	}	
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
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }
    strncpy(ret_file_name, ptr_front, ++ptr_back - ptr_front);

    /* 厂家 */
    ptr_front = ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }
    strncat(ret_file_name, ptr_front, ++ptr_back - ptr_front);

    /* 端局 */
    ptr_front = ptr_back;
    ptr_back = strchr(ptr_front, '_');
    if(ptr_back == NULL)
    {
        err_log("get_commit_file_name: in file name format incorrect\n");
        return 1;
    }
    strncat(ret_file_name, ptr_front, ++ptr_back - ptr_front);

    /* 话单类型 */
    if (bill_type != NULL)
    {
        strcat(ret_file_name, bill_type);
        strcat(ret_file_name, "_");
    }

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

    /* 获取网元名称 */
    strncpy(ret_ne_name, ptr_front, ptr_back - ptr_front);

    return 0;
}

static bool bcdhextostr(bool is_zipbcd, bool is_bigend, bool is_all_num, const char* data, int len, char * out_str)
{
    int res_len;
    int i;
    unsigned char ch;

    if(data == NULL || len <= 0)
    {
        return FALSE;
    }

    res_len = (is_zipbcd) ? len*2 : len;

    for(i = 0; i < res_len; ++i)
    {
        if(is_zipbcd)
        {
            ch = data[i/2];
            if(i%2 == 0)
            {
                ch = (is_bigend) ? ch >> 4: ch;
            }
            else
            {
                ch = (is_bigend) ? ch : ch >> 4;
            }
        }
        else
        {
            ch = data[i];
        }

        ch &= 0x0f;

        if(is_all_num && ch > 9)
        {
            break;
        }

        sprintf(out_str, "%c", BCDSTR[ch]);
        out_str++;
    }
    return TRUE;
}

