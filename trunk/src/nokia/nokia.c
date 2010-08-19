/************************************************************************/
/* nokia.c                                                              */
/*                                                                      */ 
/* 诺基亚话单解析程序                                                   */
/*                                                                      */
/* create by 刘洋 at 2010.05                                            */
/************************************************************************/

/************************************************************************/
/* include 头文件 */
/************************************************************************/
#include <string.h>
#include <stdio.h>

// #include <time.h>
// #include <stdarg.h>
/************************************************************************/
/* 宏定义 */
/************************************************************************/

/************************************************************************/
/* 全局变量定义 */
/************************************************************************/
//debug开关
int g_nDebug = 0;

/************************************************************************/
/* 枚举定义 */                                                         
/************************************************************************/
//记录类型
typedef enum                                                
{                                                                      
	eRecordType_Header	= 0x00,
	eRecordType_Moc			= 0x01,
	eRecordType_Mtc			= 0x02,
	eRecordType_Forw		= 0x03,
	eRecordType_Roam		= 0x04,
	eRecordType_Sups		= 0x05,
	eRecordType_Smmo		= 0x08,
	eRecordType_Smmt		= 0x09,
	eRecordType_Poc			= 0x0B,
	eRecordType_Ptc			= 0x0C,
	eRecordType_Doc			= 0x14,
	eRecordType_Coc			= 0x18,
	eRecordType_Ctc			= 0x19,
	eRecordType_In4			= 0x1A,
	eRecordType_Trailer	= 0x10,
	eRecordType_END			= 0xFF	
}eRecordType;                                                                     

/************************************************************************/
/* 结构体定义 */
/************************************************************************/
//Segment Length and Type In-Data
typedef struct 
{
	unsigned char Record_length[2];                                      
	unsigned char Record_type[1];                                        
} SSegHandleIn;                                                        

//Segment Length and Type Out-Data
typedef struct 
{
	unsigned int unRecord_length;                                        
	eRecordType  eRecord_type;                                           
} SSegHandleOut;                                                       

//data files header records In-Data
typedef struct 
{
	unsigned char Hea_charging_block_size[1];                            
	unsigned char Hea_tape_block_type[2];                                
	unsigned char Hea_data_length_in_block[2];                           
	unsigned char Hea_exchange_id[10];                                   
	unsigned char Hea_first_record_number[4];                            
	unsigned char Hea_batch_seq_number[4];                               
	unsigned char Hea_block_seq_number[2];                               
	unsigned char Hea_start_time[7];                                     
	unsigned char Hea_format_version[6];                                 
} SHeaderIn;                                                           
                                                        
//data files header records Out-Data
typedef struct 
{
	unsigned int unHea_record_length;                                    
	eRecordType  eHea_record_type;                                       
	char 	       szHea_charging_block_size[5];                           
	unsigned int unHea_tape_block_type;                                  
	unsigned int unHea_data_length_in_block;                             
	char         szHea_exchange_id[21];                                  
	char         szHea_first_record_number[9];                           
	char         szHea_batch_seq_number[9];                              
	char         szHea_block_seq_number[5];                              
	char         szHea_start_time[20];                                   
	char         lpHea_format_version[13];                               
} SHeaderOut;                                                          
                                                                    
//data files trailer records In-Data                                   
typedef struct                                                         
{                                                                      
	unsigned char Tra_exchange_id[10];                                   
	unsigned char Tra_end_time[7];                                       
	unsigned char Tra_last_record_number[4];                             
} STrailerIn;        

//data files trailer records Out-Data                                  
typedef struct                                                         
{                                                                      
	unsigned int unTra_record_length;                                    
	eRecordType  eTra_record_type;                         
	char         szTra_exchange_id[21];											
	char         szTra_end_time[20];                       
	char         szTra_last_record_number[9];              
} STrailerOut;                                           

//Mobile-originated call In-Data                                              
typedef struct                                                        
{                                                               
	unsigned char Moc_record_number[4];                           
	unsigned char Moc_record_status[1];                           
	unsigned char Moc_check_sum[2];                               
	unsigned char Moc_call_reference[5];                          
	unsigned char Moc_exchange_id[10];                            
	unsigned char Moc_intermediate_record_number[1];              
	unsigned char Moc_intermediate_charging_ind[1];               
	unsigned char Moc_number_of_ss_records[1];                    
	unsigned char Moc_calling_imsi[8];                            
	unsigned char Moc_calling_imei[8];                            
	unsigned char Moc_calling_number[10];                         
	unsigned char Moc_calling_category[1];                        
	unsigned char Moc_calling_ms_classmark[1];                    
	unsigned char Moc_called_imsi[8];                             
	unsigned char Moc_called_imei[8];                             
	unsigned char Moc_dialled_digits_ton[1];                      
	unsigned char Moc_called_number[12];                          
	unsigned char Moc_called_category[1];                         
	unsigned char Moc_called_ms_classmark[1];                     
	unsigned char Moc_dialled_digits[12];                         
	unsigned char Moc_calling_subs_first_lac[2];                  
	unsigned char Moc_calling_subs_first_ci[2];                   
	unsigned char Moc_calling_subs_last_ex_id[10];                
	unsigned char Moc_calling_subs_last_lac[2];                   
	unsigned char Moc_calling_subs_last_ci[2];                    
	unsigned char Moc_called_subs_first_lac[2];                   
	unsigned char Moc_called_subs_first_ci[2];                    
	unsigned char Moc_called_subs_last_ex_id[10];                 
	unsigned char Moc_called_subs_last_lac[2];                    
	unsigned char Moc_called_subs_last_ci[2];                     
	unsigned char Moc_out_circuit_group[2];                       
	unsigned char Moc_out_circuit[2];                             
	unsigned char Moc_basic_service_type[1];                      
	unsigned char Moc_basic_service_code[1];                      
	unsigned char Moc_non_transparency_indicator[1];              
	unsigned char Moc_channel_rate_indicator[1];                  
	unsigned char Moc_set_up_start_time[7];                       
	unsigned char Moc_in_channel_allocated_time[7];               
	unsigned char Moc_charging_start_time[7];                     
	unsigned char Moc_charging_end_time[7];                       
	unsigned char Moc_orig_mcz_duration[3];                       
	unsigned char Moc_cause_for_termination[4];                   
	unsigned char Moc_data_volume[2];                             
	unsigned char Moc_call_type[1];                               
	unsigned char Moc_orig_mcz_tariff_class[3];                   
	unsigned char Moc_orig_mcz_pulses[2];                         
	unsigned char Moc_dtmf_indicator[1];                          
	unsigned char Moc_aoc_indicator[1];                           
	unsigned char Moc_called_msrn_ton[1];                         
	unsigned char Moc_called_msrn[12];                            
	unsigned char Moc_called_number_ton[1];                       
	unsigned char Moc_facility_usage[4];                          
	unsigned char Moc_orig_mcz_chrg_type[1];                      
	unsigned char Moc_calling_number_ton[1];                      
	unsigned char Moc_routing_category[1];                        
	unsigned char Moc_intermediate_chrg_cause[2];                 
	unsigned char Moc_camel_call_reference[8];                    
	unsigned char Moc_camel_exchange_id_ton[1];                   
	unsigned char Moc_camel_exchange_id[9];                       
	unsigned char Moc_calling_modify_parameters[14];              
	unsigned char Moc_orig_mcz_modify_percent[2];                 
	unsigned char Moc_orig_mcz_modify_direction[1];               
	unsigned char Moc_orig_dialling_class[2];                     
	unsigned char Moc_virtual_msc_id[10];                         
	unsigned char Moc_scf_address_ton[1];                         
	unsigned char Moc_scf_address[9];                             
	unsigned char Moc_destination_number_ton[1];                  
	unsigned char Moc_destination_number_npi[1];                  
	unsigned char Moc_destination_number[16];                     
	unsigned char Moc_camel_service_key[4];                       
	unsigned char Moc_calling_imeisv[8];                          
	unsigned char Moc_called_imeisv[8];                           
	unsigned char Moc_camel_service_data[56];                     
} SMocInOld;                                                                  

//Mobile-originated call In-Data                                              
typedef struct                                                        
{                                                                      
	unsigned char Moc_record_number[4];                                       
	unsigned char Moc_record_status[1];                                       
	unsigned char Moc_check_sum[2];                                           
	unsigned char Moc_call_reference[5];                                      
	unsigned char Moc_exchange_id[10];                                        
	unsigned char Moc_intermediate_record_number[1];                          
	unsigned char Moc_intermediate_charging_ind[1];                           
	unsigned char Moc_number_of_ss_records[1];                                
	unsigned char Moc_calling_imsi[8];                                        
	unsigned char Moc_calling_imei[8];                                        
	unsigned char Moc_calling_number[10];                                     
	unsigned char Moc_calling_category[1];                                    
	unsigned char Moc_calling_ms_classmark[1];                                
	unsigned char Moc_called_imsi[8];                                         
	unsigned char Moc_called_imei[8];                                         
	unsigned char Moc_dialled_digits_ton[1];                                  
	unsigned char Moc_called_number[12];                                      
	unsigned char Moc_called_category[1];                                     
	unsigned char Moc_called_ms_classmark[1];                                 
	unsigned char Moc_dialled_digits[12];                                     
	unsigned char Moc_calling_subs_first_lac[2];                              
	unsigned char Moc_calling_subs_first_ci[2];                               
	unsigned char Moc_calling_subs_last_ex_id[10];                            
	unsigned char Moc_calling_subs_last_lac[2];                               
	unsigned char Moc_calling_subs_last_ci[2];                                
	unsigned char Moc_called_subs_first_lac[2];                               
	unsigned char Moc_called_subs_first_ci[2];                                
	unsigned char Moc_called_subs_last_ex_id[10];                             
	unsigned char Moc_called_subs_last_lac[2];                                
	unsigned char Moc_called_subs_last_ci[2];                                 
	unsigned char Moc_out_circuit_group[2];                                   
	unsigned char Moc_out_circuit[2];                                         
	unsigned char Moc_basic_service_type[1];                                  
	unsigned char Moc_basic_service_code[1];                                  
	unsigned char Moc_non_transparency_indicator[1];                          
	unsigned char Moc_channel_rate_indicator[1];                              
	unsigned char Moc_set_up_start_time[7];                                   
	unsigned char Moc_in_channel_allocated_time[7];                           
	unsigned char Moc_charging_start_time[7];                                 
	unsigned char Moc_charging_end_time[7];                                   
	unsigned char Moc_orig_mcz_duration[3];                                   
	unsigned char Moc_cause_for_termination[4];                               
	unsigned char Moc_data_volume[2];                                         
	unsigned char Moc_call_type[1];                                           
	unsigned char Moc_orig_mcz_tariff_class[3];                               
	unsigned char Moc_orig_mcz_pulses[2];                                     
	unsigned char Moc_dtmf_indicator[1];                                      
	unsigned char Moc_aoc_indicator[1];                                       
	unsigned char Moc_called_msrn_ton[1];                                     
	unsigned char Moc_called_msrn[12];                                        
	unsigned char Moc_called_number_ton[1];                                   
	unsigned char Moc_facility_usage[4];                                      
	unsigned char Moc_orig_mcz_chrg_type[1];                                  
	unsigned char Moc_calling_number_ton[1];                                  
	unsigned char Moc_routing_category[1];                                    
	unsigned char Moc_intermediate_chrg_cause[2];                             
	unsigned char Moc_camel_call_reference[8];                                
	unsigned char Moc_camel_exchange_id_ton[1];                               
	unsigned char Moc_camel_exchange_id[9];                                   
	unsigned char Moc_calling_modify_parameters[14];                          
	unsigned char Moc_orig_mcz_modify_percent[2];                             
	unsigned char Moc_orig_mcz_modify_direction[1];                           
	unsigned char Moc_orig_dialling_class[2];                                 
	unsigned char Moc_virtual_msc_id[10];                                     
	unsigned char Moc_scf_address_ton[1];                                     
	unsigned char Moc_scf_address[9];                                         
	unsigned char Moc_destination_number_ton[1];                              
	unsigned char Moc_destination_number_npi[1];                              
	unsigned char Moc_destination_number[16];                                 
	unsigned char Moc_camel_service_key[4];                                   
	unsigned char Moc_calling_imeisv[8];                                      
	unsigned char Moc_called_imeisv[8];                                       
	unsigned char Moc_emergency_call_category[1];
	unsigned char Moc_used_air_interface_user_rate[1];
	unsigned char Moc_req_air_interface_user_rate[1];
	unsigned char Moc_used_fixed_nw_user_rate[1];
	unsigned char Moc_req_fixed_nw_user_rate[1];
	unsigned char Moc_rate_adaption[1];
	unsigned char Moc_stream_identifier[1];
	unsigned char Moc_ms_classmark3[1];
	unsigned char Moc_calling_cell_band[1];
	unsigned char Moc_calling_subs_last_ex_id_ton[1];
	unsigned char Moc_called_subs_last_ex_id_ton[1];
	unsigned char Moc_calling_subs_first_mcc[2];
	unsigned char Moc_calling_subs_first_mnc[2];
	unsigned char Moc_calling_subs_last_mcc[2];
	unsigned char Moc_calling_subs_last_mnc[2];
	unsigned char Moc_called_subs_first_mcc[2];
	unsigned char Moc_called_subs_first_mnc[2];
	unsigned char Moc_called_subs_last_mcc[2];
	unsigned char Moc_called_subs_last_mnc[2];
	unsigned char Moc_camel_service_data[56]; 
	unsigned char Moc_selected_codec[1];
	unsigned char Moc_outside_user_plane_index[2]; 
	unsigned char Moc_outside_control_plane_index[2];
	unsigned char Moc_out_bnc_connection_type[1];  
	unsigned char Moc_radio_network_type[1];
} SMocIn;                                                                  

//Mobile-originated call Out-Data                                              
typedef struct                                                        
{                                                                      
	unsigned int  unMoc_record_length;                              
	eRecordType   eMoc_record_type;                                 
	char          szMoc_record_number[9];                           
	char          szMoc_record_status[20];                          
	unsigned int  unMoc_check_sum;                                  
	char          szMoc_call_reference[32];                         
	char          szMoc_exchange_id[21];                            
	char          szMoc_intermediate_record_number[3];              
	unsigned int  unMoc_intermediate_charging_ind;                  
	char          szMoc_number_of_ss_records[3];                    
	char          szMoc_calling_imsi[17];                           
	char          szMoc_calling_imei[17];                           
	char          szMoc_calling_number[21];                         
	unsigned int  unMoc_calling_category;                           
	unsigned int  unMoc_calling_ms_classmark;                       
	char          szMoc_called_imsi[17];                            
	char          szMoc_called_imei[17];                            
	unsigned int  unMoc_dialled_digits_ton;                         
	char          szMoc_called_number[25];                          
	unsigned int  unMoc_called_category;                            
	unsigned int  unMoc_called_ms_classmark;                        
	char          szMoc_dialled_digits[25];                         
	unsigned int  unMoc_calling_subs_first_lac;                     
	unsigned int  unMoc_calling_subs_first_ci;                      
	char          szMoc_calling_subs_last_ex_id[21];                
	unsigned int  unMoc_calling_subs_last_lac;                      
	unsigned int  unMoc_calling_subs_last_ci;                       
	unsigned int  unMoc_called_subs_first_lac;                      
	unsigned int  unMoc_called_subs_first_ci;                       
	char          szMoc_called_subs_last_ex_id[21];                 
	unsigned int  unMoc_called_subs_last_lac;                       
	unsigned int  unMoc_called_subs_last_ci;                        
	char          szMoc_out_circuit_group[5];                       
	char          szMoc_out_circuit[5];                             
	unsigned int  unMoc_basic_service_type;                         
	unsigned int  unMoc_basic_service_code;                         
	unsigned int  unMoc_non_transparency_indicator;                 
	unsigned int  unMoc_channel_rate_indicator;                     
	char          szMoc_set_up_start_time[20];                      
	char          szMoc_in_channel_allocated_time[20];              
	char          szMoc_charging_start_time[20];                    
	char          szMoc_charging_end_time[20];                      
	unsigned int  unMoc_orig_mcz_duration;                          
	unsigned int  unMoc_cause_for_termination;                      
	char          szMoc_data_volume[5];                             
	unsigned int  unMoc_call_type;                                  
	char          szMoc_orig_mcz_tariff_class[7];                   
	char          szMoc_orig_mcz_pulses[5];                         
	unsigned int  unMoc_dtmf_indicator;                             
	unsigned int  unMoc_aoc_indicator;                              
	unsigned int  unMoc_called_msrn_ton;                            
	char          szMoc_called_msrn[25];                            
	unsigned int  unMoc_called_number_ton;                          
	char          szMoc_facility_usage[9];                          
	unsigned int  unMoc_orig_mcz_chrg_type;                         
	unsigned int  unMoc_calling_number_ton;                         
	unsigned int  unMoc_routing_category;                           
	unsigned int  unMoc_intermediate_chrg_cause;                    
	char           szMoc_camel_call_reference[17];                  
	unsigned int  unMoc_camel_exchange_id_ton;                      
	char          szMoc_camel_exchange_id[19];                      
	char          szMoc_calling_modify_parameters[42];              
	unsigned int  unMoc_orig_mcz_modify_percent;                    
	unsigned int  unMoc_orig_mcz_modify_direction;                  
	unsigned int  unMoc_orig_dialling_class;                        
	char          szMoc_virtual_msc_id[21];                         
	unsigned int  unMoc_scf_address_ton;                            
	char          szMoc_scf_address[19];                            
	unsigned int  unMoc_destination_number_ton;                     
	unsigned int  unMoc_destination_number_npi;                     
	char          szMoc_destination_number[33];                     
	char          szMoc_camel_service_key[9];                       
	char          szMoc_calling_imeisv[17];                         
	char          szMoc_called_imeisv[17];                          
	unsigned int  unMoc_emergency_call_category;        
	unsigned int  unMoc_used_air_interface_user_rate;    
	unsigned int  unMoc_req_air_interface_user_rate;    
	unsigned int  unMoc_used_fixed_nw_user_rate;   
	unsigned int  unMoc_req_fixed_nw_user_rate;   
	unsigned int  unMoc_rate_adaption;    
	unsigned int  unMoc_stream_identifier;  
	unsigned int  unMoc_ms_classmark3;    
	unsigned int  unMoc_calling_cell_band;  
	unsigned int  unMoc_calling_subs_last_ex_id_ton;    
	unsigned int  unMoc_called_subs_last_ex_id_ton;    
	unsigned int  unMoc_calling_subs_first_mcc;
	unsigned int  unMoc_calling_subs_first_mnc;
	unsigned int  unMoc_calling_subs_last_mcc;         
	unsigned int  unMoc_calling_subs_last_mnc;         
	unsigned int  unMoc_called_subs_first_mcc;         
	unsigned int  unMoc_called_subs_first_mnc;         
	unsigned int  unMoc_called_subs_last_mcc;
	unsigned int  unMoc_called_subs_last_mnc;
	char          szMoc_camel_service_data[113];
	unsigned int  unMoc_selected_codec;
	char          szMoc_outside_user_plane_index[5];
	char          szMoc_outside_control_plane_index[5];
	unsigned int  unMoc_out_bnc_connection_type;
	unsigned int  unMoc_radio_network_type;
} SMocOut;                                                                  
 
//Mobile-terminated call In-Data                                              
typedef struct                                                         
{                                                            
	unsigned char Mtc_record_number[4];                        
	unsigned char Mtc_record_status[1];                        
	unsigned char Mtc_check_sum[2];                            
	unsigned char Mtc_call_reference[5];                       
	unsigned char Mtc_exchange_id[10];                         
	unsigned char Mtc_intermediate_record_number[1];           
	unsigned char Mtc_intermediate_charging_ind[1];            
	unsigned char Mtc_number_of_ss_records[1];                 
	unsigned char Mtc_calling_number[10];                      
	unsigned char Mtc_called_imsi[8];                          
	unsigned char Mtc_called_imei[8];                          
	unsigned char Mtc_called_number[12];                       
	unsigned char Mtc_called_category[1];                      
	unsigned char Mtc_called_ms_classmark[1];                  
	unsigned char Mtc_in_circuit_group[2];                     
	unsigned char Mtc_in_circuit[2];                           
	unsigned char Mtc_called_subs_first_lac[2];                
	unsigned char Mtc_called_subs_first_ci[2];                 
	unsigned char Mtc_called_subs_last_ex_id[10];              
	unsigned char Mtc_called_subs_last_lac[2];                 
	unsigned char Mtc_called_subs_last_ci[2];                  
	unsigned char Mtc_basic_service_type[1];                   
	unsigned char Mtc_basic_service_code[1];                   
	unsigned char Mtc_non_transparency_indicator[1];           
	unsigned char Mtc_channel_rate_indicator[1];               
	unsigned char Mtc_set_up_start_time[7];                    
	unsigned char Mtc_in_channel_allocated_time[7];            
	unsigned char Mtc_charging_start_time[7];                  
	unsigned char Mtc_charging_end_time[7];                    
	unsigned char Mtc_term_mcz_duration[3];                    
	unsigned char Mtc_cause_for_termination[4];                
	unsigned char Mtc_data_volume[2];                          
	unsigned char Mtc_call_type[1];                            
	unsigned char Mtc_term_mcz_tariff_class[3];                
	unsigned char Mtc_term_mcz_pulses[2];                      
	unsigned char Mtc_dtmf_indicator[1];                       
	unsigned char Mtc_aoc_indicator[1];                        
	unsigned char Mtc_facility_usage[4];                       
	unsigned char Mtc_term_mcz_chrg_type[1];                   
	unsigned char Mtc_calling_number_ton[1];                   
	unsigned char Mtc_called_number_ton[1];                    
	unsigned char Mtc_routing_category[1];                     
	unsigned char Mtc_intermediate_chrg_cause[2];              
	unsigned char Mtc_camel_call_reference[8];                 
	unsigned char Mtc_camel_exchange_id_ton[1];                
	unsigned char Mtc_camel_exchange_id[9];                    
	unsigned char Mtc_called_modify_parameters[14];            
	unsigned char Mtc_term_mcz_modify_percent[2];              
	unsigned char Mtc_term_mcz_modify_direction[1];            
	unsigned char Mtc_redirecting_number[12];                  
	unsigned char Mtc_redirecting_number_ton[1];               
	unsigned char Mtc_virtual_msc_id[10];                      
	unsigned char Mtc_called_imeisv[8];                        
} SMtcInOld;                                                                

//Mobile-terminated call In-Data                                              
typedef struct                                                         
{                                                                
	unsigned char Mtc_record_number[4];                            
	unsigned char Mtc_record_status[1];                            
	unsigned char Mtc_check_sum[2];                                
	unsigned char Mtc_call_reference[5];                           
	unsigned char Mtc_exchange_id[10];                             
	unsigned char Mtc_intermediate_record_number[1];               
	unsigned char Mtc_intermediate_charging_ind[1];                
	unsigned char Mtc_number_of_ss_records[1];                     
	unsigned char Mtc_calling_number[10];                          
	unsigned char Mtc_called_imsi[8];                              
	unsigned char Mtc_called_imei[8];                              
	unsigned char Mtc_called_number[12];                           
	unsigned char Mtc_called_category[1];                          
	unsigned char Mtc_called_ms_classmark[1];                      
	unsigned char Mtc_in_circuit_group[2];                         
	unsigned char Mtc_in_circuit[2];                               
	unsigned char Mtc_called_subs_first_lac[2];                    
	unsigned char Mtc_called_subs_first_ci[2];                     
	unsigned char Mtc_called_subs_last_ex_id[10];                  
	unsigned char Mtc_called_subs_last_lac[2];                     
	unsigned char Mtc_called_subs_last_ci[2];                      
	unsigned char Mtc_basic_service_type[1];                       
	unsigned char Mtc_basic_service_code[1];                       
	unsigned char Mtc_non_transparency_indicator[1];               
	unsigned char Mtc_channel_rate_indicator[1];                   
	unsigned char Mtc_set_up_start_time[7];                        
	unsigned char Mtc_in_channel_allocated_time[7];                
	unsigned char Mtc_charging_start_time[7];                      
	unsigned char Mtc_charging_end_time[7];                        
	unsigned char Mtc_term_mcz_duration[3];                        
	unsigned char Mtc_cause_for_termination[4];                    
	unsigned char Mtc_data_volume[2];                              
	unsigned char Mtc_call_type[1];                                
	unsigned char Mtc_term_mcz_tariff_class[3];                    
	unsigned char Mtc_term_mcz_pulses[2];                          
	unsigned char Mtc_dtmf_indicator[1];                           
	unsigned char Mtc_aoc_indicator[1];                            
	unsigned char Mtc_facility_usage[4];                           
	unsigned char Mtc_term_mcz_chrg_type[1];                       
	unsigned char Mtc_calling_number_ton[1];                       
	unsigned char Mtc_called_number_ton[1];                        
	unsigned char Mtc_routing_category[1];                         
	unsigned char Mtc_intermediate_chrg_cause[2];                  
	unsigned char Mtc_camel_call_reference[8];                     
	unsigned char Mtc_camel_exchange_id_ton[1];                    
	unsigned char Mtc_camel_exchange_id[9];                        
	unsigned char Mtc_called_modify_parameters[14];                
	unsigned char Mtc_term_mcz_modify_percent[2];                  
	unsigned char Mtc_term_mcz_modify_direction[1];                
	unsigned char Mtc_redirecting_number[12];                      
	unsigned char Mtc_redirecting_number_ton[1];                   
	unsigned char Mtc_virtual_msc_id[10];                          
	unsigned char Mtc_called_imeisv[8];                            
	unsigned char Mtc_used_air_interface_user_rate[1]; 
	unsigned char Mtc_req_air_interface_user_rate[1];
	unsigned char Mtc_used_fixed_nw_user_rate[1];
	unsigned char Mtc_req_fixed_nw_user_rate[1];
	unsigned char Mtc_rate_adaption[1];
	unsigned char Mtc_stream_identifier[1];
	unsigned char Mtc_ms_classmark3[1]; 
	unsigned char Mtc_called_cell_band[1];
	unsigned char Mtc_called_subs_last_ex_id_ton[1];
	unsigned char Mtc_called_subs_first_mcc[2];
	unsigned char Mtc_called_subs_first_mnc[2];
	unsigned char Mtc_called_subs_last_mcc[2];
	unsigned char Mtc_called_subs_last_mnc[2];
	unsigned char Mtc_selected_codec[1];
	unsigned char Mtc_inside_user_plane_index[2];
	unsigned char Mtc_inside_control_plane_index[2];
	unsigned char Mtc_in_bnc_connection_type[1];
	unsigned char Mtc_radio_network_type[1];
} SMtcIn;                                                        

//Mobile-terminated call Out-Data                                              
typedef struct                                                         
{      
	unsigned int unMtc_record_length;                                           
	eRecordType  eMtc_record_type;                                             
	char         szMtc_record_number[9];                                           
	char         szMtc_record_status[20];                                           
	unsigned int unMtc_check_sum;                                               
	char         szMtc_call_reference[32];                                          
	char         szMtc_exchange_id[21];                                            
	char         szMtc_intermediate_record_number[3];                             
	unsigned int unMtc_intermediate_charging_ind;                              
	char         szMtc_number_of_ss_records[3];  
	char         szMtc_calling_number[25];                                          
	char         szMtc_called_imsi[17];                                            
	char         szMtc_called_imei[17];  
	char         szMtc_called_number[25];                                          
	unsigned int unMtc_called_category;                                         
	unsigned int unMtc_called_ms_classmark;                             
	char         szMtc_in_circuit_group[5];                             
	char         szMtc_in_circuit[5];                                   
	unsigned int unMtc_called_subs_first_lac;                           
	unsigned int unMtc_called_subs_first_ci;                            
	char         szMtc_called_subs_last_ex_id[21];                      
	unsigned int unMtc_called_subs_last_lac;                            
	unsigned int unMtc_called_subs_last_ci;                             
	unsigned int unMtc_basic_service_type;                              
	unsigned int unMtc_basic_service_code;                              
	unsigned int unMtc_non_transparency_indicator;                      
	unsigned int unMtc_channel_rate_indicator;                          
	char         szMtc_set_up_start_time[20];                           
	char         szMtc_in_channel_allocated_time[20];                   
	char         szMtc_charging_start_time[20];                         
	char         szMtc_charging_end_time[20];                           
	unsigned int unMtc_term_mcz_duration;                               
	unsigned int unMtc_cause_for_termination;                           
	char         szMtc_data_volume[5];                                  
	unsigned int unMtc_call_type;                                       
	char         szMtc_term_mcz_tariff_class[7];                        
	char         szMtc_term_mcz_pulses[5];                              
	unsigned int unMtc_dtmf_indicator;                                  
	unsigned int unMtc_aoc_indicator;                                   
	char         szMtc_facility_usage[9];                               
	unsigned int unMtc_term_mcz_chrg_type;                              
	unsigned int unMtc_calling_number_ton;                              
	unsigned int unMtc_called_number_ton;                               
	unsigned int unMtc_routing_category;                                
	unsigned int unMtc_intermediate_chrg_cause;                         
	char         szMtc_camel_call_reference[17];                        
	unsigned int unMtc_camel_exchange_id_ton;                           
	char         szMtc_camel_exchange_id[19];                           
	char         szMtc_calling_modify_parameters[42];                   
	unsigned int unMtc_term_mcz_modify_percent;                         
	unsigned int unMtc_term_mcz_modify_direction;                       
	char         szMtc_redirecting_number[25];                          
	unsigned int unMtc_redirecting_number_ton;                          
	char         szMtc_virtual_msc_id[21];                              
	char         szMtc_called_imeisv[17];                               
	unsigned int unMtc_used_air_interface_user_rate;
	unsigned int unMtc_req_air_interface_user_rate;
	unsigned int unMtc_used_fixed_nw_user_rate;
	unsigned int unMtc_req_fixed_nw_user_rate;
	unsigned int unMtc_rate_adaption; 
	unsigned int unMtc_stream_identifier;
	unsigned int unMtc_ms_classmark3;
	unsigned int unMtc_called_cell_band;
	unsigned int unMtc_called_subs_last_ex_id_ton;
	unsigned int unMtc_called_subs_first_mcc;
	unsigned int unMtc_called_subs_first_mnc;
	unsigned int unMtc_called_subs_last_mcc;
	unsigned int unMtc_called_subs_last_mnc;
	unsigned int unMtc_selected_codec;
	char         szMtc_inside_user_plane_index[5];
	char         szMtc_inside_control_plane_index[5];
	unsigned int unMtc_in_bnc_connection_type;
	unsigned int unMtc_radio_network_type;
} SMtcOut;   

typedef struct                                                        
{                                                                     
	unsigned char Forw_record_number[4];                                
	unsigned char Forw_record_status[1];                                
	unsigned char Forw_check_sum[2];                                    
	unsigned char Forw_call_reference[5];                               
	unsigned char Forw_exchange_id[10];                                 
	unsigned char Forw_intermediate_record_number[1];                   
	unsigned char Forw_intermediate_charging_ind[1];                    
	unsigned char Forw_number_of_ss_records[1];                         
	unsigned char Forw_cause_for_forwarding[1];                         
	unsigned char Forw_forwarding_imsi[8];                              
	unsigned char Forw_forwarding_imei[8];                              
	unsigned char Forw_forwarding_number[12];                           
	unsigned char Forw_forwarding_category[1];                          
	unsigned char Forw_forwarding_ms_classmark[1];                      
	unsigned char Forw_forwarded_imsi[8];                               
	unsigned char Forw_forwarded_imei[8];                               
	unsigned char Forw_forwarded_number[12];                            
	unsigned char Forw_forwarded_ms_classmark[1];                       
	unsigned char Forw_orig_calling_number[10];                         
	unsigned char Forw_in_circuit_group[2];                             
	unsigned char Forw_in_circuit[2];                                   
	unsigned char Forw_forwarding_subs_first_lac[2];                    
	unsigned char Forw_forwarding_subs_first_ci[2];
	unsigned char Forw_forwarding_subs_last_ex_id[10];                  
	unsigned char Forw_forwarding_subs_last_lac[2];
	unsigned char Forw_forwarding_subs_last_ci[2];                      
	unsigned char Forw_forwarded_subs_first_lac[2];
	unsigned char Forw_forwarded_subs_first_ci[2];
	unsigned char Forw_forwarded_subs_last_ex_id[10];                   
	unsigned char Forw_forwarded_subs_last_lac[2];
	unsigned char Forw_forwarded_subs_last_ci[2];                
	unsigned char Forw_out_circuit_group[2];
	unsigned char Forw_out_circuit[2];
	unsigned char Forw_basic_service_type[1];                           
	unsigned char Forw_basic_service_code[1];                           
	unsigned char Forw_non_transparency_indicator[1];                   
	unsigned char Forw_channel_rate_indicator[1];                       
	unsigned char Forw_set_up_start_time[7];                            
	unsigned char Forw_in_channel_allocated_time[7];                    
	unsigned char Forw_charging_start_time[7];                          
	unsigned char Forw_charging_end_time[7];                            
	unsigned char Forw_forw_mcz_duration[3];                            
	unsigned char Forw_cause_for_termination[4];                        
	unsigned char Forw_data_volume[2];                                  
	unsigned char Forw_call_type[1];                                    
	unsigned char Forw_forw_mcz_tariff_class[3];                        
	unsigned char Forw_forw_mcz_pulses[2];                              
	unsigned char Forw_dtmf_indicator[1];                               
	unsigned char Forw_aoc_indicator[1];                                
	unsigned char Forw_forwarded_number_ton[1];                         
	unsigned char Forw_forwarded_msrn_ton[1];                           
	unsigned char Forw_forwarded_msrn[12];                              
	unsigned char Forw_facility_usage[4];                               
	unsigned char Forw_forw_mcz_chrg_type[1];
	unsigned char Forw_forwarding_number_ton[1];                        
	unsigned char Forw_orig_calling_number_ton[1];                      
	unsigned char Forw_routing_category[1];                             
	unsigned char Forw_intermediate_chrg_cause[2];                      
	unsigned char Forw_camel_call_reference[8];                         
	unsigned char Forw_camel_exchange_id_ton[1];                        
	unsigned char Forw_camel_exchange_id[9];                            
	unsigned char Forw_orig_dialling_class[2];                          
	unsigned char Forw_virtual_msc_id[10];                              
	unsigned char Forw_scf_address_ton[1];                              
	unsigned char Forw_scf_address[9];                                  
	unsigned char Forw_destination_number_ton[1];                       
	unsigned char Forw_destination_number_npi[1];                       
	unsigned char Forw_destination_number[16];                          
	unsigned char Forw_camel_service_key[4];                            
	unsigned char Forw_forwarding_imeisv[8];                            
	unsigned char Forw_forwarded_imeisv[8];                             
	unsigned char Forw_camel_service_data[56];
} SForwInOld;                                                         
  
typedef struct                                                        
{                                                                     
	unsigned char Forw_record_number[4];                                
	unsigned char Forw_record_status[1];                                
	unsigned char Forw_check_sum[2];                                    
	unsigned char Forw_call_reference[5];                               
	unsigned char Forw_exchange_id[10];                                 
	unsigned char Forw_intermediate_record_number[1];                   
	unsigned char Forw_intermediate_charging_ind[1];                    
	unsigned char Forw_number_of_ss_records[1];                         
	unsigned char Forw_cause_for_forwarding[1];                         
	unsigned char Forw_forwarding_imsi[8];                              
	unsigned char Forw_forwarding_imei[8];                              
	unsigned char Forw_forwarding_number[12];                           
	unsigned char Forw_forwarding_category[1];                          
	unsigned char Forw_forwarding_ms_classmark[1];                      
	unsigned char Forw_forwarded_imsi[8];                               
	unsigned char Forw_forwarded_imei[8];                               
	unsigned char Forw_forwarded_number[12];                            
	unsigned char Forw_forwarded_ms_classmark[1];                       
	unsigned char Forw_orig_calling_number[10];                         
	unsigned char Forw_in_circuit_group[2];                             
	unsigned char Forw_in_circuit[2];                                   
	unsigned char Forw_forwarding_subs_first_lac[2];                    
	unsigned char Forw_forwarding_subs_first_ci[2];                     
	unsigned char Forw_forwarding_subs_last_ex_id[10];                  
	unsigned char Forw_forwarding_subs_last_lac[2];                     
	unsigned char Forw_forwarding_subs_last_ci[2];                      
	unsigned char Forw_forwarded_subs_first_lac[2];                     
	unsigned char Forw_forwarded_subs_first_ci[2];                      
	unsigned char Forw_forwarded_subs_last_ex_id[10];                   
	unsigned char Forw_forwarded_subs_last_lac[2];                      
	unsigned char Forw_forwarded_subs_last_ci[2];                       
	unsigned char Forw_out_circuit_group[2];                            
	unsigned char Forw_out_circuit[2];                                  
	unsigned char Forw_basic_service_type[1];                           
	unsigned char Forw_basic_service_code[1];                           
	unsigned char Forw_non_transparency_indicator[1];                   
	unsigned char Forw_channel_rate_indicator[1];                       
	unsigned char Forw_set_up_start_time[7];                            
	unsigned char Forw_in_channel_allocated_time[7];                    
	unsigned char Forw_charging_start_time[7];                          
	unsigned char Forw_charging_end_time[7];                            
	unsigned char Forw_forw_mcz_duration[3];                            
	unsigned char Forw_cause_for_termination[4];                        
	unsigned char Forw_data_volume[2];                                  
	unsigned char Forw_call_type[1];                                    
	unsigned char Forw_forw_mcz_tariff_class[3];                        
	unsigned char Forw_forw_mcz_pulses[2];                              
	unsigned char Forw_dtmf_indicator[1];                               
	unsigned char Forw_aoc_indicator[1];                                
	unsigned char Forw_forwarded_number_ton[1];                         
	unsigned char Forw_forwarded_msrn_ton[1];                           
	unsigned char Forw_forwarded_msrn[12];                              
	unsigned char Forw_facility_usage[4];                               
	unsigned char Forw_forw_mcz_chrg_type[1];
	unsigned char Forw_forwarding_number_ton[1];                        
	unsigned char Forw_orig_calling_number_ton[1];                      
	unsigned char Forw_routing_category[1];                             
	unsigned char Forw_intermediate_chrg_cause[2];                      
	unsigned char Forw_camel_call_reference[8];                         
	unsigned char Forw_camel_exchange_id_ton[1];                        
	unsigned char Forw_camel_exchange_id[9];                            
	unsigned char Forw_orig_dialling_class[2];                          
	unsigned char Forw_virtual_msc_id[10];                              
	unsigned char Forw_scf_address_ton[1];                              
	unsigned char Forw_scf_address[9];                                  
	unsigned char Forw_destination_number_ton[1];                       
	unsigned char Forw_destination_number_npi[1];                       
	unsigned char Forw_destination_number[16];                          
	unsigned char Forw_camel_service_key[4];                            
	unsigned char Forw_forwarding_imeisv[8];                            
	unsigned char Forw_forwarded_imeisv[8];                             
	unsigned char Forw_rate_adaption[1];                                
	unsigned char Forw_ms_classmark3[1];                                
	unsigned char Forw_forwarding_cell_band[1];                         
	unsigned char Forw_forwarding_last_ex_id_ton[1];                    
	unsigned char Forw_forwarded_to_last_ex_id_ton[1];                  
	unsigned char Forw_forwarding_first_mcc[2];                         
	unsigned char Forw_forwarding_first_mnc[2];                         
	unsigned char Forw_forwarding_last_mcc[2];                          
	unsigned char Forw_forwarding_last_mnc[2];                          
	unsigned char Forw_forwarded_to_last_mcc[2];                        
	unsigned char Forw_forwarded_to_last_mnc[2];                        
	unsigned char Forw_camel_service_data[56];                          
	unsigned char Forw_inside_user_plane_index[2];                      
	unsigned char Forw_inside_control_plane_index[2];                   
	unsigned char Forw_in_bnc_connection_type[1];                       
	unsigned char Forw_outside_user_plane_index[2];                     
	unsigned char Forw_outside_control_plane_index[2];                  
	unsigned char Forw_out_bnc_connection_type[1];                      
	unsigned char Forw_radio_network_type[1];                                   
} SForwIn;
   
//Mobile-originated forward Out-Data                                  
typedef struct                                                        
{                                                                     
	unsigned int unForw_record_length;                                  
	eRecordType  eForw_record_type;                                     
	char         szForw_record_number[9];                               
	char         szForw_record_status[20];                              
	unsigned int unForw_check_sum;                                      
	char         szForw_call_reference[32];                             
	char         szForw_exchange_id[21];                                
	char         szForw_intermediate_record_number[3];                  
	unsigned int unForw_intermediate_charging_ind;                      
	char         szForw_number_of_ss_records[3];                        
	unsigned int unForw_cause_for_forwarding;                           
	char         szForw_forwarding_imsi[17];                            
	char         szForw_forwarding_imei[17];                            
	char         szForw_forwarding_number[25];                          
	unsigned int unForw_forwarding_category;                            
	unsigned int unForw_forwarding_ms_classmark;                        
	char         szForw_forwarded_imsi[17];                             
	char         szForw_forwarded_imei[17];                             
	char         szForw_forwarded_number[25];                           
	unsigned int unForw_forwarded_ms_classmark;                         
	char         szForw_orig_calling_number[21];                        
	char         szForw_in_circuit_group[5];                            
	char         szForw_in_circuit[5];                                  
	unsigned int unForw_forwarding_subs_first_lac;                      
	unsigned int unForw_forwarding_subs_first_ci;                       
	char         szForw_forwarding_subs_last_ex_id[21];                 
	unsigned int unForw_forwarding_subs_last_lac;                       
	unsigned int unForw_forwarding_subs_last_ci;                        
	unsigned int unForw_forwarded_subs_first_lac;                       
	unsigned int unForw_forwarded_subs_first_ci;                        
	char         szForw_forwarded_subs_last_ex_id[21];                  
	unsigned int unForw_forwarded_subs_last_lac;                        
	unsigned int unForw_forwarded_subs_last_ci;                         
	char         szForw_out_circuit_group[5];                           
	char         szForw_out_circuit[5];                                 
	unsigned int unForw_basic_service_type;                             
	unsigned int unForw_basic_service_code;                             
	unsigned int unForw_non_transparency_indicator;                     
	unsigned int unForw_channel_rate_indicator;                         
	char         szForw_set_up_start_time[20];                          
	char         szForw_in_channel_allocated_time[20];                  
	char         szForw_charging_start_time[20];                        
	char         szForw_charging_end_time[20];                          
	unsigned int unForw_forw_mcz_duration;                              
	unsigned int unForw_cause_for_termination;                          
	char         szForw_data_volume[5];                                 
	unsigned int unForw_call_type;                                      
	char         szForw_forw_mcz_tariff_class[7];                       
	char         szForw_forw_mcz_pulses[5];                             
	unsigned int unForw_dtmf_indicator;                                 
	unsigned int unForw_aoc_indicator;                                  
	unsigned int unForw_forwarded_number_ton;                           
	unsigned int unForw_forwarded_msrn_ton;                             
	char         szForw_forwarded_msrn[25];                             
	char         szForw_facility_usage[9];                              
	unsigned int unForw_forw_mcz_chrg_type;                             
	unsigned int unForw_forwarding_number_ton;                          
	unsigned int unForw_orig_calling_number_ton;                        
	unsigned int unForw_routing_category;                               
	unsigned int unForw_intermediate_chrg_cause;                        
	char         szForw_camel_call_reference[17];                       
	unsigned int unForw_camel_exchange_id_ton;                          
	char         szForw_camel_exchange_id[19];                          
	unsigned int unForw_orig_dialling_class;                            
	char         szForw_virtual_msc_id[21];                             
	unsigned int unForw_scf_address_ton;                                
	char         szForw_scf_address[19];                                
	unsigned int unForw_destination_number_ton;                         
	unsigned int unForw_destination_number_npi;                         
	char         szForw_destination_number[33];                         
	char         szForw_camel_service_key[9];                           
	char         szForw_forwarding_imeisv[17];                          
	char         szForw_forwarded_imeisv[17];                           
	unsigned int unForw_rate_adaption;                                  
	unsigned int unForw_ms_classmark3;                                  
	unsigned int unForw_forwarding_cell_band;                           
	unsigned int unForw_forwarding_last_ex_id_ton;                      
	unsigned int unForw_forwarded_to_last_ex_id_ton;                    
	unsigned int unForw_forwarding_first_mcc;                           
	unsigned int unForw_forwarding_first_mnc;                           
	unsigned int unForw_forwarding_last_mcc;                            
	unsigned int unForw_forwarding_last_mnc;                            
	unsigned int unForw_forwarded_to_last_mcc;                          
	unsigned int unForw_forwarded_to_last_mnc;                          
	char         szForw_camel_service_data[113];                        
	unsigned int unForw_inside_user_plane_index;                        
	unsigned int unForw_inside_control_plane_index;                     
	unsigned int unForw_in_bnc_connection_type;                         
	unsigned int unForw_outside_user_plane_index;                       
	unsigned int unForw_outside_control_plane_index;                    
	unsigned int unForw_out_bnc_connection_type;                        
	unsigned int unForw_radio_network_type;                             
} SForwOut;                                                           
   
typedef struct                                                        
{              
	unsigned char Roam_record_number[4];                                
	unsigned char Roam_record_status[1];                                
	unsigned char Roam_check_sum[2];                                    
	unsigned char Roam_call_reference[5];                               
	unsigned char Roam_exchange_id[10];                                 
	unsigned char Roam_intermediate_record_number[1];                   
	unsigned char Roam_intermediate_charging_ind[1];                    
	unsigned char Roam_number_of_ss_records[1];                         
	unsigned char Roam_calling_number[10];                              
	unsigned char Roam_called_imsi[8];                                  
	unsigned char Roam_called_number[12];                               
	unsigned char Roam_called_category[1];                              
	unsigned char Roam_called_msrn[12];                                 
	unsigned char Roam_in_circuit_group[2];                             
	unsigned char Roam_in_circuit[2];                                   
	unsigned char Roam_out_circuit_group[2];                            
	unsigned char Roam_out_circuit[2];                                  
	unsigned char Roam_basic_service_type[1];                           
	unsigned char Roam_basic_service_code[1];                           
	unsigned char Roam_set_up_start_time[7];                            
	unsigned char Roam_in_channel_allocated_time[7];                    
	unsigned char Roam_charging_start_time[7];                          
	unsigned char Roam_charging_end_time[7];                            
	unsigned char Roam_roam_mcz_duration[3];                            
	unsigned char Roam_cause_for_termination[4];                        
	unsigned char Roam_data_volume[2];                                  
	unsigned char Roam_call_type[1];                                    
	unsigned char Roam_roam_mcz_tariff_class[3];                        
	unsigned char Roam_roam_mcz_pulses[2];                              
	unsigned char Roam_called_msrn_ton[1];                              
	unsigned char Roam_facility_usage[4];                               
	unsigned char Roam_roam_mcz_chrg_type[1];                           
	unsigned char Roam_calling_number_ton[1];                           
	unsigned char Roam_called_number_ton[1];                            
	unsigned char Roam_routing_category[1];                             
	unsigned char Roam_cf_information[1];                               
	unsigned char Roam_intermediate_chrg_cause[2];                      
	unsigned char Roam_camel_call_reference[8];                         
	unsigned char Roam_camel_exchange_id_ton[1];                        
	unsigned char Roam_camel_exchange_id[9];                            
} SRoamInOld;                                                         
                                                                      
typedef struct                                                        
{              
	unsigned char Roam_record_number[4];                                
	unsigned char Roam_record_status[1];                                
	unsigned char Roam_check_sum[2];                                    
	unsigned char Roam_call_reference[5];                               
	unsigned char Roam_exchange_id[10];                                 
	unsigned char Roam_intermediate_record_number[1];                   
	unsigned char Roam_intermediate_charging_ind[1];                    
	unsigned char Roam_number_of_ss_records[1];                         
	unsigned char Roam_calling_number[10];                              
	unsigned char Roam_called_imsi[8];                                  
	unsigned char Roam_called_number[12];                               
	unsigned char Roam_called_category[1];                              
	unsigned char Roam_called_msrn[12];                                 
	unsigned char Roam_in_circuit_group[2];                             
	unsigned char Roam_in_circuit[2];                                   
	unsigned char Roam_out_circuit_group[2];                            
	unsigned char Roam_out_circuit[2];                                  
	unsigned char Roam_basic_service_type[1];                           
	unsigned char Roam_basic_service_code[1];                           
	unsigned char Roam_set_up_start_time[7];                            
	unsigned char Roam_in_channel_allocated_time[7];                    
	unsigned char Roam_charging_start_time[7];                          
	unsigned char Roam_charging_end_time[7];                            
	unsigned char Roam_roam_mcz_duration[3];                            
	unsigned char Roam_cause_for_termination[4];                        
	unsigned char Roam_data_volume[2];                                  
	unsigned char Roam_call_type[1];                                    
	unsigned char Roam_roam_mcz_tariff_class[3];                        
	unsigned char Roam_roam_mcz_pulses[2];                              
	unsigned char Roam_called_msrn_ton[1];                              
	unsigned char Roam_facility_usage[4];                               
	unsigned char Roam_roam_mcz_chrg_type[1];                           
	unsigned char Roam_calling_number_ton[1];                           
	unsigned char Roam_called_number_ton[1];                            
	unsigned char Roam_routing_category[1];                             
	unsigned char Roam_cf_information[1];                               
	unsigned char Roam_intermediate_chrg_cause[2];                      
	unsigned char Roam_camel_call_reference[8];                         
	unsigned char Roam_camel_exchange_id_ton[1];                        
	unsigned char Roam_camel_exchange_id[9];                        
 	unsigned char Roam_rate_adaption[1];                            
 	unsigned char Roam_selected_codec[1];                           
 	unsigned char Roam_inside_user_plane_index[2];                  
 	unsigned char Roam_inside_control_plane_index[2];               
	unsigned char Roam_in_bnc_connection_type[1];                   
 	unsigned char Roam_outside_user_plane_index[2];                 
 	unsigned char Roam_outside_control_plane_index[2];              
 	unsigned char Roam_out_bnc_connection_type[1];                  
} SRoamIn;                                                        
   
//Call to a roaming subscriber Out-Data                           
typedef struct                                                    
{                                                                 
	unsigned int unRoam_record_length;                                  
	eRecordType  eRoam_record_type;                                     
	char         szRoam_record_number[9];                               
	char         szRoam_record_status[20];                              
	unsigned int unRoam_check_sum;                                      
	char         szRoam_call_reference[32];                             
	char         szRoam_exchange_id[21];                                
	char         szRoam_intermediate_record_number[3];                  
	unsigned int unRoam_intermediate_charging_ind;                      
	char         szRoam_number_of_ss_records[3];                        
	char         szRoam_calling_number[21];                             
	char         szRoam_called_imsi[17];                                
	char         szRoam_called_number[25];                              
	unsigned int unRoam_called_category;                                
	char         szRoam_called_msrn[25];                                
	char         szRoam_in_circuit_group[5];                            
	char         szRoam_in_circuit[5];                                  
	char         szRoam_out_circuit_group[5];                           
	char         szRoam_out_circuit[5];                                 
	unsigned int unRoam_basic_service_type;                             
	unsigned int unRoam_basic_service_code;                             
	char         szRoam_set_up_start_time[20];                          
	char         szRoam_in_channel_allocated_time[20];                  
	char         szRoam_charging_start_time[20];                        
	char         szRoam_charging_end_time[20];                          
	unsigned int unRoam_roam_mcz_duration;                              
	unsigned int unRoam_cause_for_termination;                          
	char         szRoam_data_volume[5];                                 
	unsigned int unRoam_call_type;                                      
	char         szRoam_roam_mcz_tariff_class[7];                       
	char         szRoam_roam_mcz_pulses[5];                             
	unsigned int unRoam_called_msrn_ton;                          
	char         szRoam_facility_usage[9];                        
	unsigned int unRoam_roam_mcz_chrg_type;                       
	unsigned int unRoam_calling_number_ton;                       
	unsigned int unRoam_called_number_ton;                        
	unsigned int unRoam_routing_category;                         
	unsigned int unRoam_cf_information;                           
	unsigned int unRoam_intermediate_chrg_cause;                  
	char         szRoam_camel_call_reference[17];                 
	unsigned int unRoam_camel_exchange_id_ton;                    
	char         szRoam_camel_exchange_id[19];                    
	unsigned int unRoam_rate_adaption;                                
	unsigned int unRoam_selected_codec;                               
	unsigned int unRoam_inside_user_plane_index;                      
	unsigned int unRoam_inside_control_plane_index;                   
	unsigned int unRoam_in_bnc_connection_type;                       
	unsigned int unRoam_outside_user_plane_index;                     
	unsigned int unRoam_outside_control_plane_index;                  
	unsigned int unRoam_out_bnc_connection_type;                      
} SRoamOut;                                                     
                                                                
//Supplementary service                                         
typedef struct                                                  
{                                                               
	char Sups_Data[157];
} SSupsIn;  
                                                               
//Supplementary service                                                
typedef struct                                                         
{                                                                      
	char Sups_Data[150];
} SSupsInOld;                                                                 
                                                                       
//Supplementary service  8                                              
typedef struct                                                         
{                                                                      
	char Smmo_Data[144];
} SSmmoIn;    
                                                             
//Supplementary service  8                                              
typedef struct                                                         
{                                                                      
	char Smmo_Data[124];
} SSmmoInOld;                                                                 
                                                                       
//Mobile-terminated short message service                              
typedef struct                                                         
{                                                                      
	char Smmt_Data[146];
} SSmmtIn;      
                                                        
//Mobile-terminated short message service                              
typedef struct                                                         
{                                                                      
	char Smmt_Data[119];
} SSmmtInOld;                                                              
                                                                       
//PSTN-originated call                                                 
typedef struct                                                         
{                                                                      
	char Poc_Data[171];
} SPocIn;                                                                
 
//PSTN-originated call                                                 
typedef struct                                                         
{                                                                      
	char Poc_Data[164];
} SPocInOld;                                                                
                                                                      
//PSTN-terminated call                                                 
typedef struct                                                         
{                                                                      
	char Ptc_Data[119];
} SPtcIn;                                                               

//PSTN-terminated call                                                 
typedef struct                                                         
{                                                                      
	char Ptc_Data[112];
} SPtcInOld;                                                               
                                                                      
//Device-originated call                                              
typedef struct                                                        
{                                                                     
	char Doc_Data[90];
} SDocIn;                                                                
                                                                       
//Camel-originated call                                                
typedef struct                                                         
{                                                                      
	char Coc_Data[117];
} SCocIn;                                                                
                                                                       
//Camel-terminated call                                                
typedef struct                                                         
{                                                                      
	char Ctc_Data[103];
} SCtcIn;                                                                
                                                                       
//Intelligent network data 4 (data sent by Camel-SCP)                  
typedef struct                                                         
{                                                                      
	char In4_Data[93];
} SIn4In;                                                                

                                                                       
/************************************************************************/
/* 函数声明 */                                                         
/************************************************************************/
//parse函数(核心逻辑)/* module interface */
int nokia(char * in_file_name, char * out_file_name, int * rec_num);
//输出文件函数
static int GenerateMocCsvFile(char* lpSavePath, char* lpFileString, char* lpSrcFileName, SMocOut* pMocOut, FILE ** pFile, char* lpDevice);
//输出文件函数
static int GenerateMtcCsvFile(char* lpSavePath, char* lpFileString, char* lpSrcFileName, SMtcOut* pMtcOut, FILE ** pFile, char* lpDevice);
//输出文件函数
static int GenerateForwCsvFile(char* lpSavePath, char* lpFileString, char* lpSrcFileName, SForwOut* pForwOut, FILE ** pFile, char* lpDevice);
//输出文件函数
static int GenerateRoamCsvFile(char* lpSavePath,  char* lpFileString, char* lpSrcFileName, SRoamOut* pRoamOut, FILE ** pFile, char* lpDevice);
//Header解析函数
int ParseHeader(SHeaderOut* pHeaderOut, SHeaderIn* pHeaderIn, SSegHandleOut* pSegHandleOut );
//Trailer解析函数
int ParseTrailer(STrailerOut* pTrailerOut, STrailerIn* pTrailerIn, SSegHandleOut* pSegHandleOut );
//Moc解析函数
int ParseMoc(SMocOut* pMocOut, SMocIn* pMocIn, SSegHandleOut* pSegHandleOut );
//MocOld解析函数
int ParseMocOld(SMocOut* pMocOut, SMocInOld* pMocInOld, SSegHandleOut* pSegHandleOut );
//Mtc解析函数
int ParseMtc(SMtcOut* pMtcOut, SMtcIn* pMtcIn, SSegHandleOut* pSegHandleOut );
//MtcOld解析函数
int ParseMtcOld(SMtcOut* pMtcOut, SMtcInOld* pMtcInOld, SSegHandleOut* pSegHandleOut );
//Forw解析函数
int ParseForw(SForwOut* pForwOut, SForwIn* pForwIn, SSegHandleOut* pSegHandleOut );                                
//ForwOld解析函数
int ParseForwOld(SForwOut* pForwOut, SForwInOld* pForwInOld, SSegHandleOut* pSegHandleOut );                                
//Forw解析函数
int ParseRoam(SRoamOut* pRoamOut, SRoamIn* pRoamIn, SSegHandleOut* pSegHandleOut );                                
//ForwOld解析函数
int ParseRoamOld(SRoamOut* pRoamOut, SRoamInOld* pRoamInOld, SSegHandleOut* pSegHandleOut );                                
//char2 To int
static int CharToUInt(unsigned int * npOut, unsigned char* lpIn, int nLen);
//Char7 To DateTime
static int CharToDateTime(char * lpOut, unsigned char * lpIn);
//CharToStringForward
static int CharToStringForward(char * lpOut, unsigned char * lpIn, int nLen);
//CharToStringReverse
static int CharToStringReverse(char * lpOut, unsigned char * lpIn, int nLen);
//CharToCallNumber
static int CharToCallNumber(char * lpOut, unsigned char * lpIn, int nLen);
//CharToBcdDword
// static int CharToBcdDword(char * lpOut, unsigned char * lpIn);
//CharToBcdHalfWord
static int CharToBcdHalfWord(char * lpOut, unsigned char * lpIn, int nLen);

/************************************************************************/
/* 函数实现 */
/************************************************************************/   

/************************************************************************/
/* parse函数(核心逻辑) */
/* 解析函数 */
/* Parameters: in_file_name 输入文件名 */
/* Parameters: out_file_name 输入:存储路径 */
/* Parameters: out_file_name 输出:存储路径:生成文件名;生成文件名;生成文件名;生成文件名; */
/* Parameters: rec_num 解析总数 */
/************************************************************************/
int nokia(char* in_file_name, char* out_file_name, int * rec_num)
{ 
	/************************************************************************/
	/* 逻辑 */
	/* 打开文件 */
	/* 读取头 */
	/* 解析头 */
	/* 解析部分 */
	/* 保存解析结果 */
	/* 循环 */
	/************************************************************************/
		
	/************************************************************************/
	/* 变量申明 */
	/************************************************************************/
	//返回值
	int nRe = 0 ; 
	//输入文件
	FILE *pInFile = NULL;
	//输出文件
	FILE *pOutMocFile = NULL;
	FILE *pOutMtcFile = NULL;
	FILE *pOutForwFile = NULL;
	FILE *pOutRoamFile = NULL;
	//用到的结构体
	SSegHandleIn oSegHandleIn;
	SSegHandleOut oSegHandleOut;
	SHeaderIn oHeaderIn;
 	STrailerIn oTrailerIn;
	SMocIn oMocIn;
	SMocInOld oMocInOld;
	SMocOut oMocOut;
	SMtcIn oMtcIn;
	SMtcInOld oMtcInOld;
 	SMtcOut oMtcOut;
	SForwIn oForwIn;
	SForwInOld oForwInOld;
	SForwOut oForwOut;
	SRoamIn oRoamIn;
	SRoamInOld oRoamInOld;
	SRoamOut oRoamOut;
	SSupsIn oSupsIn;
	SSupsInOld oSupsInOld;
	SSmmoIn oSmmoIn;
	SSmmoInOld oSmmoInOld;
	SSmmtIn oSmmtIn;
	SSmmtInOld oSmmtInOld;
	SPocIn oPocIn;
	SPocInOld oPocInOld;
	SPtcIn oPtcIn;
	SPtcInOld oPtcInOld;
	SDocIn oDocIn;
	SCocIn oCocIn;
	SCtcIn oCtcIn;
	SIn4In oIn4In;
	//保存目录
	char szSavePath[256];
	char szSrcFileName[256];
	//设备名
	char szDevice[32];
	char szTempInFileName[256];
	char* lpTempDevice1;
	char* lpTempDevice2;
	int nI;
	//寻头模式开关(默认打开)
	int nFindHeadFlag = 1;
	/************************************************************************/
	/* 打开文件 */
	/************************************************************************/
	//打开输入文件
	if ( NULL == ( pInFile = fopen( in_file_name , "rb" ) ) )
	{ 
		//文件打开失败
		nRe = 0 ; 
		goto Exit;
	} 

	//拷贝路径
	strcpy(szSavePath, out_file_name);
	strcpy(szSrcFileName, in_file_name);
	strcat(out_file_name, ":");
	//解析设备名
	lpTempDevice1 = szTempInFileName;
	strcpy(szTempInFileName,in_file_name);
	while( NULL != ( lpTempDevice2 = strstr(lpTempDevice1, "/") ) )
	{
		lpTempDevice2 ++ ;
		lpTempDevice1 = lpTempDevice2;
	}
	lpTempDevice2 = lpTempDevice1;
	for ( nI = 0 ; nI < 3; nI ++)
	{
		if (NULL == ( lpTempDevice2 = strstr(lpTempDevice2, "_") ))
		{
			//文件名有问题
			nRe = 0 ; 
			goto Exit;				
		}
		lpTempDevice2 ++;
	}
	lpTempDevice2 --;
	lpTempDevice2[0] = '\0';
	lpTempDevice2 = lpTempDevice1;
	for ( nI = 0 ; nI < 2; nI ++)
	{
		if (NULL == ( lpTempDevice2 = strstr(lpTempDevice2, "_") ))
		{
			//文件名有问题
			nRe = 0 ; 
			goto Exit;				
		}
		lpTempDevice2 ++;
	}
	lpTempDevice2 --;
	lpTempDevice2[0] = '\0';
	lpTempDevice2 ++;
	strcpy(szDevice , lpTempDevice2);
	/************************************************************************/
	/* 读入然后解析 */
	/************************************************************************/
	while ( !feof(pInFile) )
	{
		if (1 == nFindHeadFlag)
		{
		//寻头模式
			if( 0x29 == fgetc(pInFile) && 0x00 == fgetc(pInFile) && 0x00 == fgetc(pInFile) )
			{
				//找到了头
				nFindHeadFlag = 0;
				fseek(pInFile,-3,SEEK_CUR);
				continue;
			}
		} 
		else
		{
		//读块模式
			//读长度和类型
			if(0 == fread((char*)&oSegHandleIn, sizeof(oSegHandleIn) , 1, pInFile))
			{
				nRe = 0;
			}
			//判断并解析
			if ( 0 != CharToUInt( &oSegHandleOut.unRecord_length, oSegHandleIn.Record_length, 2) )
			{
				nRe = 0;
			}
			
			switch( oSegHandleOut.eRecord_type = oSegHandleIn.Record_type[0] )
			{
			case eRecordType_Header:
				if ( 41 != oSegHandleOut.unRecord_length)
				{
					nRe = 0;
				}
				else
				{
					//读内容
					if(0 == fread((char*)&oHeaderIn, sizeof(oHeaderIn) , 1,  pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
				}
				break;
			case eRecordType_Moc:
				if (396 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oMocIn, sizeof(oMocIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					if ( 0 != ParseMoc( &oMocOut, &oMocIn, &oSegHandleOut) )
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("ParseMoc 失败\n");
						}
						goto Exit;
					}
					//写内容
					if ( 0 != GenerateMocCsvFile(szSavePath, out_file_name, szSrcFileName, &oMocOut, &pOutMocFile, szDevice))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("GenerateMocCsvFile 失败\n");
						}
						goto Exit;
					}
					(*rec_num) += 1;
				}
				else if (362 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oMocInOld, sizeof(oMocInOld) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					if ( 0 != ParseMocOld( &oMocOut, &oMocInOld, &oSegHandleOut ) )
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("ParseMocOld 失败\n");
						}
						goto Exit;
					}
					//写内容
					if ( 0 != GenerateMocCsvFile(szSavePath, out_file_name, szSrcFileName, &oMocOut, &pOutMocFile, szDevice))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("GenerateMocCsvFile 失败\n");
						}
						goto Exit;
					}
					(*rec_num) += 1;
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Moc 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Mtc:
				if (239 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oMtcIn, sizeof(oMtcIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					if ( 0 != ParseMtc( &oMtcOut, &oMtcIn, &oSegHandleOut ) )
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("ParseMtc 失败\n");
						}
						goto Exit;
					}
					//写内容
					if ( 0 != GenerateMtcCsvFile(szSavePath, out_file_name, szSrcFileName, &oMtcOut, &pOutMtcFile, szDevice))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("GenerateMocCsvFile 失败\n");
						}
						goto Exit;
					}				
					(*rec_num) += 1;
				}
				else if (215 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oMtcInOld, sizeof(oMtcInOld) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					if ( 0 != ParseMtcOld( &oMtcOut, &oMtcInOld, &oSegHandleOut ) )
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("ParseMtcOld 失败\n");
						}
						goto Exit;
					}
					//写内容
					if ( 0 != GenerateMtcCsvFile(szSavePath, out_file_name, szSrcFileName, &oMtcOut, &pOutMtcFile, szDevice))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("GenerateMocCsvFile 失败\n");
						}
						goto Exit;
					}				
					(*rec_num) += 1;
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Mtc 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Forw:
				if ( 377 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oForwIn, sizeof(oForwIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					if ( 0 != ParseForw( &oForwOut, &oForwIn, &oSegHandleOut ) )
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("ParseForw 失败\n");
						}
						goto Exit;
					}
					//写内容
					if ( 0 != GenerateForwCsvFile(szSavePath, out_file_name, szSrcFileName, &oForwOut, &pOutForwFile, szDevice))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("GenerateForwCsvFile 失败\n");
						}
						goto Exit;
					}				
					(*rec_num) += 1;
				}
				else if ( 349 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oForwInOld, sizeof(oForwInOld) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					if ( 0 != ParseForwOld( &oForwOut, &oForwInOld, &oSegHandleOut ) )
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("ParseForwOld 失败\n");
						}
						goto Exit;
					}
					//写内容
					if ( 0 != GenerateForwCsvFile(szSavePath, out_file_name, szSrcFileName, &oForwOut, &pOutForwFile, szDevice))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("GenerateForwCsvFile 失败\n");
						}
						goto Exit;
					}				
					(*rec_num) += 1;
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Forw 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Roam:
				if ( 166 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oRoamIn, sizeof(oRoamIn), 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					if ( 0 != ParseRoam( &oRoamOut, &oRoamIn, &oSegHandleOut ) )
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("ParseRoam 失败\n");
						}
						goto Exit;
					}
					//写内容
					if ( 0 != GenerateRoamCsvFile(szSavePath, out_file_name, szSrcFileName, &oRoamOut, &pOutRoamFile, szDevice))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("GenerateRoamCsvFile 失败\n");
						}
						goto Exit;
					}	
					(*rec_num) += 1;
				}
				else if(154 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oRoamInOld, sizeof(oRoamInOld), 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					if ( 0 != ParseRoamOld( &oRoamOut, &oRoamInOld, &oSegHandleOut ) )
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("ParseRoamOld 失败\n");
						}
						goto Exit;
					}
					//写内容
					if ( 0 != GenerateRoamCsvFile(szSavePath, out_file_name, szSrcFileName, &oRoamOut, &pOutRoamFile, szDevice))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("GenerateRoamCsvFile 失败\n");
						}
						goto Exit;
					}	
					(*rec_num) += 1;
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Roam 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Sups:
				if ( 160 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oSupsIn, sizeof(oSupsIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容					nRe = 0;
				}
				else if (153 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oSupsInOld, sizeof(oSupsInOld) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容					nRe = 0;
				}
				else
				{
					if (g_nDebug)
					{
						printf("eRecordType_Sups 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Smmo:
				if ( 147 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oSmmoIn, sizeof(oSmmoIn), 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容					nRe = 0;
				}
				else if(127 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oSmmoInOld, sizeof(oSmmoInOld), 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容
				}
				else
				{
					if (g_nDebug)
					{
						printf("eRecordType_Smmo 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Smmt:
				if ( 149 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oSmmtIn, sizeof(oSmmtIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容
				}
				else if (122 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oSmmtInOld, sizeof(oSmmtInOld) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Smmt 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Poc:
				if ( 174 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oPocIn, sizeof(oPocIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容				
				}
				else if (167 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oPocInOld, sizeof(oPocInOld) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容				
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Poc 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Ptc:
				if ( 122 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oPtcIn, sizeof(oPtcIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容			
				}
				else if (115 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oPtcInOld, sizeof(oPtcInOld) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Ptc 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Doc:
				if ( 93 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oDocIn, sizeof(oDocIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Doc 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Coc:
				if ( 120 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oCocIn, sizeof(oCocIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容
				}
				else
				{				
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Coc 长度不符合\n");
					}
					goto Exit;	
				}
				break;
			case eRecordType_Ctc:
				if ( 106 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oCtcIn, sizeof(oCtcIn), 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Ctc 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_In4:
				if ( 96 == oSegHandleOut.unRecord_length)
				{
					//读内容
					if(0 == fread((char*)&oIn4In, sizeof(oIn4In), 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
					//解析
					//写内容
				}
				else
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_In4 长度不符合\n");
					}
					goto Exit;
				}
				break;
			case eRecordType_Trailer:
				if ( (24) != oSegHandleOut.unRecord_length)
				{
					nRe = 0;
					if (g_nDebug)
					{
						printf("eRecordType_Moc 长度不符合\n");
					}
					goto Exit;
				}
				else
				{
					//读内容
					if(0 == fread((char*)&oTrailerIn, sizeof(oTrailerIn) , 1, pInFile))
					{
						nRe = 0;
						if (g_nDebug)
						{
							printf("fread 读取失败\n");
						}
						goto Exit;
					}
                    //改变为寻头模式
					nFindHeadFlag = 1;
				}
				break;
			case eRecordType_END:
				
				if (g_nDebug)
				{
					printf("eRecordType_END\n");
				}
				goto Exit;
				break;
				
			default:
				nRe = 0;
				if (g_nDebug)
				{
					printf("无法识别的类型\n");
				}
				goto Exit;
				break;
			}
		}
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	if ( pInFile != NULL)
	{
		fclose( pInFile );
	}
	if ( pOutMocFile != NULL)
	{
		fclose( pOutMocFile );
	}
	if ( pOutMtcFile != NULL)
	{
		fclose( pOutMtcFile );
	}
	if ( pOutForwFile != NULL)
	{
		fclose( pOutForwFile );
	}
	if ( pOutRoamFile != NULL)
	{
		fclose( pOutRoamFile );
	}
	return nRe;
}

/************************************************************************/
/* 解析Header函数 */
/* Parameters: pHeaderOut 输出结构体 */
/* Parameters: pHeaderIn 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/* Return Value: int  */
/* 0 - 解析成功  */
/* 1 - 输入文件打开失败  */
/* 2 - 输出文件打开失败  */
/* 3 - success  */
/************************************************************************/
int ParseHeader(SHeaderOut* pHeaderOut, SHeaderIn* pHeaderIn, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	pHeaderOut->unHea_record_length = pSegHandleOut->unRecord_length;
	pHeaderOut->eHea_record_type = pSegHandleOut->eRecord_type;

	//Hea_charging_block_size
	pHeaderOut->szHea_charging_block_size[2] = 'k';
	pHeaderOut->szHea_charging_block_size[3] = 'B';
	pHeaderOut->szHea_charging_block_size[4] = '\0';
	switch(pHeaderIn->Hea_charging_block_size[0])
	{
	case 0x00:
		pHeaderOut->szHea_charging_block_size[0] = '2';
		pHeaderOut->szHea_charging_block_size[1] = ' ';
		break;
	case 0x01:
		pHeaderOut->szHea_charging_block_size[0] = '8';
		pHeaderOut->szHea_charging_block_size[1] = ' ';
		break;
	case 0x02:
		pHeaderOut->szHea_charging_block_size[0] = '1';
		pHeaderOut->szHea_charging_block_size[1] = '6';
		break;
	case 0x04:
		pHeaderOut->szHea_charging_block_size[0] = '3';
		pHeaderOut->szHea_charging_block_size[1] = '2';
		break;
	case 0x08:
		pHeaderOut->szHea_charging_block_size[0] = '6';
		pHeaderOut->szHea_charging_block_size[1] = '4';
		break;
	default:
		nRe = 1;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pHeaderOut->szHea_charging_block_size);
	}

	//Hea_tape_block_type
	if (0x01 != pHeaderIn->Hea_tape_block_type[0] || 0x00 != pHeaderIn->Hea_tape_block_type[1])
	{
		nRe = 2;
		goto Exit;
	}
	else
	{
		pHeaderOut->unHea_tape_block_type = 0x0001;
	}

	//Hea_data_length_in_block
	if ( 0 != CharToUInt(&pHeaderOut->unHea_data_length_in_block, pHeaderIn->Hea_data_length_in_block, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pHeaderOut->unHea_data_length_in_block);
	}

	//Hea_exchange_id
	if (0 != CharToStringReverse(pHeaderOut->szHea_exchange_id, pHeaderIn->Hea_exchange_id, 10) )
	{
		nRe = 4;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pHeaderOut->szHea_exchange_id);
	}

	//Hea_first_record_number
	if (0 != CharToBcdHalfWord(pHeaderOut->szHea_first_record_number, pHeaderIn->Hea_first_record_number, 4) )
	{
		nRe = 5;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pHeaderOut->szHea_first_record_number);
	}
	//Hea_batch_seq_number
	if (0 != CharToBcdHalfWord(pHeaderOut->szHea_batch_seq_number, pHeaderIn->Hea_batch_seq_number, 4) )
	{
		nRe = 6;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pHeaderOut->szHea_batch_seq_number);
	}

	//Hea_block_seq_number
	if (0 != CharToBcdHalfWord(pHeaderOut->szHea_block_seq_number, pHeaderIn->Hea_block_seq_number, 2) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pHeaderOut->szHea_block_seq_number);
	}
	//Hea_start_time
	if( 0 != CharToDateTime(pHeaderOut->szHea_start_time, pHeaderIn->Hea_start_time) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pHeaderOut->szHea_start_time);
	}

	//Hea_format_version
	if (0 != CharToStringForward(pHeaderOut->lpHea_format_version, pHeaderIn->Hea_format_version, 6) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pHeaderOut->lpHea_format_version);
	}


/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}                                                     

/************************************************************************/
/* 解析Roam函数 */
/* Parameters: pRoamOut 输出结构体 */
/* Parameters: pRoamInOld 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/* Return Value: int  */
/* 0 - 解析成功  */
/* 1 - 输入文件打开失败  */
/* 2 - 输出文件打开失败  */
/* 3 - success  */
/************************************************************************/
int ParseRoamOld(SRoamOut* pRoamOut, SRoamInOld* pRoamInOld, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	char* pTemp1 = NULL;
	unsigned char* pTemp2 = NULL;
	char szTemp[255];
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	memset(pRoamOut,0,sizeof(SRoamOut));
	//Roam_record_length
 	pRoamOut->unRoam_record_length = pSegHandleOut->unRecord_length;
 	//Roam_record_type
 	pRoamOut->eRoam_record_type = pSegHandleOut->eRecord_type;

 	//Roam_record_number
	if (0 != CharToBcdHalfWord(pRoamOut->szRoam_record_number, pRoamInOld->Roam_record_number, 4) )
	{
		nRe = 1;
		goto Exit;
	}

	//Roam_record_status
	switch(pRoamInOld->Roam_record_status[0])
	{
	case 0x00:
		strcpy(pRoamOut->szRoam_record_status,"normal ok");
		break;
	case 0x01:
		strcpy(pRoamOut->szRoam_record_status,"synchronising error");
		break;
	case 0x02:
		strcpy(pRoamOut->szRoam_record_status,"different contents");
		break;
	default:
		nRe = 2;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pRoamOut->szRoam_record_status);
	}

	//Roam_check_sum
	if ( 0 != CharToUInt(&pRoamOut->unRoam_check_sum, pRoamInOld->Roam_check_sum, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_check_sum);
	}

	//Roam_call_reference
	//comp+process+focus
	pTemp1 = pRoamOut->szRoam_call_reference;
	pTemp2 = pRoamInOld->Roam_call_reference;
	strcpy(pTemp1, "comp:");
	pTemp1 += 5;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 4;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " process:");
	pTemp1 += 9;
	pTemp2 += 2;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 5;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " focus:");
	pTemp1 += 7;	
	pTemp2 += 2;
	if (0 != CharToStringForward(pTemp1, pTemp2, 1) )
	{
		nRe = 6;
		goto Exit;
	}
	pTemp1[2] = '\0';
	pTemp1 = NULL;
	pTemp2 = NULL;
	if (g_nDebug)
	{
		printf("%s\n",pRoamOut->szRoam_call_reference);
	}

	//Roam_exchange_id
	if (0 != CharToStringReverse(pRoamOut->szRoam_exchange_id, pRoamInOld->Roam_exchange_id, 10) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_exchange_id);
	}
	
	//Roam_intermediate_record_number
	if (0 != CharToStringReverse(pRoamOut->szRoam_intermediate_record_number, pRoamInOld->Roam_intermediate_record_number, 1) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_intermediate_record_number);
	}

	//Roam_intermediate_charging_ind
	if ( 0 != CharToUInt(&pRoamOut->unRoam_intermediate_charging_ind, pRoamInOld->Roam_intermediate_charging_ind, 1) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_intermediate_charging_ind);
	}

	//Roam_number_of_ss_records
	if (0 != CharToStringReverse(pRoamOut->szRoam_number_of_ss_records, pRoamInOld->Roam_number_of_ss_records, 1) )
	{
		nRe = 10;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_number_of_ss_records);
	}
	//Roam_calling_number
	if (0 != CharToCallNumber(pRoamOut->szRoam_calling_number, pRoamInOld->Roam_calling_number, 10) )
	{
		nRe = 11;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_calling_number);
	}
	
	//Roam_called_imsi
	if (0 != CharToStringReverse(pRoamOut->szRoam_called_imsi, pRoamInOld->Roam_called_imsi, 8) )
	{
		nRe = 12;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_called_imsi);
	}
	
	//szRoam_called_number
	if (0 != CharToCallNumber(pRoamOut->szRoam_called_number, pRoamInOld->Roam_called_number, 12) )
	{
		nRe = 13;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_called_number);
	}
	
	//Roam_called_category
	if ( 0 != CharToUInt(&pRoamOut->unRoam_called_category, pRoamInOld->Roam_called_category, 1) )
	{
		nRe = 14;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_called_category);
	}
	
	//Roam_called_msrn
	if (0 != CharToStringReverse(pRoamOut->szRoam_called_msrn, pRoamInOld->Roam_called_msrn, 12) )
	{
		nRe = 15;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_called_msrn);
	}

	// Roam_in_circuit_group[2];    
	if (0 != CharToStringReverse(pRoamOut->szRoam_in_circuit_group, pRoamInOld->Roam_in_circuit_group, 2) )
	{
		nRe = 16;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_in_circuit_group);
	}
	
	// Roam_in_circuit[2];  
	if (0 != CharToStringReverse(pRoamOut->szRoam_in_circuit, pRoamInOld->Roam_in_circuit, 2) )
	{
		nRe = 17;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_in_circuit);
	}
	
	// Roam_out_circuit_group[2];    
	if (0 != CharToStringReverse(pRoamOut->szRoam_out_circuit_group, pRoamInOld->Roam_out_circuit_group, 2) )
	{
		nRe = 18;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_out_circuit_group);
	}

	// Roam_out_circuit[2];  
	if (0 != CharToStringReverse(pRoamOut->szRoam_out_circuit, pRoamInOld->Roam_out_circuit, 2) )
	{
		nRe = 19;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_out_circuit);
	}

	// Roam_basic_service_type[1];   
	if ( 0 != CharToUInt(&pRoamOut->unRoam_basic_service_type, pRoamInOld->Roam_basic_service_type, 1) )
	{
		nRe = 20;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_basic_service_type);
	}	
	
	// Roam_basic_service_code[1];  
	if ( 0 != CharToUInt(&pRoamOut->unRoam_basic_service_code, pRoamInOld->Roam_basic_service_code, 1) )
	{
		nRe = 21;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_basic_service_code);
	}	
	
	// Roam_set_up_start_time[7];    
	if( 0 != CharToDateTime(pRoamOut->szRoam_set_up_start_time, pRoamInOld->Roam_set_up_start_time) )
	{
		nRe = 22;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_set_up_start_time);
	}

	// Roam_in_channel_allocated_time[7];                         
	if( 0 != CharToDateTime(pRoamOut->szRoam_in_channel_allocated_time, pRoamInOld->Roam_in_channel_allocated_time) )
	{
		nRe = 23;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_in_channel_allocated_time);
	}

	// Roam_charging_start_time[7];                               
	if( 0 != CharToDateTime(pRoamOut->szRoam_charging_start_time, pRoamInOld->Roam_charging_start_time) )
	{
		nRe = 24;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_charging_start_time);
	}

	// Roam_charging_end_time[7];  
	if( 0 != CharToDateTime(pRoamOut->szRoam_charging_end_time, pRoamInOld->Roam_charging_end_time) )
	{
		nRe = 25;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_charging_end_time);
	}
	
	// unRoam_roam_mcz_duration; 
	pTemp1 = szTemp;//pRoamOut->unRoam_roam_mcz_duration;
	if( 0 != CharToBcdHalfWord(pTemp1, pRoamInOld->Roam_roam_mcz_duration, 3) )
	{
		nRe = 26;
		goto Exit;
	}
	sscanf(pTemp1,"%d",&pRoamOut->unRoam_roam_mcz_duration);
	pTemp1 = NULL;
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_roam_mcz_duration);
	}
	
	// Roam_cause_for_termination[4]
	if( 0 != CharToUInt(&pRoamOut->unRoam_cause_for_termination, pRoamInOld->Roam_cause_for_termination, 4) )
	{
		nRe = 27;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_cause_for_termination);
	}
	
	// Roam_data_volume[2];   
	if( 0 != CharToBcdHalfWord(pRoamOut->szRoam_data_volume, pRoamInOld->Roam_data_volume, 2) )
	{
		nRe = 28;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_data_volume);
	}

	// Roam_call_type[1];    
	if( 0 != CharToUInt(&pRoamOut->unRoam_call_type, pRoamInOld->Roam_call_type, 1) )
	{
		nRe = 29;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_call_type);
	}
	
	// Roam_roam_mcz_tariff_class[3];       
	if( 0 != CharToBcdHalfWord(pRoamOut->szRoam_roam_mcz_tariff_class, pRoamInOld->Roam_roam_mcz_tariff_class, 3) )
	{
		nRe = 30;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_roam_mcz_tariff_class);
	}

	// Roam_roam_mcz_pulses[2];     
	if( 0 != CharToBcdHalfWord(pRoamOut->szRoam_roam_mcz_pulses, pRoamInOld->Roam_roam_mcz_pulses, 2) )
	{
		nRe = 31;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_roam_mcz_pulses);
	}
	
	// Roam_called_msrn_ton[1];                                 
	if( 0 != CharToUInt(&pRoamOut->unRoam_called_msrn_ton, pRoamInOld->Roam_called_msrn_ton, 1) )
	{
		nRe = 32;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_called_msrn_ton);
	}

	// Roam_facility_usage[4]; 
	if( 0 != CharToBcdHalfWord(pRoamOut->szRoam_facility_usage, pRoamInOld->Roam_facility_usage, 4) )
	{
		nRe = 33;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_facility_usage);
	}

	// Roam_roam_mcz_chrg_type[1];                                
	if( 0 != CharToUInt(&pRoamOut->unRoam_roam_mcz_chrg_type, pRoamInOld->Roam_roam_mcz_chrg_type, 1) )
	{
		nRe = 34;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_roam_mcz_chrg_type);
	}

	// Roam_calling_number_ton[1];                                
	if( 0 != CharToUInt(&pRoamOut->unRoam_calling_number_ton, pRoamInOld->Roam_calling_number_ton, 1) )
	{
		nRe = 35;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_calling_number_ton);
	}
	
	// Roam_called_number_ton[1];                                 
	if( 0 != CharToUInt(&pRoamOut->unRoam_called_number_ton, pRoamInOld->Roam_called_number_ton, 1) )
	{
		nRe = 36;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_called_number_ton);
	}

	// Roam_routing_category[1];                                  
	if( 0 != CharToUInt(&pRoamOut->unRoam_routing_category, pRoamInOld->Roam_routing_category, 1) )
	{
		nRe = 37;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_routing_category);
	}
	
	// Roam_cf_information[1];                                  
	if( 0 != CharToUInt(&pRoamOut->unRoam_cf_information, pRoamInOld->Roam_cf_information, 1) )
	{
		nRe = 38;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_cf_information);
	}
	
	// Roam_intermediate_chrg_cause[2];   
	if( 0 != CharToUInt(&pRoamOut->unRoam_intermediate_chrg_cause, pRoamInOld->Roam_intermediate_chrg_cause, 2) )
	{
		nRe = 39;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_intermediate_chrg_cause);
	}
	
	// Roam_camel_call_reference[8];  
	if( 0 != CharToStringForward(pRoamOut->szRoam_camel_call_reference, pRoamInOld->Roam_camel_call_reference, 8) )
	{
		nRe = 40;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_camel_call_reference);
	}
	
	// Roam_camel_exchange_id_ton[1];                             
	if( 0 != CharToUInt(&pRoamOut->unRoam_camel_exchange_id_ton, pRoamInOld->Roam_camel_exchange_id_ton, 1) )
	{
		nRe = 41;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_camel_exchange_id_ton);
	}

	// Roam_camel_exchange_id[9]; 
	if( 0 != CharToStringReverse(pRoamOut->szRoam_camel_exchange_id, pRoamInOld->Roam_camel_exchange_id, 9) )
	{
		nRe = 42;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_camel_exchange_id);
	}
/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}

/************************************************************************/
/* 解析Roam函数 */
/* Parameters: pRoamOut 输出结构体 */
/* Parameters: pRoamIn 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/* Return Value: int  */
/************************************************************************/
int ParseRoam(SRoamOut* pRoamOut, SRoamIn* pRoamIn, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	char* pTemp1 = NULL;
	unsigned char* pTemp2 = NULL;
	char szTemp[255];
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	memset(pRoamOut,0,sizeof(SRoamOut));
	//Roam_record_length
 	pRoamOut->unRoam_record_length = pSegHandleOut->unRecord_length;
 	//Roam_record_type
 	pRoamOut->eRoam_record_type = pSegHandleOut->eRecord_type;

 	//Roam_record_number
	if (0 != CharToBcdHalfWord(pRoamOut->szRoam_record_number, pRoamIn->Roam_record_number, 4) )
	{
		nRe = 1;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_record_number);
	}

	//Roam_record_status
	switch(pRoamIn->Roam_record_status[0])
	{
	case 0x00:
		strcpy(pRoamOut->szRoam_record_status,"normal ok");
		break;
	case 0x01:
		strcpy(pRoamOut->szRoam_record_status,"synchronising error");
		break;
	case 0x02:
		strcpy(pRoamOut->szRoam_record_status,"different contents");
		break;
	default:
		nRe = 2;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pRoamOut->szRoam_record_status);
	}

	//Roam_check_sum
	if ( 0 != CharToUInt(&pRoamOut->unRoam_check_sum, pRoamIn->Roam_check_sum, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_check_sum);
	}

	//Roam_call_reference
	//comp+process+focus
	pTemp1 = pRoamOut->szRoam_call_reference;
	pTemp2 = pRoamIn->Roam_call_reference;
	strcpy(pTemp1, "comp:");
	pTemp1 += 5;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 4;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " process:");
	pTemp1 += 9;
	pTemp2 += 2;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 5;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " focus:");
	pTemp1 += 7;	
	pTemp2 += 2;
	if (0 != CharToStringForward(pTemp1, pTemp2, 1) )
	{
		nRe = 6;
		goto Exit;
	}
	pTemp1[2] = '\0';
	pTemp1 = NULL;
	pTemp2 = NULL;
	if (g_nDebug)
	{
		printf("%s\n",pRoamOut->szRoam_call_reference);
	}

	//Roam_exchange_id
	if (0 != CharToStringReverse(pRoamOut->szRoam_exchange_id, pRoamIn->Roam_exchange_id, 10) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_exchange_id);
	}
	
	//Roam_intermediate_record_number
	if (0 != CharToStringReverse(pRoamOut->szRoam_intermediate_record_number, pRoamIn->Roam_intermediate_record_number, 1) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_intermediate_record_number);
	}

	//Roam_intermediate_charging_ind
	if ( 0 != CharToUInt(&pRoamOut->unRoam_intermediate_charging_ind, pRoamIn->Roam_intermediate_charging_ind, 1) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_intermediate_charging_ind);
	}


	//Roam_number_of_ss_records
	if (0 != CharToStringReverse(pRoamOut->szRoam_number_of_ss_records, pRoamIn->Roam_number_of_ss_records, 1) )
	{
		nRe = 10;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_number_of_ss_records);
	}
	//Roam_calling_number
	if (0 != CharToCallNumber(pRoamOut->szRoam_calling_number, pRoamIn->Roam_calling_number, 10) )
	{
		nRe = 11;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_calling_number);
	}
	
	//Roam_called_imsi
	if (0 != CharToStringReverse(pRoamOut->szRoam_called_imsi, pRoamIn->Roam_called_imsi, 8) )
	{
		nRe = 12;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_called_imsi);
	}
	
	//szRoam_called_number
	if (0 != CharToCallNumber(pRoamOut->szRoam_called_number, pRoamIn->Roam_called_number, 12) )
	{
		nRe = 13;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_called_number);
	}
	
	
	//Roam_called_category
	if ( 0 != CharToUInt(&pRoamOut->unRoam_called_category, pRoamIn->Roam_called_category, 1) )
	{
		nRe = 14;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_called_category);
	}
	
	//Roam_called_msrn
	if (0 != CharToStringReverse(pRoamOut->szRoam_called_msrn, pRoamIn->Roam_called_msrn, 12) )
	{
		nRe = 15;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_called_msrn);
	}

	// Roam_in_circuit_group[2];    
	if (0 != CharToStringReverse(pRoamOut->szRoam_in_circuit_group, pRoamIn->Roam_in_circuit_group, 2) )
	{
		nRe = 16;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_in_circuit_group);
	}
	
	// Roam_in_circuit[2];  
	if (0 != CharToStringReverse(pRoamOut->szRoam_in_circuit, pRoamIn->Roam_in_circuit, 2) )
	{
		nRe = 17;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_in_circuit);
	}
	
	// Roam_out_circuit_group[2];    
	if (0 != CharToStringReverse(pRoamOut->szRoam_out_circuit_group, pRoamIn->Roam_out_circuit_group, 2) )
	{
		nRe = 18;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_out_circuit_group);
	}

	// Roam_out_circuit[2];  
	if (0 != CharToStringReverse(pRoamOut->szRoam_out_circuit, pRoamIn->Roam_out_circuit, 2) )
	{
		nRe = 19;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_out_circuit);
	}

	// Roam_basic_service_type[1];   
	if ( 0 != CharToUInt(&pRoamOut->unRoam_basic_service_type, pRoamIn->Roam_basic_service_type, 1) )
	{
		nRe = 20;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_basic_service_type);
	}	
	
	// Roam_basic_service_code[1];  
	if ( 0 != CharToUInt(&pRoamOut->unRoam_basic_service_code, pRoamIn->Roam_basic_service_code, 1) )
	{
		nRe = 21;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pRoamOut->unRoam_basic_service_code);
	}	
	
	// Roam_set_up_start_time[7];    
	if( 0 != CharToDateTime(pRoamOut->szRoam_set_up_start_time, pRoamIn->Roam_set_up_start_time) )
	{
		nRe = 22;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_set_up_start_time);
	}

	// Roam_in_channel_allocated_time[7];                         
	if( 0 != CharToDateTime(pRoamOut->szRoam_in_channel_allocated_time, pRoamIn->Roam_in_channel_allocated_time) )
	{
		nRe = 23;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_in_channel_allocated_time);
	}

	// Roam_charging_start_time[7];                               
	if( 0 != CharToDateTime(pRoamOut->szRoam_charging_start_time, pRoamIn->Roam_charging_start_time) )
	{
		nRe = 24;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_charging_start_time);
	}

	// Roam_charging_end_time[7];  
	if( 0 != CharToDateTime(pRoamOut->szRoam_charging_end_time, pRoamIn->Roam_charging_end_time) )
	{
		nRe = 25;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_charging_end_time);
	}
	
	// unRoam_roam_mcz_duration; 
	pTemp1 = szTemp;//pRoamOut->unRoam_roam_mcz_duration;
	if( 0 != CharToBcdHalfWord(pTemp1, pRoamIn->Roam_roam_mcz_duration, 3) )
	{
		nRe = 26;
		goto Exit;
	}
	sscanf(pTemp1,"%d",&pRoamOut->unRoam_roam_mcz_duration);
	pTemp1 = NULL;
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_roam_mcz_duration);
	}
	
	// Roam_cause_for_termination[4]
	if( 0 != CharToUInt(&pRoamOut->unRoam_cause_for_termination, pRoamIn->Roam_cause_for_termination, 4) )
	{
		nRe = 27;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_cause_for_termination);
	}
	
	// Roam_data_volume[2];   
	if( 0 != CharToBcdHalfWord(pRoamOut->szRoam_data_volume, pRoamIn->Roam_data_volume, 2) )
	{
		nRe = 28;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_data_volume);
	}

	// Roam_call_type[1];    
	if( 0 != CharToUInt(&pRoamOut->unRoam_call_type, pRoamIn->Roam_call_type, 1) )
	{
		nRe = 29;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_call_type);
	}
	
	// Roam_roam_mcz_tariff_class[3];       
	if( 0 != CharToBcdHalfWord(pRoamOut->szRoam_roam_mcz_tariff_class, pRoamIn->Roam_roam_mcz_tariff_class, 3) )
	{
		nRe = 30;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_roam_mcz_tariff_class);
	}

	// Roam_roam_mcz_pulses[2];     
	if( 0 != CharToBcdHalfWord(pRoamOut->szRoam_roam_mcz_pulses, pRoamIn->Roam_roam_mcz_pulses, 2) )
	{
		nRe = 31;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_roam_mcz_pulses);
	}
	
	// Roam_called_msrn_ton[1];                                 
	if( 0 != CharToUInt(&pRoamOut->unRoam_called_msrn_ton, pRoamIn->Roam_called_msrn_ton, 1) )
	{
		nRe = 32;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_called_msrn_ton);
	}

	// Roam_facility_usage[4]; 
	if( 0 != CharToBcdHalfWord(pRoamOut->szRoam_facility_usage, pRoamIn->Roam_facility_usage, 4) )
	{
		nRe = 33;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_facility_usage);
	}

	// Roam_roam_mcz_chrg_type[1];                                
	if( 0 != CharToUInt(&pRoamOut->unRoam_roam_mcz_chrg_type, pRoamIn->Roam_roam_mcz_chrg_type, 1) )
	{
		nRe = 34;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_roam_mcz_chrg_type);
	}

	// Roam_calling_number_ton[1];                                
	if( 0 != CharToUInt(&pRoamOut->unRoam_calling_number_ton, pRoamIn->Roam_calling_number_ton, 1) )
	{
		nRe = 35;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_calling_number_ton);
	}
	
	// Roam_called_number_ton[1];                                 
	if( 0 != CharToUInt(&pRoamOut->unRoam_called_number_ton, pRoamIn->Roam_called_number_ton, 1) )
	{
		nRe = 36;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_called_number_ton);
	}

	// Roam_routing_category[1];                                  
	if( 0 != CharToUInt(&pRoamOut->unRoam_routing_category, pRoamIn->Roam_routing_category, 1) )
	{
		nRe = 37;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_routing_category);
	}
	
	// Roam_cf_information[1];                                  
	if( 0 != CharToUInt(&pRoamOut->unRoam_cf_information, pRoamIn->Roam_cf_information, 1) )
	{
		nRe = 38;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_cf_information);
	}
	
	// Roam_intermediate_chrg_cause[2];   
	if( 0 != CharToUInt(&pRoamOut->unRoam_intermediate_chrg_cause, pRoamIn->Roam_intermediate_chrg_cause, 2) )
	{
		nRe = 39;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_intermediate_chrg_cause);
	}
	
	// Roam_camel_call_reference[8];  
	if( 0 != CharToStringForward(pRoamOut->szRoam_camel_call_reference, pRoamIn->Roam_camel_call_reference, 8) )
	{
		nRe = 40;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_camel_call_reference);
	}
	
	// Roam_camel_exchange_id_ton[1];                             
	if( 0 != CharToUInt(&pRoamOut->unRoam_camel_exchange_id_ton, pRoamIn->Roam_camel_exchange_id_ton, 1) )
	{
		nRe = 41;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_camel_exchange_id_ton);
	}

	// Roam_camel_exchange_id[9]; 
	if( 0 != CharToStringReverse(pRoamOut->szRoam_camel_exchange_id, pRoamIn->Roam_camel_exchange_id, 9) )
	{
		nRe = 42;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pRoamOut->szRoam_camel_exchange_id);
	}
	
// 	unsigned int unRoam_rate_adaption;                               //@(C(1),154,C(1),370,159)     
	if( 0 != CharToUInt(&pRoamOut->unRoam_rate_adaption, pRoamIn->Roam_rate_adaption, 1) )
	{
		nRe = 43;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_rate_adaption);
	}

// 	unsigned int unRoam_selected_codec;                              //@(C(1),155,C(1),648,183)     
	if( 0 != CharToUInt(&pRoamOut->unRoam_selected_codec, pRoamIn->Roam_selected_codec, 1) )
	{
		nRe = 44;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_selected_codec);
	}

// 	unsigned int unRoam_inside_user_plane_index;                     //@(BCD(2),156,W(1),357,151)  
	if( 0 != CharToUInt(&pRoamOut->unRoam_inside_user_plane_index, pRoamIn->Roam_inside_user_plane_index, 1) )
	{
		nRe = 45;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_inside_user_plane_index);
	}

// 	unsigned int unRoam_inside_control_plane_index;                  //@(BCD(2),158,W(1),359,152)   
	if( 0 != CharToUInt(&pRoamOut->unRoam_inside_control_plane_index, pRoamIn->Roam_inside_control_plane_index, 1) )
	{
		nRe = 46;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_inside_control_plane_index);
	}

// 	unsigned int unRoam_in_bnc_connection_type;                      //@(C(1),160,C(1),356,150)   
	if( 0 != CharToUInt(&pRoamOut->unRoam_in_bnc_connection_type, pRoamIn->Roam_in_bnc_connection_type, 1) )
	{
		nRe = 47;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_in_bnc_connection_type);
	}

// 	unsigned int unRoam_outside_user_plane_index;                    //@(BCD(2),161,W(1),362,155)   
	if( 0 != CharToUInt(&pRoamOut->unRoam_outside_user_plane_index, pRoamIn->Roam_outside_user_plane_index, 2) )
	{
		nRe = 48;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_outside_user_plane_index);
	}

// 	unsigned int unRoam_outside_control_plane_index;                 //@(BCD(2),163,W(1),364,156)  
	if( 0 != CharToUInt(&pRoamOut->unRoam_outside_control_plane_index, pRoamIn->Roam_outside_control_plane_index, 2) )
	{
		nRe = 49;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_outside_control_plane_index);
	}

// 	unsigned int unRoam_out_bnc_connection_type;                     //@(C(1),165,C(1),361,154)            
	if( 0 != CharToUInt(&pRoamOut->unRoam_out_bnc_connection_type, pRoamIn->Roam_out_bnc_connection_type, 1) )
	{
		nRe = 50;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pRoamOut->unRoam_out_bnc_connection_type);
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}

/************************************************************************/
/* 解析Trailer函数 */
/* Parameters: pRoamOut 输出结构体 */
/* Parameters: pRoamInOld 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/* Return Value: int  */
/************************************************************************/
int ParseTrailer(STrailerOut* pTrailerOut, STrailerIn* pTrailerIn, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	//Roam_record_length
 	pTrailerOut->unTra_record_length = pSegHandleOut->unRecord_length;
 	//Roam_record_type
 	pTrailerOut->eTra_record_type = pSegHandleOut->eRecord_type;
	//szTra_exchange_id[21];    
	if (0 != CharToStringReverse(pTrailerOut->szTra_exchange_id, pTrailerIn->Tra_exchange_id, 10) )
	{
		nRe = 1;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pTrailerOut->szTra_exchange_id);
	}

// 	char szTra_end_time[20];  
	if( 0 != CharToDateTime(pTrailerOut->szTra_end_time, pTrailerIn->Tra_end_time) )
	{
		nRe = 2;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pTrailerOut->szTra_end_time);
	}

// 	char szTra_last_record_number[9];                             
	if (0 != CharToBcdHalfWord(pTrailerOut->szTra_last_record_number, pTrailerIn->Tra_last_record_number, 4) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pTrailerOut->szTra_last_record_number);
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}


/************************************************************************/
/* 解析Moc函数 */
/* Parameters: pMocOut 输出结构体 */
/* Parameters: pMocInOld 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/* Return Value: int  */
/* 0 - 解析成功  */
/* 1 - 输入文件打开失败  */
/* 2 - 输出文件打开失败  */
/* 3 - success  */
/************************************************************************/
int ParseMocOld(SMocOut* pMocOut, SMocInOld* pMocInOld, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	char* pTemp1 = NULL;
	unsigned char* pTemp2 = NULL;
	int   nI  = 0;
	unsigned int   nTemp  = 0;
	char  szTemp[255];
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	memset(pMocOut,0,sizeof(SMocOut));
	//Moc_record_length
 	pMocOut->unMoc_record_length = pSegHandleOut->unRecord_length;
 	//Moc_record_type
 	pMocOut->eMoc_record_type = pSegHandleOut->eRecord_type;

 	//Moc_record_number
	if (0 != CharToBcdHalfWord(pMocOut->szMoc_record_number, pMocInOld->Moc_record_number, 4) )
	{
		nRe = 1;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_record_number);
	}

	//Moc_record_status
	switch(pMocInOld->Moc_record_status[0])
	{
	case 0x00:
		strcpy(pMocOut->szMoc_record_status,"normal ok");
		break;
	case 0x01:
		strcpy(pMocOut->szMoc_record_status,"synchronising error");
		break;
	case 0x02:
		strcpy(pMocOut->szMoc_record_status,"different contents");
		break;
	default:
		nRe = 2;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pMocOut->szMoc_record_status);
	}

	//Moc_check_sum
	if ( 0 != CharToUInt(&pMocOut->unMoc_check_sum, pMocInOld->Moc_check_sum, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_check_sum);
	}

	//Moc_call_reference
	//comp+process+focus
	pTemp1 = pMocOut->szMoc_call_reference;
	pTemp2 = pMocInOld->Moc_call_reference;
	strcpy(pTemp1, "comp:");
	pTemp1 += 5;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 4;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " process:");
	pTemp1 += 9;
	pTemp2 += 2;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 5;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " focus:");
	pTemp1 += 7;	
	pTemp2 += 2;
	if (0 != CharToStringForward(pTemp1, pTemp2, 1) )
	{
		nRe = 6;
		goto Exit;
	}
	pTemp1[2] = '\0';
	pTemp1 = NULL;
	pTemp2 = NULL;
	if (g_nDebug)
	{
		printf("%s\n",pMocOut->szMoc_call_reference);
	}

	//Moc_exchange_id
	if (0 != CharToStringReverse(pMocOut->szMoc_exchange_id, pMocInOld->Moc_exchange_id, 10) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_exchange_id);
	}
	
	//Moc_intermediate_record_number
	if (0 != CharToStringReverse(pMocOut->szMoc_intermediate_record_number, pMocInOld->Moc_intermediate_record_number, 1) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_intermediate_record_number);
	}

	//Moc_intermediate_charging_ind
	if ( 0 != CharToUInt(&pMocOut->unMoc_intermediate_charging_ind, pMocInOld->Moc_intermediate_charging_ind, 1) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_intermediate_charging_ind);
	}

	//Moc_number_of_ss_records
	if (0 != CharToStringReverse(pMocOut->szMoc_number_of_ss_records, pMocInOld->Moc_number_of_ss_records, 1) )
	{
		nRe = 10;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_number_of_ss_records);
	}

	//Moc_calling_imsi
	if (0 != CharToStringReverse(pMocOut->szMoc_calling_imsi, pMocInOld->Moc_calling_imsi, 8) )
	{
		nRe = 11;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_imsi);
	}
	
	//Moc_calling_imei
	if (0 != CharToStringReverse(pMocOut->szMoc_calling_imei, pMocInOld->Moc_calling_imei, 8) )
	{
		nRe = 12;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_imei);
	}

	//Moc_calling_number
	if (0 != CharToCallNumber(pMocOut->szMoc_calling_number, pMocInOld->Moc_calling_number, 10) )
	{
		nRe = 13;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_number);
	}
	
	//Moc_calling_category
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_category, pMocInOld->Moc_calling_category, 1) )
	{
		nRe = 14;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_category);
	}

	//Moc_calling_ms_classmark
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_ms_classmark, pMocInOld->Moc_calling_ms_classmark, 1) )
	{
		nRe = 15;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_ms_classmark);
	}

	//Moc_called_imsi
	if (0 != CharToStringReverse(pMocOut->szMoc_called_imsi, pMocInOld->Moc_called_imsi, 8) )
	{
		nRe = 16;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_imsi);
	}
	
	//Moc_called_imei
	if (0 != CharToStringReverse(pMocOut->szMoc_called_imei, pMocInOld->Moc_called_imei, 8) )
	{
		nRe = 17;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_imei);
	}

	//unMoc_dialled_digits_ton
	if ( 0 != CharToUInt(&pMocOut->unMoc_dialled_digits_ton, pMocInOld->Moc_dialled_digits_ton, 1) )
	{
		nRe = 18;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_dialled_digits_ton);
	}

	//szMoc_called_number
	if (0 != CharToCallNumber(pMocOut->szMoc_called_number, pMocInOld->Moc_called_number, 12) )
	{
		nRe = 19;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_number);
	}
	
	
	//Moc_called_category
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_category, pMocInOld->Moc_called_category, 1) )
	{
		nRe = 20;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_category);
	}
	
	//Moc_called_ms_classmark
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_ms_classmark, pMocInOld->Moc_called_ms_classmark, 1) )
	{
		nRe = 21;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_ms_classmark);
	}
	
	// Moc_dialled_digits[12];    
	if (0 != CharToStringReverse(pMocOut->szMoc_dialled_digits, pMocInOld->Moc_dialled_digits, 12) )
	{
		nRe = 22;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_dialled_digits);
	}
	
	// Moc_calling_subs_first_lac[2];   
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_first_lac, pMocInOld->Moc_calling_subs_first_lac, 2) )
	{
		nRe = 23;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_subs_first_lac);
	}	

	// Moc_calling_subs_first_ci[2]; 
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_first_ci, pMocInOld->Moc_calling_subs_first_ci, 2) )
	{
		nRe = 24;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_subs_first_ci);
	}	

	// Moc_calling_subs_last_ex_id[10];                          
	if (0 != CharToStringReverse(pMocOut->szMoc_calling_subs_last_ex_id, pMocInOld->Moc_calling_subs_last_ex_id, 10) )
	{
		nRe = 25;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_subs_last_ex_id);
	}

	// Moc_calling_subs_last_lac[2];                             
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_last_lac, pMocInOld->Moc_calling_subs_last_lac, 2) )
	{
		nRe = 26;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_subs_last_lac);
	}	

	// Moc_calling_subs_last_ci[2];                              
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_last_ci, pMocInOld->Moc_calling_subs_last_ci, 2) )
	{
		nRe = 27;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_subs_last_ci);
	}	

	// Moc_called_subs_first_lac[2];                             
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_subs_first_lac, pMocInOld->Moc_called_subs_first_lac, 2) )
	{
		nRe = 28;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_subs_first_lac);
	}	

	// Moc_called_subs_first_ci[2];                              
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_subs_first_ci, pMocInOld->Moc_called_subs_first_ci, 2) )
	{
		nRe = 29;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_subs_first_ci);
	}	

	// Moc_called_subs_last_ex_id[10];                           
	if (0 != CharToStringReverse(pMocOut->szMoc_called_subs_last_ex_id, pMocInOld->Moc_called_subs_last_ex_id, 10) )
	{
		nRe = 30;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_subs_last_ex_id);
	}

	// Moc_called_subs_last_lac[2];                              
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_subs_last_lac, pMocInOld->Moc_called_subs_last_lac, 2) )
	{
		nRe = 31;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_subs_last_lac);
	}	

	// Moc_called_subs_last_ci[2];   
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_subs_last_ci, pMocInOld->Moc_called_subs_last_ci, 2) )
	{
		nRe = 32;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_subs_last_ci);
	}	
	
	// Moc_out_circuit_group[2];    
	if (0 != CharToStringReverse(pMocOut->szMoc_out_circuit_group, pMocInOld->Moc_out_circuit_group, 2) )
	{
		nRe = 33;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_out_circuit_group);
	}

	// Moc_out_circuit[2];  
	if (0 != CharToStringReverse(pMocOut->szMoc_out_circuit, pMocInOld->Moc_out_circuit, 2) )
	{
		nRe = 34;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_out_circuit);
	}

	// Moc_basic_service_type[1];   
	if ( 0 != CharToUInt(&pMocOut->unMoc_basic_service_type, pMocInOld->Moc_basic_service_type, 1) )
	{
		nRe = 35;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_basic_service_type);
	}	
	
	// Moc_basic_service_code[1];  
	if ( 0 != CharToUInt(&pMocOut->unMoc_basic_service_code, pMocInOld->Moc_basic_service_code, 1) )
	{
		nRe = 36;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_basic_service_code);
	}	
	
	// Moc_non_transparency_indicator[1];   
	if ( 0 != CharToUInt(&pMocOut->unMoc_non_transparency_indicator, pMocInOld->Moc_non_transparency_indicator, 1) )
	{
		nRe = 37;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_non_transparency_indicator);
	}	
	
	// Moc_channel_rate_indicator[1];    
	if ( 0 != CharToUInt(&pMocOut->unMoc_channel_rate_indicator, pMocInOld->Moc_channel_rate_indicator, 1) )
	{
		nRe = 38;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_channel_rate_indicator);
	}	
	
	// Moc_set_up_start_time[7];    
	if( 0 != CharToDateTime(pMocOut->szMoc_set_up_start_time, pMocInOld->Moc_set_up_start_time) )
	{
		nRe = 39;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_set_up_start_time);
	}

	// Moc_in_channel_allocated_time[7];                         
	if( 0 != CharToDateTime(pMocOut->szMoc_in_channel_allocated_time, pMocInOld->Moc_in_channel_allocated_time) )
	{
		nRe = 40;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_in_channel_allocated_time);
	}

	// Moc_charging_start_time[7];                               
	if( 0 != CharToDateTime(pMocOut->szMoc_charging_start_time, pMocInOld->Moc_charging_start_time) )
	{
		nRe = 41;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_charging_start_time);
	}

	// Moc_charging_end_time[7];  
	if( 0 != CharToDateTime(pMocOut->szMoc_charging_end_time, pMocInOld->Moc_charging_end_time) )
	{
		nRe = 42;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_charging_end_time);
	}
	
	// unMoc_orig_mcz_duration; 
	pTemp1 = szTemp;
	if( 0 != CharToBcdHalfWord(pTemp1, pMocInOld->Moc_orig_mcz_duration, 3) )
	{
		nRe = 43;
		goto Exit;
	}
	sscanf(pTemp1,"%d",&pMocOut->unMoc_orig_mcz_duration);
	pTemp1 = NULL;
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_mcz_duration);
	}
	
	// Moc_cause_for_termination[4]
	if( 0 != CharToUInt(&pMocOut->unMoc_cause_for_termination, pMocInOld->Moc_cause_for_termination, 4) )
	{
		nRe = 44;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_cause_for_termination);
	}
	
	// Moc_data_volume[2];   
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_data_volume, pMocInOld->Moc_data_volume, 2) )
	{
		nRe = 45;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_data_volume);
	}

	// Moc_call_type[1];    
	if( 0 != CharToUInt(&pMocOut->unMoc_call_type, pMocInOld->Moc_call_type, 1) )
	{
		nRe = 46;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_call_type);
	}
	
	// Moc_orig_mcz_tariff_class[3];       
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_orig_mcz_tariff_class, pMocInOld->Moc_orig_mcz_tariff_class, 3) )
	{
		nRe = 47;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_orig_mcz_tariff_class);
	}

	// Moc_orig_mcz_pulses[2];     
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_orig_mcz_pulses, pMocInOld->Moc_orig_mcz_pulses, 2) )
	{
		nRe = 48;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_orig_mcz_pulses);
	}
	
	// Moc_dtmf_indicator[1];   
	if( 0 != CharToUInt(&pMocOut->unMoc_dtmf_indicator, pMocInOld->Moc_dtmf_indicator, 1) )
	{
		nRe = 49;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_dtmf_indicator);
	}

	// Moc_aoc_indicator[1];
	if( 0 != CharToUInt(&pMocOut->unMoc_aoc_indicator, pMocInOld->Moc_aoc_indicator, 1) )
	{
		nRe = 50;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_aoc_indicator);
	}

	// Moc_called_msrn_ton[1];    
	if( 0 != CharToUInt(&pMocOut->unMoc_called_msrn_ton, pMocInOld->Moc_called_msrn_ton, 1) )
	{
		nRe = 51;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_msrn_ton);
	}
	
	// Moc_called_msrn[12];    
	if( 0 != CharToStringReverse(pMocOut->szMoc_called_msrn, pMocInOld->Moc_called_msrn, 12) )
	{
		nRe = 52;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_msrn);
	}

	// Moc_called_number_ton[1];                                 
	if( 0 != CharToUInt(&pMocOut->unMoc_called_number_ton, pMocInOld->Moc_called_number_ton, 1) )
	{
		nRe = 53;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_number_ton);
	}

	// Moc_facility_usage[4]; 
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_facility_usage, pMocInOld->Moc_facility_usage, 4) )
	{
		nRe = 54;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_facility_usage);
	}

	// Moc_orig_mcz_chrg_type[1];                                
	if( 0 != CharToUInt(&pMocOut->unMoc_orig_mcz_chrg_type, pMocInOld->Moc_orig_mcz_chrg_type, 1) )
	{
		nRe = 55;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_mcz_chrg_type);
	}

	// Moc_calling_number_ton[1];                                
	if( 0 != CharToUInt(&pMocOut->unMoc_calling_number_ton, pMocInOld->Moc_calling_number_ton, 1) )
	{
		nRe = 56;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_calling_number_ton);
	}
	
	// Moc_routing_category[1];                                  
	if( 0 != CharToUInt(&pMocOut->unMoc_routing_category, pMocInOld->Moc_routing_category, 1) )
	{
		nRe = 57;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_routing_category);
	}

	// Moc_intermediate_chrg_cause[2];   
	if( 0 != CharToUInt(&pMocOut->unMoc_intermediate_chrg_cause, pMocInOld->Moc_intermediate_chrg_cause, 2) )
	{
		nRe = 58;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_intermediate_chrg_cause);
	}
	
	// Moc_camel_call_reference[8];  
	if( 0 != CharToStringForward(pMocOut->szMoc_camel_call_reference, pMocInOld->Moc_camel_call_reference, 8) )
	{
		nRe = 59;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_camel_call_reference);
	}
	
	// Moc_camel_exchange_id_ton[1];                             
	if( 0 != CharToUInt(&pMocOut->unMoc_camel_exchange_id_ton, pMocInOld->Moc_camel_exchange_id_ton, 1) )
	{
		nRe = 60;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_camel_exchange_id_ton);
	}

	// Moc_camel_exchange_id[9]; 
	if( 0 != CharToStringReverse(pMocOut->szMoc_camel_exchange_id, pMocInOld->Moc_camel_exchange_id, 9) )
	{
		nRe = 61;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_camel_exchange_id);
	}
	
	// Moc_calling_modify_parameters[14];    
	pTemp2 = pMocInOld->Moc_calling_modify_parameters;
	for (nI = 0; nI < 7 ; nI++)
	{
		if( 0 != CharToUInt(&nTemp, pTemp2, 2) )
		{
			nRe = 62;
			goto Exit;
		}
		sprintf(szTemp, "%d|", nTemp);
		strcat(pMocOut->szMoc_calling_modify_parameters, szTemp);
		pTemp2 += 2;
	}
	pTemp2 = NULL;
	nI = 0;
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_modify_parameters);
	}
	
	// Moc_orig_mcz_modify_percent[2]; 
	if( 0 != CharToUInt(&pMocOut->unMoc_orig_mcz_modify_percent, pMocInOld->Moc_orig_mcz_modify_percent, 2) )
	{
		nRe = 63;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_mcz_modify_percent);
	}
		
	// Moc_orig_mcz_modify_direction[1];                         
	if( 0 != CharToUInt(&pMocOut->unMoc_orig_mcz_modify_direction, pMocInOld->Moc_orig_mcz_modify_direction, 1) )
	{
		nRe = 64;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_mcz_modify_direction);
	}

	// Moc_orig_dialling_class[2];                               
	if( 0 != CharToUInt(&pMocOut->unMoc_orig_dialling_class, pMocInOld->Moc_orig_dialling_class, 2) )
	{
		nRe = 65;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_dialling_class);
	}

	// Moc_virtual_msc_id[10];   
	if( 0 != CharToStringReverse(pMocOut->szMoc_virtual_msc_id, pMocInOld->Moc_virtual_msc_id, 10) )
	{
		nRe = 66;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_virtual_msc_id);
	}
	
	// Moc_scf_address_ton[1];    
	if( 0 != CharToUInt(&pMocOut->unMoc_scf_address_ton, pMocInOld->Moc_scf_address_ton, 1) )
	{
		nRe = 67;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_scf_address_ton);
	}
	
	// Moc_scf_address[9];       
	if( 0 != CharToStringReverse(pMocOut->szMoc_scf_address, pMocInOld->Moc_scf_address, 9) )
	{
		nRe = 68;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_scf_address);
	}
	

	// Moc_destination_number_ton[1];                            
	if( 0 != CharToUInt(&pMocOut->unMoc_destination_number_ton, pMocInOld->Moc_destination_number_ton, 1) )
	{
		nRe = 69;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_destination_number_ton);
	}

	// Moc_destination_number_npi[1];                            
	if( 0 != CharToUInt(&pMocOut->unMoc_destination_number_npi, pMocInOld->Moc_destination_number_npi, 1) )
	{
		nRe = 70;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_destination_number_npi);
	}

	// Moc_destination_number[16];  
	if( 0 != CharToStringReverse(pMocOut->szMoc_destination_number, pMocInOld->Moc_destination_number, 16) )
	{
		nRe = 71;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_destination_number);
	}

	// Moc_camel_service_key[4];                                 
	if( 0 != CharToStringReverse(pMocOut->szMoc_camel_service_key, pMocInOld->Moc_camel_service_key, 4) )
	{
		nRe = 72;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_camel_service_key);
	}

	// Moc_calling_imeisv[8];                                    
	if( 0 != CharToStringReverse(pMocOut->szMoc_calling_imeisv, pMocInOld->Moc_calling_imeisv, 8) )
	{
		nRe = 73;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_imeisv);
	}
	
	// Moc_called_imeisv[8];                                     
	if( 0 != CharToStringReverse(pMocOut->szMoc_called_imeisv, pMocInOld->Moc_called_imeisv, 8) )
	{
		nRe = 74;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_imeisv);
	}

	// 	char szMoc_camel_service_data[113];//  (C(56))                FF FF FF	//No.94
	if( 0 != CharToStringForward(pMocOut->szMoc_camel_service_data, pMocInOld->Moc_camel_service_data, 56) )
	{
		nRe = 75;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_camel_service_data);
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}

/************************************************************************/
/* 解析Moc函数 */
/* Parameters: pMocOut 输出结构体 */
/* Parameters: pMocIn 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/************************************************************************/
int ParseMoc(SMocOut* pMocOut, SMocIn* pMocIn, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	char* pTemp1 = NULL;
	unsigned char* pTemp2 = NULL;
	int   nI  = 0;
	unsigned int   nTemp  = 0;
	char  szTemp[255];
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	memset(pMocOut,0,sizeof(SMocOut));
	//Moc_record_length
 	pMocOut->unMoc_record_length = pSegHandleOut->unRecord_length;
 	//Moc_record_type
 	pMocOut->eMoc_record_type = pSegHandleOut->eRecord_type;

 	//Moc_record_number
	if (0 != CharToBcdHalfWord(pMocOut->szMoc_record_number, pMocIn->Moc_record_number, 4) )
	{
		nRe = 1;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_record_number);
	}

	//Moc_record_status
	switch(pMocIn->Moc_record_status[0])
	{
	case 0x00:
		strcpy(pMocOut->szMoc_record_status,"normal ok");
		break;
	case 0x01:
		strcpy(pMocOut->szMoc_record_status,"synchronising error");
		break;
	case 0x02:
		strcpy(pMocOut->szMoc_record_status,"different contents");
		break;
	default:
		nRe = 2;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pMocOut->szMoc_record_status);
	}

	//Moc_check_sum
	if ( 0 != CharToUInt(&pMocOut->unMoc_check_sum, pMocIn->Moc_check_sum, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_check_sum);
	}

	//Moc_call_reference
	//comp+process+focus
	pTemp1 = pMocOut->szMoc_call_reference;
	pTemp2 = pMocIn->Moc_call_reference;
	strcpy(pTemp1, "comp:");
	pTemp1 += 5;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 4;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " process:");
	pTemp1 += 9;
	pTemp2 += 2;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 5;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " focus:");
	pTemp1 += 7;	
	pTemp2 += 2;
	if (0 != CharToStringForward(pTemp1, pTemp2, 1) )
	{
		nRe = 6;
		goto Exit;
	}
	pTemp1[2] = '\0';
	pTemp1 = NULL;
	pTemp2 = NULL;
	if (g_nDebug)
	{
		printf("%s\n",pMocOut->szMoc_call_reference);
	}

	//Moc_exchange_id
	if (0 != CharToStringReverse(pMocOut->szMoc_exchange_id, pMocIn->Moc_exchange_id, 10) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_exchange_id);
	}
	
	//Moc_intermediate_record_number
	if (0 != CharToStringReverse(pMocOut->szMoc_intermediate_record_number, pMocIn->Moc_intermediate_record_number, 1) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_intermediate_record_number);
	}

	//Moc_intermediate_charging_ind
	if ( 0 != CharToUInt(&pMocOut->unMoc_intermediate_charging_ind, pMocIn->Moc_intermediate_charging_ind, 1) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_intermediate_charging_ind);
	}

	//Moc_number_of_ss_records
	if (0 != CharToStringReverse(pMocOut->szMoc_number_of_ss_records, pMocIn->Moc_number_of_ss_records, 1) )
	{
		nRe = 10;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_number_of_ss_records);
	}

	//Moc_calling_imsi
	if (0 != CharToStringReverse(pMocOut->szMoc_calling_imsi, pMocIn->Moc_calling_imsi, 8) )
	{
		nRe = 11;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_imsi);
	}
	
	//Moc_calling_imei
	if (0 != CharToStringReverse(pMocOut->szMoc_calling_imei, pMocIn->Moc_calling_imei, 8) )
	{
		nRe = 12;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_imei);
	}

	//Moc_calling_number
	if (0 != CharToCallNumber(pMocOut->szMoc_calling_number, pMocIn->Moc_calling_number, 10) )
	{
		nRe = 13;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_number);
	}
	
	//Moc_calling_category
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_category, pMocIn->Moc_calling_category, 1) )
	{
		nRe = 14;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_category);
	}

	//Moc_calling_ms_classmark
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_ms_classmark, pMocIn->Moc_calling_ms_classmark, 1) )
	{
		nRe = 15;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_ms_classmark);
	}

	//Moc_called_imsi
	if (0 != CharToStringReverse(pMocOut->szMoc_called_imsi, pMocIn->Moc_called_imsi, 8) )
	{
		nRe = 16;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_imsi);
	}
	
	//Moc_called_imei
	if (0 != CharToStringReverse(pMocOut->szMoc_called_imei, pMocIn->Moc_called_imei, 8) )
	{
		nRe = 17;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_imei);
	}

	//unMoc_dialled_digits_ton
	if ( 0 != CharToUInt(&pMocOut->unMoc_dialled_digits_ton, pMocIn->Moc_dialled_digits_ton, 1) )
	{
		nRe = 18;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_dialled_digits_ton);
	}

	//szMoc_called_number
	if (0 != CharToCallNumber(pMocOut->szMoc_called_number, pMocIn->Moc_called_number, 12) )
	{
		nRe = 19;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_number);
	}
	
	//Moc_called_category
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_category, pMocIn->Moc_called_category, 1) )
	{
		nRe = 20;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_category);
	}
	
	//Moc_called_ms_classmark
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_ms_classmark, pMocIn->Moc_called_ms_classmark, 1) )
	{
		nRe = 21;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_ms_classmark);
	}
	
	// Moc_dialled_digits[12];    
	if (0 != CharToStringReverse(pMocOut->szMoc_dialled_digits, pMocIn->Moc_dialled_digits, 12) )
	{
		nRe = 22;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_dialled_digits);
	}
	
	// Moc_calling_subs_first_lac[2];   
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_first_lac, pMocIn->Moc_calling_subs_first_lac, 2) )
	{
		nRe = 23;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_subs_first_lac);
	}	

	// Moc_calling_subs_first_ci[2]; 
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_first_ci, pMocIn->Moc_calling_subs_first_ci, 2) )
	{
		nRe = 24;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_subs_first_ci);
	}	

	// Moc_calling_subs_last_ex_id[10];                          
	if (0 != CharToStringReverse(pMocOut->szMoc_calling_subs_last_ex_id, pMocIn->Moc_calling_subs_last_ex_id, 10) )
	{
		nRe = 25;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_subs_last_ex_id);
	}

	// Moc_calling_subs_last_lac[2];                             
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_last_lac, pMocIn->Moc_calling_subs_last_lac, 2) )
	{
		nRe = 26;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_subs_last_lac);
	}	

	// Moc_calling_subs_last_ci[2];                              
	if ( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_last_ci, pMocIn->Moc_calling_subs_last_ci, 2) )
	{
		nRe = 27;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_calling_subs_last_ci);
	}	

	// Moc_called_subs_first_lac[2];                             
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_subs_first_lac, pMocIn->Moc_called_subs_first_lac, 2) )
	{
		nRe = 28;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_subs_first_lac);
	}	

	// Moc_called_subs_first_ci[2];                              
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_subs_first_ci, pMocIn->Moc_called_subs_first_ci, 2) )
	{
		nRe = 29;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_subs_first_ci);
	}	

	// Moc_called_subs_last_ex_id[10];                           
	if (0 != CharToStringReverse(pMocOut->szMoc_called_subs_last_ex_id, pMocIn->Moc_called_subs_last_ex_id, 10) )
	{
		nRe = 30;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_subs_last_ex_id);
	}

	// Moc_called_subs_last_lac[2];                              
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_subs_last_lac, pMocIn->Moc_called_subs_last_lac, 2) )
	{
		nRe = 31;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_subs_last_lac);
	}	

	// Moc_called_subs_last_ci[2];   
	if ( 0 != CharToUInt(&pMocOut->unMoc_called_subs_last_ci, pMocIn->Moc_called_subs_last_ci, 2) )
	{
		nRe = 32;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_called_subs_last_ci);
	}	
	
	// Moc_out_circuit_group[2];    
	if (0 != CharToStringReverse(pMocOut->szMoc_out_circuit_group, pMocIn->Moc_out_circuit_group, 2) )
	{
		nRe = 33;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_out_circuit_group);
	}

	// Moc_out_circuit[2];  
	if (0 != CharToStringReverse(pMocOut->szMoc_out_circuit, pMocIn->Moc_out_circuit, 2) )
	{
		nRe = 34;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_out_circuit);
	}

	// Moc_basic_service_type[1];   
	if ( 0 != CharToUInt(&pMocOut->unMoc_basic_service_type, pMocIn->Moc_basic_service_type, 1) )
	{
		nRe = 35;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_basic_service_type);
	}	
	
	// Moc_basic_service_code[1];  
	if ( 0 != CharToUInt(&pMocOut->unMoc_basic_service_code, pMocIn->Moc_basic_service_code, 1) )
	{
		nRe = 36;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_basic_service_code);
	}	
	
	// Moc_non_transparency_indicator[1];   
	if ( 0 != CharToUInt(&pMocOut->unMoc_non_transparency_indicator, pMocIn->Moc_non_transparency_indicator, 1) )
	{
		nRe = 37;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_non_transparency_indicator);
	}	
	
	// Moc_channel_rate_indicator[1];    
	if ( 0 != CharToUInt(&pMocOut->unMoc_channel_rate_indicator, pMocIn->Moc_channel_rate_indicator, 1) )
	{
		nRe = 38;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMocOut->unMoc_channel_rate_indicator);
	}	
	
	// Moc_set_up_start_time[7];    
	if( 0 != CharToDateTime(pMocOut->szMoc_set_up_start_time, pMocIn->Moc_set_up_start_time) )
	{
		nRe = 39;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_set_up_start_time);
	}

	// Moc_in_channel_allocated_time[7];                         
	if( 0 != CharToDateTime(pMocOut->szMoc_in_channel_allocated_time, pMocIn->Moc_in_channel_allocated_time) )
	{
		nRe = 40;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_in_channel_allocated_time);
	}

	// Moc_charging_start_time[7];                               
	if( 0 != CharToDateTime(pMocOut->szMoc_charging_start_time, pMocIn->Moc_charging_start_time) )
	{
		nRe = 41;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_charging_start_time);
	}

	// Moc_charging_end_time[7];  
	if( 0 != CharToDateTime(pMocOut->szMoc_charging_end_time, pMocIn->Moc_charging_end_time) )
	{
		nRe = 42;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_charging_end_time);
	}
	
	// unMoc_orig_mcz_duration; 
	pTemp1 = szTemp;
	if( 0 != CharToBcdHalfWord(pTemp1, pMocIn->Moc_orig_mcz_duration, 3) )
	{
		nRe = 43;
		goto Exit;
	}
	sscanf(pTemp1,"%d",&pMocOut->unMoc_orig_mcz_duration);
	pTemp1 = NULL;
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_mcz_duration);
	}
	
	// Moc_cause_for_termination[4]
	if( 0 != CharToUInt(&pMocOut->unMoc_cause_for_termination, pMocIn->Moc_cause_for_termination, 4) )
	{
		nRe = 44;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_cause_for_termination);
	}
	
	// Moc_data_volume[2];   
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_data_volume, pMocIn->Moc_data_volume, 2) )
	{
		nRe = 45;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_data_volume);
	}

	// Moc_call_type[1];    
	if( 0 != CharToUInt(&pMocOut->unMoc_call_type, pMocIn->Moc_call_type, 1) )
	{
		nRe = 46;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_call_type);
	}
	
	// Moc_orig_mcz_tariff_class[3];       
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_orig_mcz_tariff_class, pMocIn->Moc_orig_mcz_tariff_class, 3) )
	{
		nRe = 47;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_orig_mcz_tariff_class);
	}

	// Moc_orig_mcz_pulses[2];     
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_orig_mcz_pulses, pMocIn->Moc_orig_mcz_pulses, 2) )
	{
		nRe = 48;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_orig_mcz_pulses);
	}
	
	// Moc_dtmf_indicator[1];   
	if( 0 != CharToUInt(&pMocOut->unMoc_dtmf_indicator, pMocIn->Moc_dtmf_indicator, 1) )
	{
		nRe = 49;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_dtmf_indicator);
	}

	// Moc_aoc_indicator[1];
	if( 0 != CharToUInt(&pMocOut->unMoc_aoc_indicator, pMocIn->Moc_aoc_indicator, 1) )
	{
		nRe = 50;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_aoc_indicator);
	}

	// Moc_called_msrn_ton[1];    
	if( 0 != CharToUInt(&pMocOut->unMoc_called_msrn_ton, pMocIn->Moc_called_msrn_ton, 1) )
	{
		nRe = 51;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_msrn_ton);
	}
	
	// Moc_called_msrn[12];    
	if( 0 != CharToStringReverse(pMocOut->szMoc_called_msrn, pMocIn->Moc_called_msrn, 12) )
	{
		nRe = 52;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_msrn);
	}

	// Moc_called_number_ton[1];                                 
	if( 0 != CharToUInt(&pMocOut->unMoc_called_number_ton, pMocIn->Moc_called_number_ton, 1) )
	{
		nRe = 53;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_number_ton);
	}

	// Moc_facility_usage[4]; 
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_facility_usage, pMocIn->Moc_facility_usage, 4) )
	{
		nRe = 54;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_facility_usage);
	}

	// Moc_orig_mcz_chrg_type[1];                                
	if( 0 != CharToUInt(&pMocOut->unMoc_orig_mcz_chrg_type, pMocIn->Moc_orig_mcz_chrg_type, 1) )
	{
		nRe = 55;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_mcz_chrg_type);
	}

	// Moc_calling_number_ton[1];                                
	if( 0 != CharToUInt(&pMocOut->unMoc_calling_number_ton, pMocIn->Moc_calling_number_ton, 1) )
	{
		nRe = 56;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_calling_number_ton);
	}
	
	// Moc_routing_category[1];                                  
	if( 0 != CharToUInt(&pMocOut->unMoc_routing_category, pMocIn->Moc_routing_category, 1) )
	{
		nRe = 57;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_routing_category);
	}

	// Moc_intermediate_chrg_cause[2];   
	if( 0 != CharToUInt(&pMocOut->unMoc_intermediate_chrg_cause, pMocIn->Moc_intermediate_chrg_cause, 2) )
	{
		nRe = 58;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_intermediate_chrg_cause);
	}
	
	// Moc_camel_call_reference[8];  
	if( 0 != CharToStringForward(pMocOut->szMoc_camel_call_reference, pMocIn->Moc_camel_call_reference, 8) )
	{
		nRe = 59;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_camel_call_reference);
	}
	
	// Moc_camel_exchange_id_ton[1];                             
	if( 0 != CharToUInt(&pMocOut->unMoc_camel_exchange_id_ton, pMocIn->Moc_camel_exchange_id_ton, 1) )
	{
		nRe = 60;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_camel_exchange_id_ton);
	}

	// Moc_camel_exchange_id[9]; 
	if( 0 != CharToStringReverse(pMocOut->szMoc_camel_exchange_id, pMocIn->Moc_camel_exchange_id, 9) )
	{
		nRe = 61;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_camel_exchange_id);
	}
	
	// Moc_calling_modify_parameters[14];    
	pTemp2 = pMocIn->Moc_calling_modify_parameters;
	for (nI = 0; nI < 7 ; nI++)
	{
		if( 0 != CharToUInt(&nTemp, pTemp2, 2) )
		{
			nRe = 62;
			goto Exit;
		}
		sprintf(szTemp, "%d|", nTemp);
		strcat(pMocOut->szMoc_calling_modify_parameters, szTemp);
		pTemp2 += 2;
	}
	pTemp2 = NULL;
	nI = 0;
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_modify_parameters);
	}
	
	// Moc_orig_mcz_modify_percent[2]; 
	if( 0 != CharToUInt(&pMocOut->unMoc_orig_mcz_modify_percent, pMocIn->Moc_orig_mcz_modify_percent, 2) )
	{
		nRe = 63;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_mcz_modify_percent);
	}
		
	// Moc_orig_mcz_modify_direction[1];                         
	if( 0 != CharToUInt(&pMocOut->unMoc_orig_mcz_modify_direction, pMocIn->Moc_orig_mcz_modify_direction, 1) )
	{
		nRe = 64;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_mcz_modify_direction);
	}

	// Moc_orig_dialling_class[2];                               
	if( 0 != CharToUInt(&pMocOut->unMoc_orig_dialling_class, pMocIn->Moc_orig_dialling_class, 2) )
	{
		nRe = 65;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_orig_dialling_class);
	}

	// Moc_virtual_msc_id[10];   
	if( 0 != CharToStringReverse(pMocOut->szMoc_virtual_msc_id, pMocIn->Moc_virtual_msc_id, 10) )
	{
		nRe = 66;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_virtual_msc_id);
	}
	
	// Moc_scf_address_ton[1];    
	if( 0 != CharToUInt(&pMocOut->unMoc_scf_address_ton, pMocIn->Moc_scf_address_ton, 1) )
	{
		nRe = 67;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_scf_address_ton);
	}
	
	// Moc_scf_address[9];       
	if( 0 != CharToStringReverse(pMocOut->szMoc_scf_address, pMocIn->Moc_scf_address, 9) )
	{
		nRe = 68;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_scf_address);
	}
	

	// Moc_destination_number_ton[1];                            
	if( 0 != CharToUInt(&pMocOut->unMoc_destination_number_ton, pMocIn->Moc_destination_number_ton, 1) )
	{
		nRe = 69;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_destination_number_ton);
	}

	// Moc_destination_number_npi[1];                            
	if( 0 != CharToUInt(&pMocOut->unMoc_destination_number_npi, pMocIn->Moc_destination_number_npi, 1) )
	{
		nRe = 70;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_destination_number_npi);
	}

	// Moc_destination_number[16];  
	if( 0 != CharToStringReverse(pMocOut->szMoc_destination_number, pMocIn->Moc_destination_number, 16) )
	{
		nRe = 71;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_destination_number);
	}

	// Moc_camel_service_key[4];                                 
	if( 0 != CharToStringReverse(pMocOut->szMoc_camel_service_key, pMocIn->Moc_camel_service_key, 4) )
	{
		nRe = 72;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_camel_service_key);
	}

	// Moc_calling_imeisv[8];                                    
	if( 0 != CharToStringReverse(pMocOut->szMoc_calling_imeisv, pMocIn->Moc_calling_imeisv, 8) )
	{
		nRe = 73;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_calling_imeisv);
	}
	
	// Moc_called_imeisv[8];                                     
	if( 0 != CharToStringReverse(pMocOut->szMoc_called_imeisv, pMocIn->Moc_called_imeisv, 8) )
	{
		nRe = 74;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_called_imeisv);
	}
// 	unsigned int unMoc_emergency_call_category;//  (C(1))             00		//No.75
	if( 0 != CharToUInt(&pMocOut->unMoc_emergency_call_category, pMocIn->Moc_emergency_call_category, 1) )
	{
		nRe = 75;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_emergency_call_category);
	}

// 	unsigned int unMoc_used_air_interface_user_rate;//  (C(1))        00		//No.76
	if( 0 != CharToUInt(&pMocOut->unMoc_used_air_interface_user_rate, pMocIn->Moc_used_air_interface_user_rate, 1) )
	{
		nRe = 76;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_used_air_interface_user_rate);
	}

// 	unsigned int unMoc_req_air_interface_user_rate;//  (C(1))         FF		//No.77
	if( 0 != CharToUInt(&pMocOut->unMoc_req_air_interface_user_rate, pMocIn->Moc_req_air_interface_user_rate, 1) )
	{
		nRe = 77;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_req_air_interface_user_rate);
	}

// 	unsigned int unMoc_used_fixed_nw_user_rate;//  (C(1))             FF		//No.78
	if( 0 != CharToUInt(&pMocOut->unMoc_used_fixed_nw_user_rate, pMocIn->Moc_used_fixed_nw_user_rate, 1) )
	{
		nRe = 78;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_used_fixed_nw_user_rate);
	}

// 	unsigned int unMoc_req_fixed_nw_user_rate;//  (C(1))              FF		//No.79
	if( 0 != CharToUInt(&pMocOut->unMoc_req_fixed_nw_user_rate, pMocIn->Moc_req_fixed_nw_user_rate, 1) )
	{
		nRe = 79;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_req_fixed_nw_user_rate);
	}

// 	unsigned int unMoc_rate_adaption;//  (C(1))                       FF		//No.80
	if( 0 != CharToUInt(&pMocOut->unMoc_rate_adaption, pMocIn->Moc_rate_adaption, 1) )
	{
		nRe = 80;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_rate_adaption);
	}

// 	unsigned int unMoc_stream_identifier;//  (C(1))                   00		//No.81
	if( 0 != CharToUInt(&pMocOut->unMoc_stream_identifier, pMocIn->Moc_stream_identifier, 1) )
	{
		nRe = 81;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_stream_identifier);
	}

// 	unsigned int unMoc_ms_classmark3;//  (C(1))                       02		//No.82
	if( 0 != CharToUInt(&pMocOut->unMoc_ms_classmark3, pMocIn->Moc_ms_classmark3, 1) )
	{
		nRe = 82;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_ms_classmark3);
	}

// 	unsigned int unMoc_calling_cell_band;//  (C(1))                   00		//No.83
	if( 0 != CharToUInt(&pMocOut->unMoc_calling_cell_band, pMocIn->Moc_calling_cell_band, 1) )
	{
		nRe = 83;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_calling_cell_band);
	}

// 	unsigned int unMoc_calling_subs_last_ex_id_ton;//  (C(1))         05		//No.84
	if( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_last_ex_id_ton, pMocIn->Moc_calling_subs_last_ex_id_ton, 1) )
	{
		nRe = 84;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_calling_subs_last_ex_id_ton);
	}

// 	unsigned int unMoc_called_subs_last_ex_id_ton;//  (C(1))          FF		//No.85
	if( 0 != CharToUInt(&pMocOut->unMoc_called_subs_last_ex_id_ton, pMocIn->Moc_called_subs_last_ex_id_ton, 1) )
	{
		nRe = 85;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_subs_last_ex_id_ton);
	}

// 	unsigned int unMoc_calling_subs_first_mcc;//  (C(2))              64 F0	//No.86
	if( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_first_mcc, pMocIn->Moc_calling_subs_first_mcc, 2) )
	{
		nRe = 86;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_calling_subs_first_mcc);
	}

// 	unsigned int unMoc_calling_subs_first_mnc;//  (C(2))              00 FF	//No.87
	if( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_first_mnc, pMocIn->Moc_calling_subs_first_mnc, 2) )
	{
		nRe = 87;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_calling_subs_first_mnc);
	}

// 	unsigned int unMoc_calling_subs_last_mcc;//  (C(2))               64 F0	//No.88
	if( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_last_mcc, pMocIn->Moc_calling_subs_last_mcc, 2) )
	{
		nRe = 88;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_calling_subs_last_mcc);
	}

// 	unsigned int unMoc_calling_subs_last_mnc;//  (C(2))               00 FF	//No.89
	if( 0 != CharToUInt(&pMocOut->unMoc_calling_subs_last_mnc, pMocIn->Moc_calling_subs_last_mnc, 2) )
	{
		nRe = 89;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_calling_subs_last_mnc);
	}

// 	unsigned int unMoc_called_subs_first_mcc;//  (C(2))               FF FF	//No.90
	if( 0 != CharToUInt(&pMocOut->unMoc_called_subs_first_mcc, pMocIn->Moc_called_subs_first_mcc, 2) )
	{
		nRe = 90;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_subs_first_mcc);
	}

// 	unsigned int unMoc_called_subs_first_mnc;//  (C(2))               FF FF	//No.91
	if( 0 != CharToUInt(&pMocOut->unMoc_called_subs_first_mnc, pMocIn->Moc_called_subs_first_mnc, 2) )
	{
		nRe = 91;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_subs_first_mnc);
	}

// 	unsigned int unMoc_called_subs_last_mcc;//  (C(2))                FF FF	//No.92
	if( 0 != CharToUInt(&pMocOut->unMoc_called_subs_last_mcc, pMocIn->Moc_called_subs_last_mcc, 2) )
	{
		nRe = 92;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_subs_last_mcc);
	}

// 	unsigned int unMoc_called_subs_last_mnc;//  (C(2))                FF FF	//No.93
	if( 0 != CharToUInt(&pMocOut->unMoc_called_subs_last_mnc, pMocIn->Moc_called_subs_last_mnc, 2) )
	{
		nRe = 93;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_called_subs_last_mnc);
	}

// 	char szMoc_camel_service_data[113];//  (C(56))                FF FF FF	//No.94
	if( 0 != CharToStringForward(pMocOut->szMoc_camel_service_data, pMocIn->Moc_camel_service_data, 56) )
	{
		nRe = 94;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_camel_service_data);
	}

// 	unsigned int unMoc_selected_codec;//  (C(1))                      FF		//No.95
	if( 0 != CharToUInt(&pMocOut->unMoc_selected_codec, pMocIn->Moc_selected_codec, 1) )
	{
		nRe = 95;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_selected_codec);
	}

// 	char szMoc_outside_user_plane_index[5];//  (BCD(2))          01 00	//No.96
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_outside_user_plane_index, pMocIn->Moc_outside_user_plane_index, 2) )
	{
		nRe = 96;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_outside_user_plane_index);
	}

// 	char szMoc_outside_control_plane_index[5];//  (BCD(2))       01 00	//No.97
	if( 0 != CharToBcdHalfWord(pMocOut->szMoc_outside_control_plane_index, pMocIn->Moc_outside_control_plane_index, 2) )
	{
		nRe = 97;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMocOut->szMoc_outside_control_plane_index);
	}


// 	unsigned int unMoc_out_bnc_connection_type;//  (C(1))             10		//No.98
	if( 0 != CharToUInt(&pMocOut->unMoc_out_bnc_connection_type, pMocIn->Moc_out_bnc_connection_type, 1) )
	{
		nRe = 98;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_out_bnc_connection_type);
	}

// 	unsigned int unMoc_radio_network_type;//  (C(1))                  01		//No.99
	if( 0 != CharToUInt(&pMocOut->unMoc_radio_network_type, pMocIn->Moc_radio_network_type, 1) )
	{
		nRe = 99;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMocOut->unMoc_radio_network_type);
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}

/************************************************************************/
/* 解析Mtc函数 */
/* Parameters: pMtcOut 输出结构体 */
/* Parameters: pMtcInOld 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/************************************************************************/
int ParseMtcOld(SMtcOut* pMtcOut, SMtcInOld* pMtcInOld, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	char* pTemp1 = NULL;
	unsigned char* pTemp2 = NULL;
	int   nI  = 0;
	unsigned int   nTemp  = 0;
	char  szTemp[255];
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	memset(pMtcOut, 0 ,sizeof(SMtcOut));
	//unMtc_record_length;                                           
 	pMtcOut->unMtc_record_length = pSegHandleOut->unRecord_length;
 	//eMtc_record_type;                                             
 	pMtcOut->eMtc_record_type = pSegHandleOut->eRecord_type;

 	//szMtc_record_number
	if (0 != CharToBcdHalfWord(pMtcOut->szMtc_record_number, pMtcInOld->Mtc_record_number, 4) )
	{
		nRe = 1;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_record_number);
	}

	//szMtc_record_status
	switch(pMtcInOld->Mtc_record_status[0])
	{
	case 0x00:
		strcpy(pMtcOut->szMtc_record_status,"normal ok");
		break;
	case 0x01:
		strcpy(pMtcOut->szMtc_record_status,"synchronising error");
		break;
	case 0x02:
		strcpy(pMtcOut->szMtc_record_status,"different contents");
		break;
	default:
		nRe = 2;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pMtcOut->szMtc_record_status);
	}

	//unMtc_check_sum;                                               
	if ( 0 != CharToUInt(&pMtcOut->unMtc_check_sum, pMtcInOld->Mtc_check_sum, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_check_sum);
	}

	//szMtc_call_reference
	//comp+process+focus
	pTemp1 = pMtcOut->szMtc_call_reference;
	pTemp2 = pMtcInOld->Mtc_call_reference;
	strcpy(pTemp1, "comp:");
	pTemp1 += 5;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 4;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " process:");
	pTemp1 += 9;
	pTemp2 += 2;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 5;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " focus:");
	pTemp1 += 7;	
	pTemp2 += 2;
	if (0 != CharToStringForward(pTemp1, pTemp2, 1) )
	{
		nRe = 6;
		goto Exit;
	}
	pTemp1[2] = '\0';
	pTemp1 = NULL;
	pTemp2 = NULL;
	if (g_nDebug)
	{
		printf("%s\n",pMtcOut->szMtc_call_reference);
	}

	//szMtc_exchange_id
	if (0 != CharToStringReverse(pMtcOut->szMtc_exchange_id, pMtcInOld->Mtc_exchange_id, 10) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_exchange_id);
	}
	
	//szMtc_intermediate_record_number
	if (0 != CharToStringReverse(pMtcOut->szMtc_intermediate_record_number, pMtcInOld->Mtc_intermediate_record_number, 1) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_intermediate_record_number);
	}

	//unMtc_intermediate_charging_ind;                              
	if ( 0 != CharToUInt(&pMtcOut->unMtc_intermediate_charging_ind, pMtcInOld->Mtc_intermediate_charging_ind, 1) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_intermediate_charging_ind);
	}

	//szMtc_number_of_ss_records
	if (0 != CharToStringReverse(pMtcOut->szMtc_number_of_ss_records, pMtcInOld->Mtc_number_of_ss_records, 1) )
	{
		nRe = 10;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_number_of_ss_records);
	}
	//szMtc_calling_number
	if (0 != CharToCallNumber(pMtcOut->szMtc_calling_number, pMtcInOld->Mtc_calling_number, 12) )
	{
		nRe = 11;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_calling_number);
	}
	
	//szMtc_called_imsi
	if (0 != CharToStringReverse(pMtcOut->szMtc_called_imsi, pMtcInOld->Mtc_called_imsi, 8) )
	{
		nRe = 12;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_imsi);
	}
	
	//szMtc_called_imei
	if (0 != CharToStringReverse(pMtcOut->szMtc_called_imei, pMtcInOld->Mtc_called_imei, 8) )
	{
		nRe = 13;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_imei);
	}

	//szMtc_called_number
	if (0 != CharToCallNumber(pMtcOut->szMtc_called_number, pMtcInOld->Mtc_called_number, 12) )
	{
		nRe = 14;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_number);
	}
	
	
	//unMtc_called_category;                                         
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_category, pMtcInOld->Mtc_called_category, 1) )
	{
		nRe = 15;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_category);
	}
	
	//unMtc_called_ms_classmark;                                     
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_ms_classmark, pMtcInOld->Mtc_called_ms_classmark, 1) )
	{
		nRe = 16;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_ms_classmark);
	}
	
	// pMtc_in_circuit_group[2];    
	if (0 != CharToStringReverse(pMtcOut->szMtc_in_circuit_group, pMtcInOld->Mtc_in_circuit_group, 2) )
	{
		nRe = 17;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_in_circuit_group);
	}
	
	// szMtc_out_circuit[2];  
	if (0 != CharToStringReverse(pMtcOut->szMtc_in_circuit, pMtcInOld->Mtc_in_circuit, 2) )
	{
		nRe = 18;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_in_circuit);
	}

	// unMtc_called_subs_first_lac;                                   [2];                             
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_first_lac, pMtcInOld->Mtc_called_subs_first_lac, 2) )
	{
		nRe = 19;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_subs_first_lac);
	}	

	// unMtc_called_subs_first_ci;                                                                 
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_first_ci, pMtcInOld->Mtc_called_subs_first_ci, 2) )
	{
		nRe = 20;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_subs_first_ci);
	}	

	// szMtc_called_subs_last_ex_id[10];                           
	if (0 != CharToStringReverse(pMtcOut->szMtc_called_subs_last_ex_id, pMtcInOld->Mtc_called_subs_last_ex_id, 10) )
	{
		nRe = 21;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_subs_last_ex_id);
	}

	// unMtc_called_subs_last_lac;                                                                 
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_last_lac, pMtcInOld->Mtc_called_subs_last_lac, 2) )
	{
		nRe = 22;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_subs_last_lac);
	}	

	// unMtc_called_subs_last_ci;                                     [2];   
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_last_ci, pMtcInOld->Mtc_called_subs_last_ci, 2) )
	{
		nRe = 23;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_subs_last_ci);
	}	
	

	// unMtc_basic_service_type;                                      [1];   
	if ( 0 != CharToUInt(&pMtcOut->unMtc_basic_service_type, pMtcInOld->Mtc_basic_service_type, 1) )
	{
		nRe = 24;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_basic_service_type);
	}	
	
	// unMtc_basic_service_code;                                      [1];  
	if ( 0 != CharToUInt(&pMtcOut->unMtc_basic_service_code, pMtcInOld->Mtc_basic_service_code, 1) )
	{
		nRe = 25;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_basic_service_code);
	}	
	
	// unMtc_non_transparency_indicator;                              [1];   
	if ( 0 != CharToUInt(&pMtcOut->unMtc_non_transparency_indicator, pMtcInOld->Mtc_non_transparency_indicator, 1) )
	{
		nRe = 26;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_non_transparency_indicator);
	}	
	
	// unMtc_channel_rate_indicator;                                  [1];    
	if ( 0 != CharToUInt(&pMtcOut->unMtc_channel_rate_indicator, pMtcInOld->Mtc_channel_rate_indicator, 1) )
	{
		nRe = 27;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_channel_rate_indicator);
	}	
	
	// szMtc_set_up_start_time[7];    
	if( 0 != CharToDateTime(pMtcOut->szMtc_set_up_start_time, pMtcInOld->Mtc_set_up_start_time) )
	{
		nRe = 28;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_set_up_start_time);
	}

	// Mtc_in_channel_allocated_time[7];                         
	if( 0 != CharToDateTime(pMtcOut->szMtc_in_channel_allocated_time, pMtcInOld->Mtc_in_channel_allocated_time) )
	{
		nRe = 29;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_in_channel_allocated_time);
	}

	// Mtc_charging_start_time[7];                               
	if( 0 != CharToDateTime(pMtcOut->szMtc_charging_start_time, pMtcInOld->Mtc_charging_start_time) )
	{
		nRe = 30;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_charging_start_time);
	}

	// Mtc_charging_end_time[7];  
	if( 0 != CharToDateTime(pMtcOut->szMtc_charging_end_time, pMtcInOld->Mtc_charging_end_time) )
	{
		nRe = 31;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_charging_end_time);
	}
	
	// unMtc_term_mcz_duration; 
	pTemp1 = szTemp;//pMtcOut->unMtc_term_mcz_duration;
	if( 0 != CharToBcdHalfWord(pTemp1, pMtcInOld->Mtc_term_mcz_duration, 3) )
	{
		nRe = 32;
		goto Exit;
	}
	sscanf(pTemp1,"%d",&pMtcOut->unMtc_term_mcz_duration);
	pTemp1 = NULL;
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_term_mcz_duration);
	}
	
	// unMtc_cause_for_termination;                                   [4]
	if( 0 != CharToUInt(&pMtcOut->unMtc_cause_for_termination, pMtcInOld->Mtc_cause_for_termination, 4) )
	{
		nRe = 33;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_cause_for_termination);
	}
	
	// Mtc_data_volume[2];   
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_data_volume, pMtcInOld->Mtc_data_volume, 2) )
	{
		nRe = 34;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_data_volume);
	}

	// Mtc_call_type[1];    
	if( 0 != CharToUInt(&pMtcOut->unMtc_call_type, pMtcInOld->Mtc_call_type, 1) )
	{
		nRe = 35;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_call_type);
	}
	
	// Mtc_term_mcz_tariff_class[3];       
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_term_mcz_tariff_class, pMtcInOld->Mtc_term_mcz_tariff_class, 3) )
	{
		nRe = 36;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_term_mcz_tariff_class);
	}

	// Mtc_orig_mcz_pulses[2];     
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_term_mcz_pulses, pMtcInOld->Mtc_term_mcz_pulses, 2) )
	{
		nRe = 37;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_term_mcz_pulses);
	}
	
	// Mtc_dtmf_indicator[1];   
	if( 0 != CharToUInt(&pMtcOut->unMtc_dtmf_indicator, pMtcInOld->Mtc_dtmf_indicator, 1) )
	{
		nRe = 38;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_dtmf_indicator);
	}

	// Mtc_aoc_indicator[1];
	if( 0 != CharToUInt(&pMtcOut->unMtc_aoc_indicator, pMtcInOld->Mtc_aoc_indicator, 1) )
	{
		nRe = 39;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_aoc_indicator);
	}

	// Mtc_facility_usage[4]; 
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_facility_usage, pMtcInOld->Mtc_facility_usage, 4) )
	{
		nRe = 40;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_facility_usage);
	}

	// unMtc_term_mcz_chrg_type;                                      [1];                                
	if( 0 != CharToUInt(&pMtcOut->unMtc_term_mcz_chrg_type, pMtcInOld->Mtc_term_mcz_chrg_type, 1) )
	{
		nRe = 41;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_term_mcz_chrg_type);
	}

	// Mtc_calling_number_ton[1];                                
	if( 0 != CharToUInt(&pMtcOut->unMtc_calling_number_ton, pMtcInOld->Mtc_calling_number_ton, 1) )
	{
		nRe = 42;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_calling_number_ton);
	}

	// Mtc_called_number_ton[1];                                
	if( 0 != CharToUInt(&pMtcOut->unMtc_called_number_ton, pMtcInOld->Mtc_called_number_ton, 1) )
	{
		nRe = 43;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_called_number_ton);
	}
	
	// Mtc_routing_category[1];                                  
	if( 0 != CharToUInt(&pMtcOut->unMtc_routing_category, pMtcInOld->Mtc_routing_category, 1) )
	{
		nRe = 44;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_routing_category);
	}

	// Mtc_intermediate_chrg_cause[2];   
	if( 0 != CharToUInt(&pMtcOut->unMtc_intermediate_chrg_cause, pMtcInOld->Mtc_intermediate_chrg_cause, 2) )
	{
		nRe = 45;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_intermediate_chrg_cause);
	}
	
	// szMtc_camel_call_reference[8];  
	if( 0 != CharToStringForward(pMtcOut->szMtc_camel_call_reference, pMtcInOld->Mtc_camel_call_reference, 8) )
	{
		nRe = 46;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_camel_call_reference);
	}
	
	// Mtc_camel_exchange_id_ton[1];                             
	if( 0 != CharToUInt(&pMtcOut->unMtc_camel_exchange_id_ton, pMtcInOld->Mtc_camel_exchange_id_ton, 1) )
	{
		nRe = 47;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_camel_exchange_id_ton);
	}

	// Mtc_camel_exchange_id[9]; 
	if( 0 != CharToStringReverse(pMtcOut->szMtc_camel_exchange_id, pMtcInOld->Mtc_camel_exchange_id, 9) )
	{
		nRe = 48;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_camel_exchange_id);
	}
	
	// Mtc_calling_modify_parameters[14];    
	pTemp2 = pMtcInOld->Mtc_called_modify_parameters;
	for (nI = 0; nI < 7 ; nI++)
	{
		if( 0 != CharToUInt(&nTemp, pTemp2, 2) )
		{
			nRe = 49;
			goto Exit;
		}
		sprintf(szTemp, "%d|", nTemp);
		strcat(pMtcOut->szMtc_calling_modify_parameters, szTemp);
		pTemp2 += 2;
	}
	pTemp2 = NULL;
	nI = 0;
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_calling_modify_parameters);
	}
	
	// unMtc_term_mcz_modify_percent[2]; 
	if( 0 != CharToUInt(&pMtcOut->unMtc_term_mcz_modify_percent, pMtcInOld->Mtc_term_mcz_modify_percent, 2) )
	{
		nRe = 50;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_term_mcz_modify_percent);
	}
		
	// Mtc_term_mcz_modify_direction[1];                         
	if( 0 != CharToUInt(&pMtcOut->unMtc_term_mcz_modify_direction, pMtcInOld->Mtc_term_mcz_modify_direction, 1) )
	{
		nRe = 51;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_term_mcz_modify_direction);
	}

	//szMtc_redirecting_number
	if( 0 != CharToCallNumber(pMtcOut->szMtc_redirecting_number, pMtcInOld->Mtc_redirecting_number, 12) )
	{
		nRe = 52;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_redirecting_number);
	}
	
	// unMtc_redirecting_number_ton;                                     [1];                         
	if( 0 != CharToUInt(&pMtcOut->unMtc_redirecting_number_ton, pMtcInOld->Mtc_redirecting_number_ton, 1) )
	{
		nRe = 53;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_redirecting_number_ton);
	}

	// Mtc_virtual_msc_id[10];   
	if( 0 != CharToStringReverse(pMtcOut->szMtc_virtual_msc_id, pMtcInOld->Mtc_virtual_msc_id, 10) )
	{
		nRe = 54;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_virtual_msc_id);
	}
	
	// Mtc_called_imeisv[8];                                     
	if( 0 != CharToStringReverse(pMtcOut->szMtc_called_imeisv, pMtcInOld->Mtc_called_imeisv, 8) )
	{
		nRe = 55;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_imeisv);
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}

/************************************************************************/
/* 解析Mtc函数 */
/* Parameters: pMtcOut 输出结构体 */
/* Parameters: pMtcIn 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/************************************************************************/
int ParseMtc(SMtcOut* pMtcOut, SMtcIn* pMtcIn, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	char* pTemp1 = NULL;
	unsigned char* pTemp2 = NULL;
	int   nI  = 0;
	unsigned int   nTemp  = 0;
	char  szTemp[255];
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	memset(pMtcOut, 0 ,sizeof(SMtcOut));
	//unMtc_record_length;                                           
 	pMtcOut->unMtc_record_length = pSegHandleOut->unRecord_length;
 	//eMtc_record_type;                                             
 	pMtcOut->eMtc_record_type = pSegHandleOut->eRecord_type;

 	//szMtc_record_number
	if (0 != CharToBcdHalfWord(pMtcOut->szMtc_record_number, pMtcIn->Mtc_record_number, 4) )
	{
		nRe = 1;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_record_number);
	}

	//szMtc_record_status
	switch(pMtcIn->Mtc_record_status[0])
	{
	case 0x00:
		strcpy(pMtcOut->szMtc_record_status,"normal ok");
		break;
	case 0x01:
		strcpy(pMtcOut->szMtc_record_status,"synchronising error");
		break;
	case 0x02:
		strcpy(pMtcOut->szMtc_record_status,"different contents");
		break;
	default:
		nRe = 2;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pMtcOut->szMtc_record_status);
	}

	//unMtc_check_sum;                                               
	if ( 0 != CharToUInt(&pMtcOut->unMtc_check_sum, pMtcIn->Mtc_check_sum, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_check_sum);
	}

	//szMtc_call_reference
	//comp+process+focus
	pTemp1 = pMtcOut->szMtc_call_reference;
	pTemp2 = pMtcIn->Mtc_call_reference;
	strcpy(pTemp1, "comp:");
	pTemp1 += 5;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 4;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " process:");
	pTemp1 += 9;
	pTemp2 += 2;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 5;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " focus:");
	pTemp1 += 7;	
	pTemp2 += 2;
	if (0 != CharToStringForward(pTemp1, pTemp2, 1) )
	{
		nRe = 6;
		goto Exit;
	}
	pTemp1[2] = '\0';
	pTemp1 = NULL;
	pTemp2 = NULL;
	if (g_nDebug)
	{
		printf("%s\n",pMtcOut->szMtc_call_reference);
	}

	//szMtc_exchange_id
	if (0 != CharToStringReverse(pMtcOut->szMtc_exchange_id, pMtcIn->Mtc_exchange_id, 10) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_exchange_id);
	}
	
	//szMtc_intermediate_record_number
	if (0 != CharToStringReverse(pMtcOut->szMtc_intermediate_record_number, pMtcIn->Mtc_intermediate_record_number, 1) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_intermediate_record_number);
	}

	//unMtc_intermediate_charging_ind;                              
	if ( 0 != CharToUInt(&pMtcOut->unMtc_intermediate_charging_ind, pMtcIn->Mtc_intermediate_charging_ind, 1) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_intermediate_charging_ind);
	}

	//szMtc_number_of_ss_records
	if (0 != CharToStringReverse(pMtcOut->szMtc_number_of_ss_records, pMtcIn->Mtc_number_of_ss_records, 1) )
	{
		nRe = 10;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_number_of_ss_records);
	}
	//szMtc_calling_number
	if (0 != CharToCallNumber(pMtcOut->szMtc_calling_number, pMtcIn->Mtc_calling_number, 12) )
	{
		nRe = 11;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_calling_number);
	}
	
	//szMtc_called_imsi
	if (0 != CharToStringReverse(pMtcOut->szMtc_called_imsi, pMtcIn->Mtc_called_imsi, 8) )
	{
		nRe = 12;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_imsi);
	}
	
	//szMtc_called_imei
	if (0 != CharToStringReverse(pMtcOut->szMtc_called_imei, pMtcIn->Mtc_called_imei, 8) )
	{
		nRe = 13;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_imei);
	}

	//szMtc_called_number
	if (0 != CharToCallNumber(pMtcOut->szMtc_called_number, pMtcIn->Mtc_called_number, 12) )
	{
		nRe = 14;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_number);
	}
	
	
	//unMtc_called_category;                                         
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_category, pMtcIn->Mtc_called_category, 1) )
	{
		nRe = 15;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_category);
	}
	
	//unMtc_called_ms_classmark;                                     
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_ms_classmark, pMtcIn->Mtc_called_ms_classmark, 1) )
	{
		nRe = 16;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_ms_classmark);
	}
	
	// pMtc_in_circuit_group[2];    
	if (0 != CharToStringReverse(pMtcOut->szMtc_in_circuit_group, pMtcIn->Mtc_in_circuit_group, 2) )
	{
		nRe = 17;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_in_circuit_group);
	}
	
	// szMtc_out_circuit[2];  
	if (0 != CharToStringReverse(pMtcOut->szMtc_in_circuit, pMtcIn->Mtc_in_circuit, 2) )
	{
		nRe = 18;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_in_circuit);
	}


	// unMtc_called_subs_first_lac;                                   [2];                             
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_first_lac, pMtcIn->Mtc_called_subs_first_lac, 2) )
	{
		nRe = 19;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_subs_first_lac);
	}	

	// unMtc_called_subs_first_ci;                                                                 
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_first_ci, pMtcIn->Mtc_called_subs_first_ci, 2) )
	{
		nRe = 20;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_subs_first_ci);
	}	

	// szMtc_called_subs_last_ex_id[10];                           
	if (0 != CharToStringReverse(pMtcOut->szMtc_called_subs_last_ex_id, pMtcIn->Mtc_called_subs_last_ex_id, 10) )
	{
		nRe = 21;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_subs_last_ex_id);
	}

	// unMtc_called_subs_last_lac;                                                                 
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_last_lac, pMtcIn->Mtc_called_subs_last_lac, 2) )
	{
		nRe = 22;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_subs_last_lac);
	}	

	// unMtc_called_subs_last_ci;                                     [2];   
	if ( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_last_ci, pMtcIn->Mtc_called_subs_last_ci, 2) )
	{
		nRe = 23;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_called_subs_last_ci);
	}	
	

	// unMtc_basic_service_type;                                      [1];   
	if ( 0 != CharToUInt(&pMtcOut->unMtc_basic_service_type, pMtcIn->Mtc_basic_service_type, 1) )
	{
		nRe = 24;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_basic_service_type);
	}	
	
	// unMtc_basic_service_code;                                      [1];  
	if ( 0 != CharToUInt(&pMtcOut->unMtc_basic_service_code, pMtcIn->Mtc_basic_service_code, 1) )
	{
		nRe = 25;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_basic_service_code);
	}	
	
	// unMtc_non_transparency_indicator;                              [1];   
	if ( 0 != CharToUInt(&pMtcOut->unMtc_non_transparency_indicator, pMtcIn->Mtc_non_transparency_indicator, 1) )
	{
		nRe = 26;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_non_transparency_indicator);
	}	
	
	// unMtc_channel_rate_indicator;                                  [1];    
	if ( 0 != CharToUInt(&pMtcOut->unMtc_channel_rate_indicator, pMtcIn->Mtc_channel_rate_indicator, 1) )
	{
		nRe = 27;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pMtcOut->unMtc_channel_rate_indicator);
	}	
	
	// szMtc_set_up_start_time[7];    
	if( 0 != CharToDateTime(pMtcOut->szMtc_set_up_start_time, pMtcIn->Mtc_set_up_start_time) )
	{
		nRe = 28;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_set_up_start_time);
	}

	// Mtc_in_channel_allocated_time[7];                         
	if( 0 != CharToDateTime(pMtcOut->szMtc_in_channel_allocated_time, pMtcIn->Mtc_in_channel_allocated_time) )
	{
		nRe = 29;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_in_channel_allocated_time);
	}

	// Mtc_charging_start_time[7];                               
	if( 0 != CharToDateTime(pMtcOut->szMtc_charging_start_time, pMtcIn->Mtc_charging_start_time) )
	{
		nRe = 30;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_charging_start_time);
	}

	// Mtc_charging_end_time[7];  
	if( 0 != CharToDateTime(pMtcOut->szMtc_charging_end_time, pMtcIn->Mtc_charging_end_time) )
	{
		nRe = 31;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_charging_end_time);
	}
	
	// unMtc_term_mcz_duration; 
	pTemp1 = szTemp;//pMtcOut->unMtc_term_mcz_duration;
	if( 0 != CharToBcdHalfWord(pTemp1, pMtcIn->Mtc_term_mcz_duration, 3) )
	{
		nRe = 32;
		goto Exit;
	}
	sscanf(pTemp1,"%d",&pMtcOut->unMtc_term_mcz_duration);
	pTemp1 = NULL;
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_term_mcz_duration);
	}
	
	// unMtc_cause_for_termination;                                   [4]
	if( 0 != CharToUInt(&pMtcOut->unMtc_cause_for_termination, pMtcIn->Mtc_cause_for_termination, 4) )
	{
		nRe = 33;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_cause_for_termination);
	}
	
	// Mtc_data_volume[2];   
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_data_volume, pMtcIn->Mtc_data_volume, 2) )
	{
		nRe = 34;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_data_volume);
	}

	// Mtc_call_type[1];    
	if( 0 != CharToUInt(&pMtcOut->unMtc_call_type, pMtcIn->Mtc_call_type, 1) )
	{
		nRe = 35;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_call_type);
	}
	
	// Mtc_term_mcz_tariff_class[3];       
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_term_mcz_tariff_class, pMtcIn->Mtc_term_mcz_tariff_class, 3) )
	{
		nRe = 36;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_term_mcz_tariff_class);
	}

	// Mtc_orig_mcz_pulses[2];     
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_term_mcz_pulses, pMtcIn->Mtc_term_mcz_pulses, 2) )
	{
		nRe = 37;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_term_mcz_pulses);
	}
	
	// Mtc_dtmf_indicator[1];   
	if( 0 != CharToUInt(&pMtcOut->unMtc_dtmf_indicator, pMtcIn->Mtc_dtmf_indicator, 1) )
	{
		nRe = 38;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_dtmf_indicator);
	}

	// Mtc_aoc_indicator[1];
	if( 0 != CharToUInt(&pMtcOut->unMtc_aoc_indicator, pMtcIn->Mtc_aoc_indicator, 1) )
	{
		nRe = 39;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_aoc_indicator);
	}

	// Mtc_facility_usage[4]; 
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_facility_usage, pMtcIn->Mtc_facility_usage, 4) )
	{
		nRe = 40;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_facility_usage);
	}

	// unMtc_term_mcz_chrg_type;                                      [1];                                
	if( 0 != CharToUInt(&pMtcOut->unMtc_term_mcz_chrg_type, pMtcIn->Mtc_term_mcz_chrg_type, 1) )
	{
		nRe = 41;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_term_mcz_chrg_type);
	}

	// Mtc_calling_number_ton[1];                                
	if( 0 != CharToUInt(&pMtcOut->unMtc_calling_number_ton, pMtcIn->Mtc_calling_number_ton, 1) )
	{
		nRe = 42;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_calling_number_ton);
	}

	// Mtc_called_number_ton[1];                                
	if( 0 != CharToUInt(&pMtcOut->unMtc_called_number_ton, pMtcIn->Mtc_called_number_ton, 1) )
	{
		nRe = 43;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_called_number_ton);
	}
	
	// Mtc_routing_category[1];                                  
	if( 0 != CharToUInt(&pMtcOut->unMtc_routing_category, pMtcIn->Mtc_routing_category, 1) )
	{
		nRe = 44;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_routing_category);
	}

	// Mtc_intermediate_chrg_cause[2];   
	if( 0 != CharToUInt(&pMtcOut->unMtc_intermediate_chrg_cause, pMtcIn->Mtc_intermediate_chrg_cause, 2) )
	{
		nRe = 45;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_intermediate_chrg_cause);
	}
	
	// szMtc_camel_call_reference[8];  
	if( 0 != CharToStringForward(pMtcOut->szMtc_camel_call_reference, pMtcIn->Mtc_camel_call_reference, 8) )
	{
		nRe = 46;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_camel_call_reference);
	}
	
	// Mtc_camel_exchange_id_ton[1];                             
	if( 0 != CharToUInt(&pMtcOut->unMtc_camel_exchange_id_ton, pMtcIn->Mtc_camel_exchange_id_ton, 1) )
	{
		nRe = 47;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_camel_exchange_id_ton);
	}

	// Mtc_camel_exchange_id[9]; 
	if( 0 != CharToStringReverse(pMtcOut->szMtc_camel_exchange_id, pMtcIn->Mtc_camel_exchange_id, 9) )
	{
		nRe = 48;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_camel_exchange_id);
	}
	
	// Mtc_calling_modify_parameters[14];    
	pTemp2 = pMtcIn->Mtc_called_modify_parameters;
	for (nI = 0; nI < 7 ; nI++)
	{
		if( 0 != CharToUInt(&nTemp, pTemp2, 2) )
		{
			nRe = 49;
			goto Exit;
		}
		sprintf(szTemp, "%d|", nTemp);
		strcat(pMtcOut->szMtc_calling_modify_parameters, szTemp);
		pTemp2 += 2;
	}
	pTemp2 = NULL;
	nI = 0;
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_calling_modify_parameters);
	}
	
	// unMtc_term_mcz_modify_percent[2]; 
	if( 0 != CharToUInt(&pMtcOut->unMtc_term_mcz_modify_percent, pMtcIn->Mtc_term_mcz_modify_percent, 2) )
	{
		nRe = 50;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_term_mcz_modify_percent);
	}
		
	// Mtc_term_mcz_modify_direction[1];                         
	if( 0 != CharToUInt(&pMtcOut->unMtc_term_mcz_modify_direction, pMtcIn->Mtc_term_mcz_modify_direction, 1) )
	{
		nRe = 51;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_term_mcz_modify_direction);
	}

	//szMtc_redirecting_number
	if( 0 != CharToCallNumber(pMtcOut->szMtc_redirecting_number, pMtcIn->Mtc_redirecting_number, 12) )
	{
		nRe = 52;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_redirecting_number);
	}
	
	// unMtc_redirecting_number_ton;                                     [1];                         
	if( 0 != CharToUInt(&pMtcOut->unMtc_redirecting_number_ton, pMtcIn->Mtc_redirecting_number_ton, 1) )
	{
		nRe = 53;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_redirecting_number_ton);
	}

	// Mtc_virtual_msc_id[10];   
	if( 0 != CharToStringReverse(pMtcOut->szMtc_virtual_msc_id, pMtcIn->Mtc_virtual_msc_id, 10) )
	{
		nRe = 54;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_virtual_msc_id);
	}
	
	// Mtc_called_imeisv[8];                                     
	if( 0 != CharToStringReverse(pMtcOut->szMtc_called_imeisv, pMtcIn->Mtc_called_imeisv, 8) )
	{
		nRe = 55;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_called_imeisv);
	}


// 	unsigned int unMtc_used_air_interface_user_rate;//  (C(1))        00
	if( 0 != CharToUInt(&pMtcOut->unMtc_used_air_interface_user_rate, pMtcIn->Mtc_used_air_interface_user_rate, 1) )
	{
		nRe = 56;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_used_air_interface_user_rate);
	}

// 	unsigned int unMtc_req_air_interface_user_rate;//  (C(1))         FF
	if( 0 != CharToUInt(&pMtcOut->unMtc_req_air_interface_user_rate, pMtcIn->Mtc_req_air_interface_user_rate, 1) )
	{
		nRe = 57;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_req_air_interface_user_rate);
	}
// 	unsigned int unMtc_used_fixed_nw_user_rate;//  (C(1))             FF
	if( 0 != CharToUInt(&pMtcOut->unMtc_used_fixed_nw_user_rate, pMtcIn->Mtc_used_fixed_nw_user_rate, 1) )
	{
		nRe = 58;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_used_fixed_nw_user_rate);
	}
// 	unsigned int unMtc_req_fixed_nw_user_rate;//  (C(1))              FF
	if( 0 != CharToUInt(&pMtcOut->unMtc_req_fixed_nw_user_rate, pMtcIn->Mtc_req_fixed_nw_user_rate, 1) )
	{
		nRe = 59;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_req_fixed_nw_user_rate);
	}
// 	unsigned int unMtc_rate_adaption;// (C(1))                       FF
	if( 0 != CharToUInt(&pMtcOut->unMtc_rate_adaption, pMtcIn->Mtc_rate_adaption, 1) )
	{
		nRe = 60;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_rate_adaption);
	}
// 	unsigned int unMtc_stream_identifier;//  (C(1))                   00
	if( 0 != CharToUInt(&pMtcOut->unMtc_stream_identifier, pMtcIn->Mtc_stream_identifier, 1) )
	{
		nRe = 61;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_stream_identifier);
	}
// 	unsigned int unMtc_ms_classmark3;//  (C(1))                       02
	if( 0 != CharToUInt(&pMtcOut->unMtc_ms_classmark3, pMtcIn->Mtc_ms_classmark3, 1) )
	{
		nRe = 62;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_ms_classmark3);
	}

// 	unsigned int unMtc_called_cell_band;//  (C(1))                    00
	if( 0 != CharToUInt(&pMtcOut->unMtc_called_cell_band, pMtcIn->Mtc_called_cell_band, 1) )
	{
		nRe = 63;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_called_cell_band);
	}

// 	unsigned int unMtc_called_subs_last_ex_id_ton;//  (C(1))          05
	if( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_last_ex_id_ton, pMtcIn->Mtc_called_subs_last_ex_id_ton, 1) )
	{
		nRe = 64;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_called_subs_last_ex_id_ton);
	}

// 	unsigned int unMtc_called_subs_first_mcc;//  (C(2))               64 F0
	if( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_first_mcc, pMtcIn->Mtc_called_subs_first_mcc, 2) )
	{
		nRe = 65;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_called_subs_first_mcc);
	}
// 	unsigned int unMtc_called_subs_first_mnc;//  (C(2))               00 FF
	if( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_first_mnc, pMtcIn->Mtc_called_subs_first_mnc, 2) )
	{
		nRe = 66;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_called_subs_first_mnc);
	}
// 	unsigned int unMtc_called_subs_last_mcc;//  (C(2))                64 F0
	if( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_last_mcc, pMtcIn->Mtc_called_subs_last_mcc, 2) )
	{
		nRe = 67;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_called_subs_last_mcc);
	}
// 	unsigned int unMtc_called_subs_last_mnc;//  (C(2))                00 FF
	if( 0 != CharToUInt(&pMtcOut->unMtc_called_subs_last_mnc, pMtcIn->Mtc_called_subs_last_mnc, 2) )
	{
		nRe = 68;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_called_subs_last_mnc);
	}

// 	unsigned int unMtc_selected_codec;//  (C(1))                      FF
	if( 0 != CharToUInt(&pMtcOut->unMtc_selected_codec, pMtcIn->Mtc_selected_codec, 1) )
	{
		nRe = 69;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_selected_codec);
	}
// 	char Mtc_inside_user_plane_index[5];//  (BCD(2))           01 00
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_inside_user_plane_index, pMtcIn->Mtc_inside_user_plane_index, 2) )
	{
		nRe = 70;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_inside_user_plane_index);
	}
// 	char Mtc_inside_control_plane_index[5];//  (BCD(2))        01 00
	if( 0 != CharToBcdHalfWord(pMtcOut->szMtc_inside_control_plane_index, pMtcIn->Mtc_inside_control_plane_index, 2) )
	{
		nRe = 71;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pMtcOut->szMtc_inside_control_plane_index);
	}
// 	unsigned int unMtc_in_bnc_connection_type;//  (C(1))              10
	if( 0 != CharToUInt(&pMtcOut->unMtc_in_bnc_connection_type, pMtcIn->Mtc_in_bnc_connection_type, 1) )
	{
		nRe = 72;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_in_bnc_connection_type);
	}

// 	unsigned int unMtc_radio_network_type;//  (C(1))                  01
	if( 0 != CharToUInt(&pMtcOut->unMtc_radio_network_type, pMtcIn->Mtc_radio_network_type, 1) )
	{
		nRe = 73;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pMtcOut->unMtc_radio_network_type);
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}

/************************************************************************/
/* 解析Forw函数 */
/* Parameters: pForwOut 输出结构体 */
/* Parameters: pForwInOld 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/************************************************************************/
int ParseForwOld(SForwOut* pForwOut, SForwInOld* pForwInOld, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	char* pTemp1 = NULL;
	unsigned char* pTemp2 = NULL;
	char  szTemp[255];
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	memset(pForwOut,0,sizeof(SForwOut));
	//unForw_record_length;                                           
 	pForwOut->unForw_record_length = pSegHandleOut->unRecord_length;
 	//eForw_record_type;                                             
 	pForwOut->eForw_record_type = pSegHandleOut->eRecord_type;

 	//szForw_record_number
	if (0 != CharToBcdHalfWord(pForwOut->szForw_record_number, pForwInOld->Forw_record_number, 4) )
	{
		nRe = 1;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_record_number);
	}

	//szForw_record_status
	switch(pForwInOld->Forw_record_status[0])
	{
	case 0x00:
		strcpy(pForwOut->szForw_record_status,"normal ok");
		break;
	case 0x01:
		strcpy(pForwOut->szForw_record_status,"synchronising error");
		break;
	case 0x02:
		strcpy(pForwOut->szForw_record_status,"different contents");
		break;
	default:
		nRe = 2;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pForwOut->szForw_record_status);
	}

	//unForw_check_sum;                                               
	if ( 0 != CharToUInt(&pForwOut->unForw_check_sum, pForwInOld->Forw_check_sum, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_check_sum);
	}

	//szForw_call_reference
	//comp+process+focus
	pTemp1 = pForwOut->szForw_call_reference;
	pTemp2 = pForwInOld->Forw_call_reference;
	strcpy(pTemp1, "comp:");
	pTemp1 += 5;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 4;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " process:");
	pTemp1 += 9;
	pTemp2 += 2;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 5;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " focus:");
	pTemp1 += 7;	
	pTemp2 += 2;
	if (0 != CharToStringForward(pTemp1, pTemp2, 1) )
	{
		nRe = 6;
		goto Exit;
	}
	pTemp1[2] = '\0';
	pTemp1 = NULL;
	pTemp2 = NULL;
	if (g_nDebug)
	{
		printf("%s\n",pForwOut->szForw_call_reference);
	}

	//szForw_exchange_id
	if (0 != CharToStringReverse(pForwOut->szForw_exchange_id, pForwInOld->Forw_exchange_id, 10) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_exchange_id);
	}
	
	//szForw_intermediate_record_number
	if (0 != CharToStringReverse(pForwOut->szForw_intermediate_record_number, pForwInOld->Forw_intermediate_record_number, 1) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_intermediate_record_number);
	}

	//unForw_intermediate_charging_ind;                              
	if ( 0 != CharToUInt(&pForwOut->unForw_intermediate_charging_ind, pForwInOld->Forw_intermediate_charging_ind, 1) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_intermediate_charging_ind);
	}

	//szForw_number_of_ss_records
	if (0 != CharToStringReverse(pForwOut->szForw_number_of_ss_records, pForwInOld->Forw_number_of_ss_records, 1) )
	{
		nRe = 10;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_number_of_ss_records);
	}
	
	//char Forw_cause_for_forwarding[1];                                     //No.10
	if ( 0 != CharToUInt(&pForwOut->unForw_cause_for_forwarding, pForwInOld->Forw_cause_for_forwarding, 1) )
	{
		nRe = 11;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_cause_for_forwarding);
	}
	
	//szForw_forwarding_imsi
	if (0 != CharToStringReverse(pForwOut->szForw_forwarding_imsi, pForwInOld->Forw_forwarding_imsi, 8) )
	{
		nRe = 12;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_imsi);
	}
	
	//szForw_forwarding_imei
	if (0 != CharToStringReverse(pForwOut->szForw_forwarding_imei, pForwInOld->Forw_forwarding_imei, 8) )
	{
		nRe = 13;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_imei);
	}
	
	//Forw_forwarding_number
	if (0 != CharToCallNumber(pForwOut->szForw_forwarding_number, pForwInOld->Forw_forwarding_number, 10) )
	{
		nRe = 14;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_number);
	}
	
	
	//Forw_forwarding_category;                                         
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_category, pForwInOld->Forw_forwarding_category, 1) )
	{
		nRe = 15;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_category);
	}
	
	//Forw_forwarding_ms_classmark;                                     
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_ms_classmark, pForwInOld->Forw_forwarding_ms_classmark, 1) )
	{
		nRe = 16;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_ms_classmark);
	}

	//szForw_forwarded_imsi
	if (0 != CharToStringReverse(pForwOut->szForw_forwarded_imsi, pForwInOld->Forw_forwarded_imsi, 8) )
	{
		nRe = 17;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_imsi);
	}
	
	//szForw_forwarded_imei
	if (0 != CharToStringReverse(pForwOut->szForw_forwarded_imei, pForwInOld->Forw_forwarded_imei, 8) )
	{
		nRe = 18;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_imei);
	}
	
	//Forw_forwarded_number
	if (0 != CharToCallNumber(pForwOut->szForw_forwarded_number, pForwInOld->Forw_forwarded_number, 10) )
	{
		nRe = 19;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_number);
	}
	
	//Forw_forwarded_ms_classmark;                                     
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarded_ms_classmark, pForwInOld->Forw_forwarded_ms_classmark, 1) )
	{
		nRe = 20;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarded_ms_classmark);
	}

	//Forw_orig_calling_number[10];                                     //No.21
	if (0 != CharToCallNumber(pForwOut->szForw_orig_calling_number, pForwInOld->Forw_orig_calling_number, 2) )
	{
		nRe = 21;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_orig_calling_number);
	}

	// pForw_in_circuit_group[2];    
	if (0 != CharToStringReverse(pForwOut->szForw_in_circuit_group, pForwInOld->Forw_in_circuit_group, 2) )
	{
		nRe = 22;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_in_circuit_group);
	}
	
	// szForw_out_circuit[2];  
	if (0 != CharToStringReverse(pForwOut->szForw_in_circuit, pForwInOld->Forw_in_circuit, 2) )
	{
		nRe = 23;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_in_circuit);
	}

	// Forw_forwarding_subs_first_lac;                                   [2];                             
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_subs_first_lac, pForwInOld->Forw_forwarding_subs_first_lac, 2) )
	{
		nRe = 24;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_subs_first_lac);
	}	

	// Forw_forwarding_subs_first_ci;                                                                 
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_subs_first_ci, pForwInOld->Forw_forwarding_subs_first_ci, 2) )
	{
		nRe = 25;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_subs_first_ci);
	}	

	// Forw_forwarding_subs_last_ex_id[10];                           
	if (0 != CharToStringReverse(pForwOut->szForw_forwarding_subs_last_ex_id, pForwInOld->Forw_forwarding_subs_last_ex_id, 10) )
	{
		nRe = 26;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_subs_last_ex_id);

	}

	// Forw_forwarding_subs_last_lac;                                                                 
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_subs_last_lac, pForwInOld->Forw_forwarding_subs_last_lac, 2) )
	{
		nRe = 27;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_subs_last_lac);
	}	

	// Forw_forwarding_subs_last_ci;                                     [2];   
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_subs_last_ci, pForwInOld->Forw_forwarding_subs_last_ci, 2) )
	{
		nRe = 28;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_subs_last_ci);
	}	
	
	// pForw_out_circuit_group[2];    
	if (0 != CharToStringReverse(pForwOut->szForw_out_circuit_group, pForwInOld->Forw_out_circuit_group, 2) )
	{
		nRe = 29;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_out_circuit_group);
	}
	
	// szForw_out_circuit[2];  
	if (0 != CharToStringReverse(pForwOut->szForw_out_circuit, pForwInOld->Forw_out_circuit, 2) )
	{
		nRe = 30;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_out_circuit);
	}

	// unForw_basic_service_type;                                      [1];   
	if ( 0 != CharToUInt(&pForwOut->unForw_basic_service_type, pForwInOld->Forw_basic_service_type, 1) )
	{
		nRe = 31;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_basic_service_type);
	}	
	
	// unForw_basic_service_code;                                      [1];  
	if ( 0 != CharToUInt(&pForwOut->unForw_basic_service_code, pForwInOld->Forw_basic_service_code, 1) )
	{
		nRe = 32;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_basic_service_code);
	}	
	
	// unForw_non_transparency_indicator;                              [1];   
	if ( 0 != CharToUInt(&pForwOut->unForw_non_transparency_indicator, pForwInOld->Forw_non_transparency_indicator, 1) )
	{
		nRe = 33;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_non_transparency_indicator);
	}	
	
	// unForw_channel_rate_indicator;                                  [1];    
	if ( 0 != CharToUInt(&pForwOut->unForw_channel_rate_indicator, pForwInOld->Forw_channel_rate_indicator, 1) )
	{
		nRe = 34;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_channel_rate_indicator);
	}	
	
	// szForw_set_up_start_time[7];    
	if( 0 != CharToDateTime(pForwOut->szForw_set_up_start_time, pForwInOld->Forw_set_up_start_time) )
	{
		nRe = 35;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_set_up_start_time);
	}

	// Forw_in_channel_allocated_time[7];                         
	if( 0 != CharToDateTime(pForwOut->szForw_in_channel_allocated_time, pForwInOld->Forw_in_channel_allocated_time) )
	{
		nRe = 36;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_in_channel_allocated_time);
	}

	// Forw_charging_start_time[7];                               
	if( 0 != CharToDateTime(pForwOut->szForw_charging_start_time, pForwInOld->Forw_charging_start_time) )
	{
		nRe = 37;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_charging_start_time);
	}

	// Forw_charging_end_time[7];  
	if( 0 != CharToDateTime(pForwOut->szForw_charging_end_time, pForwInOld->Forw_charging_end_time) )
	{
		nRe = 38;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_charging_end_time);
	}
	
	// unForw_forw_mcz_duration; 
	pTemp1 = szTemp; //pForwOut->unForw_forw_mcz_duration;
	if( 0 != CharToBcdHalfWord(pTemp1, pForwInOld->Forw_forw_mcz_duration, 3) )
	{
		nRe = 39;
		goto Exit;
	}
	sscanf(pTemp1,"%d",&pForwOut->unForw_forw_mcz_duration);
	pTemp1 = NULL;
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forw_mcz_duration);
	}
	
	// unForw_cause_for_termination;                                   [4]
	if( 0 != CharToUInt(&pForwOut->unForw_cause_for_termination, pForwInOld->Forw_cause_for_termination, 4) )
	{
		nRe = 40;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_cause_for_termination);
	}
	
	// Forw_data_volume[2];   
	if( 0 != CharToBcdHalfWord(pForwOut->szForw_data_volume, pForwInOld->Forw_data_volume, 2) )
	{
		nRe = 41;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_data_volume);
	}

	// Forw_call_type[1];    
	if( 0 != CharToUInt(&pForwOut->unForw_call_type, pForwInOld->Forw_call_type, 1) )
	{
		nRe = 42;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_call_type);
	}
	
	// Forw_forw_mcz_tariff_class[3];       
	if( 0 != CharToBcdHalfWord(pForwOut->szForw_forw_mcz_tariff_class, pForwInOld->Forw_forw_mcz_tariff_class, 3) )
	{
		nRe = 43;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forw_mcz_tariff_class);
	}

	// Forw_orig_mcz_pulses[2];     
	if( 0 != CharToBcdHalfWord(pForwOut->szForw_forw_mcz_pulses, pForwInOld->Forw_forw_mcz_pulses, 2) )
	{
		nRe = 44;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forw_mcz_pulses);
	}
	
	// Forw_dtmf_indicator[1];   
	if( 0 != CharToUInt(&pForwOut->unForw_dtmf_indicator, pForwInOld->Forw_dtmf_indicator, 1) )
	{
		nRe = 45;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_dtmf_indicator);
	}

	// Forw_aoc_indicator[1];
	if( 0 != CharToUInt(&pForwOut->unForw_aoc_indicator, pForwInOld->Forw_aoc_indicator, 1) )
	{
		nRe = 46;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_aoc_indicator);
	}

// 	char Forw_forwarded_number_ton[1];                                       //No.53
	if( 0 != CharToUInt(&pForwOut->unForw_forwarded_number_ton, pForwInOld->Forw_forwarded_number_ton, 1) )
	{
		nRe = 47;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarded_number_ton);
	}

// 	char Forw_forwarded_msrn_ton[1];                                         //No.51
	if( 0 != CharToUInt(&pForwOut->unForw_forwarded_msrn_ton, pForwInOld->Forw_forwarded_msrn_ton, 1) )
	{
		nRe = 48;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarded_msrn_ton);
	}

// 	char Forw_forwarded_msrn[12];                                            //No.52
	if( 0 != CharToStringReverse(pForwOut->szForw_forwarded_msrn, pForwInOld->Forw_forwarded_msrn, 12) )
	{
		nRe = 49;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_msrn);
	}

	// Forw_facility_usage[4]; 
	if( 0 != CharToBcdHalfWord(pForwOut->szForw_facility_usage, pForwInOld->Forw_facility_usage, 4) )
	{
		nRe = 50;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_facility_usage);
	}

	// Forw_forw_mcz_chrg_type;                                      [1];                                
	if( 0 != CharToUInt(&pForwOut->unForw_forw_mcz_chrg_type, pForwInOld->Forw_forw_mcz_chrg_type, 1) )
	{
		nRe = 51;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forw_mcz_chrg_type);
	}

	// Forw_forwarding_number_ton[1];                                
	if( 0 != CharToUInt(&pForwOut->unForw_forwarding_number_ton, pForwInOld->Forw_forwarding_number_ton, 1) )
	{
		nRe = 52;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarding_number_ton);
	}
	
	//Forw_orig_calling_number_ton
	if( 0 != CharToUInt(&pForwOut->unForw_orig_calling_number_ton, pForwInOld->Forw_orig_calling_number_ton, 1) )
	{
		nRe = 53;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_orig_calling_number_ton);
	}

	// Forw_routing_category[1];                                  
	if( 0 != CharToUInt(&pForwOut->unForw_routing_category, pForwInOld->Forw_routing_category, 1) )
	{
		nRe = 54;
		goto Exit;
	}
	if (g_nDebug)
	{

		printf("%d\n", pForwOut->unForw_routing_category);
	}

	// Forw_intermediate_chrg_cause[2];   
	if( 0 != CharToUInt(&pForwOut->unForw_intermediate_chrg_cause, pForwInOld->Forw_intermediate_chrg_cause, 2) )
	{
		nRe = 55;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_intermediate_chrg_cause);
	}
	
	// szForw_camel_call_reference[8];  
	if( 0 != CharToStringForward(pForwOut->szForw_camel_call_reference, pForwInOld->Forw_camel_call_reference, 8) )
	{
		nRe = 56;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_camel_call_reference);
	}
	
	// Forw_camel_exchange_id_ton[1];                             
	if( 0 != CharToUInt(&pForwOut->unForw_camel_exchange_id_ton, pForwInOld->Forw_camel_exchange_id_ton, 1) )
	{
		nRe = 57;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_camel_exchange_id_ton);
	}

	// Forw_camel_exchange_id[9]; 
	if( 0 != CharToStringReverse(pForwOut->szForw_camel_exchange_id, pForwInOld->Forw_camel_exchange_id, 9) )
	{
		nRe = 58;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_camel_exchange_id);
	}
	
	//Forw_orig_dialling_class
	if( 0 != CharToUInt(&pForwOut->unForw_orig_dialling_class, pForwInOld->Forw_orig_dialling_class, 2) )
	{
		nRe = 59;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_orig_dialling_class);
	}

	// Forw_virtual_msc_id[10];   
	if( 0 != CharToStringReverse(pForwOut->szForw_virtual_msc_id, pForwInOld->Forw_virtual_msc_id, 10) )
	{
		nRe = 60;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_virtual_msc_id);
	}
		
	// Forw_scf_address_ton[1];    
	if( 0 != CharToUInt(&pForwOut->unForw_scf_address_ton, pForwInOld->Forw_scf_address_ton, 1) )
	{
		nRe = 61;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_scf_address_ton);
	}
	
	// Forw_scf_address[9];       
	if( 0 != CharToStringReverse(pForwOut->szForw_scf_address, pForwInOld->Forw_scf_address, 9) )
	{
		nRe = 62;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_scf_address);
	}
	
	
	// Forw_destination_number_ton[1];                            
	if( 0 != CharToUInt(&pForwOut->unForw_destination_number_ton, pForwInOld->Forw_destination_number_ton, 1) )
	{
		nRe = 63;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_destination_number_ton);
	}
	
	// Forw_destination_number_npi[1];                            
	if( 0 != CharToUInt(&pForwOut->unForw_destination_number_npi, pForwInOld->Forw_destination_number_npi, 1) )
	{
		nRe = 64;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_destination_number_npi);
	}
	
	// Forw_destination_number[16];  
	if( 0 != CharToStringReverse(pForwOut->szForw_destination_number, pForwInOld->Forw_destination_number, 16) )
	{
		nRe = 65;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_destination_number);
	}
	
	// Forw_camel_service_key[4];                                 
	if( 0 != CharToStringReverse(pForwOut->szForw_camel_service_key, pForwInOld->Forw_camel_service_key, 4) )
	{
		nRe = 66;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_camel_service_key);
	}

	// Forw_forwarding_imeisv[8];                                     
	if( 0 != CharToStringReverse(pForwOut->szForw_forwarding_imeisv, pForwInOld->Forw_forwarding_imeisv, 8) )
	{
		nRe = 67;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_imeisv);
	}
	// Forw_forwarded_imeisv[8];                                     
	if( 0 != CharToStringReverse(pForwOut->szForw_forwarded_imeisv, pForwInOld->Forw_forwarded_imeisv, 8) )
	{
		nRe = 68;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_imeisv);
	}

// 	char szForw_camel_service_data[113];                                     //No.75
	if( 0 != CharToStringForward(pForwOut->szForw_camel_service_data, pForwInOld->Forw_camel_service_data, 56) )
	{
		nRe = 69;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_camel_service_data);
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}

/************************************************************************/
/* 解析Forw函数 */
/* Parameters: pForwOut 输出结构体 */
/* Parameters: pForwIn 输入结构体体部分 */
/* Parameters: pSegHandleOut 输入结构体头部分 */
/************************************************************************/
int ParseForw( SForwOut* pForwOut, SForwIn* pForwIn, SSegHandleOut* pSegHandleOut )
{
	//返回值
	int nRe = 0 ; 
	char* pTemp1 = NULL;
	unsigned char* pTemp2 = NULL;
	char  szTemp[255];
	/************************************************************************/
	/* 转化开始 */
	/************************************************************************/
	memset(pForwOut,0,sizeof(SForwOut));
	//unForw_record_length;                                           
 	pForwOut->unForw_record_length = pSegHandleOut->unRecord_length;
 	//eForw_record_type;                                             
 	pForwOut->eForw_record_type = pSegHandleOut->eRecord_type;

 	//szForw_record_number
	if (0 != CharToBcdHalfWord(pForwOut->szForw_record_number, pForwIn->Forw_record_number, 4) )
	{
		nRe = 1;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_record_number);
	}

	//szForw_record_status
	switch(pForwIn->Forw_record_status[0])
	{
	case 0x00:
		strcpy(pForwOut->szForw_record_status,"normal ok");
		break;
	case 0x01:
		strcpy(pForwOut->szForw_record_status,"synchronising error");
		break;
	case 0x02:
		strcpy(pForwOut->szForw_record_status,"different contents");
		break;
	default:
		nRe = 2;
		goto Exit;
		break;
	}
	if (g_nDebug)
	{
		printf("%s\n",pForwOut->szForw_record_status);
	}

	//unForw_check_sum;                                               
	if ( 0 != CharToUInt(&pForwOut->unForw_check_sum, pForwIn->Forw_check_sum, 2) )
	{
		nRe = 3;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_check_sum);
	}

	//szForw_call_reference
	//comp+process+focus
	pTemp1 = pForwOut->szForw_call_reference;
	pTemp2 = pForwIn->Forw_call_reference;
	strcpy(pTemp1, "comp:");
	pTemp1 += 5;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 4;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " process:");
	pTemp1 += 9;
	pTemp2 += 2;
	if (0 != CharToBcdHalfWord(pTemp1, pTemp2, 2) )
	{
		nRe = 5;
		goto Exit;
	}
	pTemp1 += 4;
	strcpy(pTemp1, " focus:");
	pTemp1 += 7;	
	pTemp2 += 2;
	if (0 != CharToStringForward(pTemp1, pTemp2, 1) )
	{
		nRe = 6;
		goto Exit;
	}
	pTemp1[2] = '\0';
	pTemp1 = NULL;
	pTemp2 = NULL;
	if (g_nDebug)
	{
		printf("%s\n",pForwOut->szForw_call_reference);
	}

	//szForw_exchange_id
	if (0 != CharToStringReverse(pForwOut->szForw_exchange_id, pForwIn->Forw_exchange_id, 10) )
	{
		nRe = 7;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_exchange_id);
	}
	
	//szForw_intermediate_record_number
	if (0 != CharToStringReverse(pForwOut->szForw_intermediate_record_number, pForwIn->Forw_intermediate_record_number, 1) )
	{
		nRe = 8;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_intermediate_record_number);
	}

	//unForw_intermediate_charging_ind;                              
	if ( 0 != CharToUInt(&pForwOut->unForw_intermediate_charging_ind, pForwIn->Forw_intermediate_charging_ind, 1) )
	{
		nRe = 9;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_intermediate_charging_ind);
	}

	//szForw_number_of_ss_records
	if (0 != CharToStringReverse(pForwOut->szForw_number_of_ss_records, pForwIn->Forw_number_of_ss_records, 1) )
	{
		nRe = 10;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_number_of_ss_records);
	}
	
	//char Forw_cause_for_forwarding[1];                                     //No.10
	if ( 0 != CharToUInt(&pForwOut->unForw_cause_for_forwarding, pForwIn->Forw_cause_for_forwarding, 1) )
	{
		nRe = 11;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_cause_for_forwarding);
	}
	
	//szForw_forwarding_imsi
	if (0 != CharToStringReverse(pForwOut->szForw_forwarding_imsi, pForwIn->Forw_forwarding_imsi, 8) )
	{
		nRe = 12;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_imsi);
	}
	
	//szForw_forwarding_imei
	if (0 != CharToStringReverse(pForwOut->szForw_forwarding_imei, pForwIn->Forw_forwarding_imei, 8) )
	{
		nRe = 13;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_imei);
	}
	
	//Forw_forwarding_number
	if (0 != CharToCallNumber(pForwOut->szForw_forwarding_number, pForwIn->Forw_forwarding_number, 10) )
	{
		nRe = 14;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_number);
	}
	
	
	//Forw_forwarding_category;                                         
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_category, pForwIn->Forw_forwarding_category, 1) )
	{
		nRe = 15;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_category);
	}
	
	//Forw_forwarding_ms_classmark;                                     
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_ms_classmark, pForwIn->Forw_forwarding_ms_classmark, 1) )
	{
		nRe = 16;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_ms_classmark);
	}

	//szForw_forwarded_imsi
	if (0 != CharToStringReverse(pForwOut->szForw_forwarded_imsi, pForwIn->Forw_forwarded_imsi, 8) )
	{
		nRe = 17;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_imsi);
	}
	
	//szForw_forwarded_imei
	if (0 != CharToStringReverse(pForwOut->szForw_forwarded_imei, pForwIn->Forw_forwarded_imei, 8) )
	{
		nRe = 18;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_imei);
	}
	
	//Forw_forwarded_number
	if (0 != CharToCallNumber(pForwOut->szForw_forwarded_number, pForwIn->Forw_forwarded_number, 10) )
	{
		nRe = 19;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_number);
	}
	
	//Forw_forwarded_ms_classmark;                                     
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarded_ms_classmark, pForwIn->Forw_forwarded_ms_classmark, 1) )
	{
		nRe = 20;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarded_ms_classmark);
	}

	//Forw_orig_calling_number[10];                                     //No.21
	if (0 != CharToCallNumber(pForwOut->szForw_orig_calling_number, pForwIn->Forw_orig_calling_number, 2) )
	{
		nRe = 21;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_orig_calling_number);
	}

	// pForw_in_circuit_group[2];    
	if (0 != CharToStringReverse(pForwOut->szForw_in_circuit_group, pForwIn->Forw_in_circuit_group, 2) )
	{
		nRe = 22;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_in_circuit_group);
	}
	
	// szForw_out_circuit[2];  
	if (0 != CharToStringReverse(pForwOut->szForw_in_circuit, pForwIn->Forw_in_circuit, 2) )
	{
		nRe = 23;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_in_circuit);
	}

	// Forw_forwarding_subs_first_lac;                                   [2];                             
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_subs_first_lac, pForwIn->Forw_forwarding_subs_first_lac, 2) )
	{
		nRe = 24;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_subs_first_lac);
	}	

	// Forw_forwarding_subs_first_ci;                                                                 
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_subs_first_ci, pForwIn->Forw_forwarding_subs_first_ci, 2) )
	{
		nRe = 25;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_subs_first_ci);
	}	

	// Forw_forwarding_subs_last_ex_id[10];                           
	if (0 != CharToStringReverse(pForwOut->szForw_forwarding_subs_last_ex_id, pForwIn->Forw_forwarding_subs_last_ex_id, 10) )
	{
		nRe = 26;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_subs_last_ex_id);
	}

	// Forw_forwarding_subs_last_lac;                                                                 
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_subs_last_lac, pForwIn->Forw_forwarding_subs_last_lac, 2) )
	{
		nRe = 27;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_subs_last_lac);
	}	

	// Forw_forwarding_subs_last_ci;                                     [2];   
	if ( 0 != CharToUInt(&pForwOut->unForw_forwarding_subs_last_ci, pForwIn->Forw_forwarding_subs_last_ci, 2) )
	{
		nRe = 28;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_forwarding_subs_last_ci);
	}	
	
	// pForw_out_circuit_group[2];    
	if (0 != CharToStringReverse(pForwOut->szForw_out_circuit_group, pForwIn->Forw_out_circuit_group, 2) )
	{
		nRe = 29;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_out_circuit_group);
	}
	
	// szForw_out_circuit[2];  
	if (0 != CharToStringReverse(pForwOut->szForw_out_circuit, pForwIn->Forw_out_circuit, 2) )
	{
		nRe = 30;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_out_circuit);
	}

	// unForw_basic_service_type;                                      [1];   
	if ( 0 != CharToUInt(&pForwOut->unForw_basic_service_type, pForwIn->Forw_basic_service_type, 1) )
	{
		nRe = 31;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_basic_service_type);
	}	
	
	// unForw_basic_service_code;                                      [1];  
	if ( 0 != CharToUInt(&pForwOut->unForw_basic_service_code, pForwIn->Forw_basic_service_code, 1) )
	{
		nRe = 32;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_basic_service_code);
	}	
	
	// unForw_non_transparency_indicator;                              [1];   
	if ( 0 != CharToUInt(&pForwOut->unForw_non_transparency_indicator, pForwIn->Forw_non_transparency_indicator, 1) )
	{
		nRe = 33;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_non_transparency_indicator);
	}	
	
	// unForw_channel_rate_indicator;                                  [1];    
	if ( 0 != CharToUInt(&pForwOut->unForw_channel_rate_indicator, pForwIn->Forw_channel_rate_indicator, 1) )
	{
		nRe = 34;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n",pForwOut->unForw_channel_rate_indicator);
	}	
	
	// szForw_set_up_start_time[7];    
	if( 0 != CharToDateTime(pForwOut->szForw_set_up_start_time, pForwIn->Forw_set_up_start_time) )
	{
		nRe = 35;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_set_up_start_time);
	}

	// Forw_in_channel_allocated_time[7];                         
	if( 0 != CharToDateTime(pForwOut->szForw_in_channel_allocated_time, pForwIn->Forw_in_channel_allocated_time) )
	{
		nRe = 36;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_in_channel_allocated_time);
	}

	// Forw_charging_start_time[7];                               
	if( 0 != CharToDateTime(pForwOut->szForw_charging_start_time, pForwIn->Forw_charging_start_time) )
	{
		nRe = 37;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_charging_start_time);
	}

	// Forw_charging_end_time[7];  
	if( 0 != CharToDateTime(pForwOut->szForw_charging_end_time, pForwIn->Forw_charging_end_time) )
	{
		nRe = 38;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_charging_end_time);
	}
	
	// unForw_forw_mcz_duration; 
	pTemp1 = szTemp; //pForwOut->unForw_forw_mcz_duration;
	if( 0 != CharToBcdHalfWord(pTemp1, pForwIn->Forw_forw_mcz_duration, 3) )
	{
		nRe = 39;
		goto Exit;
	}
	sscanf(pTemp1,"%d",&pForwOut->unForw_forw_mcz_duration);
	pTemp1 = NULL;
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forw_mcz_duration);
	}
	
	// unForw_cause_for_termination;                                   [4]
	if( 0 != CharToUInt(&pForwOut->unForw_cause_for_termination, pForwIn->Forw_cause_for_termination, 4) )
	{
		nRe = 40;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_cause_for_termination);
	}
	
	// Forw_data_volume[2];   
	if( 0 != CharToBcdHalfWord(pForwOut->szForw_data_volume, pForwIn->Forw_data_volume, 2) )
	{
		nRe = 41;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_data_volume);
	}

	// Forw_call_type[1];    
	if( 0 != CharToUInt(&pForwOut->unForw_call_type, pForwIn->Forw_call_type, 1) )
	{
		nRe = 42;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_call_type);
	}
	
	// Forw_forw_mcz_tariff_class[3];       
	if( 0 != CharToBcdHalfWord(pForwOut->szForw_forw_mcz_tariff_class, pForwIn->Forw_forw_mcz_tariff_class, 3) )
	{
		nRe = 43;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forw_mcz_tariff_class);
	}

	// Forw_orig_mcz_pulses[2];     
	if( 0 != CharToBcdHalfWord(pForwOut->szForw_forw_mcz_pulses, pForwIn->Forw_forw_mcz_pulses, 2) )
	{
		nRe = 44;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forw_mcz_pulses);
	}
	
	// Forw_dtmf_indicator[1];   
	if( 0 != CharToUInt(&pForwOut->unForw_dtmf_indicator, pForwIn->Forw_dtmf_indicator, 1) )
	{
		nRe = 45;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_dtmf_indicator);
	}

	// Forw_aoc_indicator[1];
	if( 0 != CharToUInt(&pForwOut->unForw_aoc_indicator, pForwIn->Forw_aoc_indicator, 1) )
	{
		nRe = 46;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_aoc_indicator);
	}

// 	char Forw_forwarded_number_ton[1];                                       //No.53
	if( 0 != CharToUInt(&pForwOut->unForw_forwarded_number_ton, pForwIn->Forw_forwarded_number_ton, 1) )
	{
		nRe = 47;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarded_number_ton);
	}

// 	char Forw_forwarded_msrn_ton[1];                                         //No.51
	if( 0 != CharToUInt(&pForwOut->unForw_forwarded_msrn_ton, pForwIn->Forw_forwarded_msrn_ton, 1) )
	{
		nRe = 48;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarded_msrn_ton);
	}

// 	char Forw_forwarded_msrn[12];                                            //No.52
	if( 0 != CharToStringReverse(pForwOut->szForw_forwarded_msrn, pForwIn->Forw_forwarded_msrn, 12) )
	{
		nRe = 49;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_msrn);
	}

	// Forw_facility_usage[4]; 
	if( 0 != CharToBcdHalfWord(pForwOut->szForw_facility_usage, pForwIn->Forw_facility_usage, 4) )
	{
		nRe = 50;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_facility_usage);
	}

	// Forw_forw_mcz_chrg_type;                                      [1];                                
	if( 0 != CharToUInt(&pForwOut->unForw_forw_mcz_chrg_type, pForwIn->Forw_forw_mcz_chrg_type, 1) )
	{
		nRe = 51;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forw_mcz_chrg_type);
	}

	// Forw_forwarding_number_ton[1];                                
	if( 0 != CharToUInt(&pForwOut->unForw_forwarding_number_ton, pForwIn->Forw_forwarding_number_ton, 1) )
	{
		nRe = 52;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarding_number_ton);
	}
	
	//Forw_orig_calling_number_ton
	if( 0 != CharToUInt(&pForwOut->unForw_orig_calling_number_ton, pForwIn->Forw_orig_calling_number_ton, 1) )
	{
		nRe = 53;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_orig_calling_number_ton);
	}

	// Forw_routing_category[1];                                  
	if( 0 != CharToUInt(&pForwOut->unForw_routing_category, pForwIn->Forw_routing_category, 1) )
	{
		nRe = 54;
		goto Exit;
	}
	if (g_nDebug)
	{

		printf("%d\n", pForwOut->unForw_routing_category);
	}

	// Forw_intermediate_chrg_cause[2];   
	if( 0 != CharToUInt(&pForwOut->unForw_intermediate_chrg_cause, pForwIn->Forw_intermediate_chrg_cause, 2) )
	{
		nRe = 55;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_intermediate_chrg_cause);
	}
	
	// szForw_camel_call_reference[8];  
	if( 0 != CharToStringForward(pForwOut->szForw_camel_call_reference, pForwIn->Forw_camel_call_reference, 8) )
	{
		nRe = 56;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_camel_call_reference);
	}
	
	// Forw_camel_exchange_id_ton[1];                             
	if( 0 != CharToUInt(&pForwOut->unForw_camel_exchange_id_ton, pForwIn->Forw_camel_exchange_id_ton, 1) )
	{
		nRe = 57;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_camel_exchange_id_ton);
	}

	// Forw_camel_exchange_id[9]; 
	if( 0 != CharToStringReverse(pForwOut->szForw_camel_exchange_id, pForwIn->Forw_camel_exchange_id, 9) )
	{
		nRe = 58;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_camel_exchange_id);
	}
	
	//Forw_orig_dialling_class
	if( 0 != CharToUInt(&pForwOut->unForw_orig_dialling_class, pForwIn->Forw_orig_dialling_class, 2) )
	{
		nRe = 59;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_orig_dialling_class);
	}

	// Forw_virtual_msc_id[10];   
	if( 0 != CharToStringReverse(pForwOut->szForw_virtual_msc_id, pForwIn->Forw_virtual_msc_id, 10) )
	{
		nRe = 60;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_virtual_msc_id);
	}
		
	// Forw_scf_address_ton[1];    
	if( 0 != CharToUInt(&pForwOut->unForw_scf_address_ton, pForwIn->Forw_scf_address_ton, 1) )
	{
		nRe = 61;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_scf_address_ton);
	}
	
	// Forw_scf_address[9];       
	if( 0 != CharToStringReverse(pForwOut->szForw_scf_address, pForwIn->Forw_scf_address, 9) )
	{
		nRe = 62;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_scf_address);
	}
	
	
	// Forw_destination_number_ton[1];                            
	if( 0 != CharToUInt(&pForwOut->unForw_destination_number_ton, pForwIn->Forw_destination_number_ton, 1) )
	{
		nRe = 63;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_destination_number_ton);
	}
	
	// Forw_destination_number_npi[1];                            
	if( 0 != CharToUInt(&pForwOut->unForw_destination_number_npi, pForwIn->Forw_destination_number_npi, 1) )
	{
		nRe = 64;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_destination_number_npi);
	}
	
	// Forw_destination_number[16];  
	if( 0 != CharToStringReverse(pForwOut->szForw_destination_number, pForwIn->Forw_destination_number, 16) )
	{
		nRe = 65;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_destination_number);
	}
	
	// Forw_camel_service_key[4];                                 
	if( 0 != CharToStringReverse(pForwOut->szForw_camel_service_key, pForwIn->Forw_camel_service_key, 4) )
	{
		nRe = 66;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_camel_service_key);
	}

	// Forw_forwarding_imeisv[8];                                     
	if( 0 != CharToStringReverse(pForwOut->szForw_forwarding_imeisv, pForwIn->Forw_forwarding_imeisv, 8) )
	{
		nRe = 67;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarding_imeisv);
	}
	// Forw_forwarded_imeisv[8];                                     
	if( 0 != CharToStringReverse(pForwOut->szForw_forwarded_imeisv, pForwIn->Forw_forwarded_imeisv, 8) )
	{
		nRe = 68;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_forwarded_imeisv);
	}
// 	unsigned int unForw_rate_adaption;   
	if( 0 != CharToUInt(&pForwOut->unForw_rate_adaption, pForwIn->Forw_rate_adaption, 1) )
	{
		nRe = 69;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_rate_adaption);
	}

// 	unsigned int unForw_ms_classmark3;   
	if( 0 != CharToUInt(&pForwOut->unForw_ms_classmark3, pForwIn->Forw_ms_classmark3, 1) )
	{
		nRe = 70;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_ms_classmark3);
	}

// 	unsigned int unForw_forwarding_cell_band;   
	if( 0 != CharToUInt(&pForwOut->unForw_forwarding_cell_band, pForwIn->Forw_forwarding_cell_band, 1) )
	{
		nRe = 71;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarding_cell_band);
	}

// 	unsigned int unForw_forwarding_last_ex_id_ton;  
	if( 0 != CharToUInt(&pForwOut->unForw_forwarding_last_ex_id_ton, pForwIn->Forw_forwarding_last_ex_id_ton, 1) )
	{
		nRe = 72;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarding_last_ex_id_ton);
	}

// 	unsigned int unForw_forwarded_to_last_ex_id_ton;  
	if( 0 != CharToUInt(&pForwOut->unForw_forwarded_to_last_ex_id_ton, pForwIn->Forw_forwarded_to_last_ex_id_ton, 1) )
	{
		nRe = 73;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarded_to_last_ex_id_ton);
	}

// 	unsigned int unForw_forwarding_first_mcc;      
	if( 0 != CharToUInt(&pForwOut->unForw_forwarding_first_mcc, pForwIn->Forw_forwarding_first_mcc, 2) )
	{
		nRe = 74;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarding_first_mcc);
	}

// 	unsigned int unForw_forwarding_first_mnc;   
	if( 0 != CharToUInt(&pForwOut->unForw_forwarding_first_mnc, pForwIn->Forw_forwarding_first_mnc, 2) )
	{
		nRe = 75;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarding_first_mnc);
	}

// 	unsigned int unForw_forwarding_last_mcc;         
	if( 0 != CharToUInt(&pForwOut->unForw_forwarding_last_mcc, pForwIn->Forw_forwarding_last_mcc, 2) )
	{
		nRe = 76;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarding_last_mcc);
	}

// 	unsigned int unForw_forwarding_last_mnc;        
	if( 0 != CharToUInt(&pForwOut->unForw_forwarding_last_mnc, pForwIn->Forw_forwarding_last_mnc, 2) )
	{
		nRe = 77;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarding_last_mnc);
	}

// 	unsigned int unForw_forwarded_to_last_mcc; 
	if( 0 != CharToUInt(&pForwOut->unForw_forwarded_to_last_mcc, pForwIn->Forw_forwarded_to_last_mcc, 2) )
	{
		nRe = 78;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarded_to_last_mcc);
	}

// 	unsigned int unForw_forwarded_to_last_mnc; 
	if( 0 != CharToUInt(&pForwOut->unForw_forwarded_to_last_mnc, pForwIn->Forw_forwarded_to_last_mnc, 2) )
	{
		nRe = 79;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_forwarded_to_last_mnc);
	}

// 	char szForw_camel_service_data[113];                                     //No.75
	if( 0 != CharToStringForward(pForwOut->szForw_camel_service_data, pForwIn->Forw_camel_service_data, 56) )
	{
		nRe = 80;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%s\n", pForwOut->szForw_camel_service_data);
	}

// 	unsigned int unForw_inside_user_plane_index; 
	if( 0 != CharToUInt(&pForwOut->unForw_inside_user_plane_index, pForwIn->Forw_inside_user_plane_index, 2) )
	{
		nRe = 81;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_inside_user_plane_index);
	}

// 	unsigned int unForw_inside_control_plane_index;  
	if( 0 != CharToUInt(&pForwOut->unForw_inside_control_plane_index, pForwIn->Forw_inside_control_plane_index, 2) )
	{
		nRe = 82;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_inside_control_plane_index);
	}

// 	unsigned int unForw_in_bnc_connection_type;    
	if( 0 != CharToUInt(&pForwOut->unForw_in_bnc_connection_type, pForwIn->Forw_in_bnc_connection_type, 1) )
	{
		nRe = 83;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_in_bnc_connection_type);
	}

// 	unsigned int unForw_outside_user_plane_index;    
	if( 0 != CharToUInt(&pForwOut->unForw_outside_user_plane_index, pForwIn->Forw_outside_user_plane_index, 2) )
	{
		nRe = 84;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_outside_user_plane_index);
	}

// 	unsigned int unForw_outside_control_plane_index;  
	if( 0 != CharToUInt(&pForwOut->unForw_outside_control_plane_index, pForwIn->Forw_outside_control_plane_index, 2) )
	{
		nRe = 85;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_outside_control_plane_index);
	}

// 	unsigned int unForw_out_bnc_connection_type;   
	if( 0 != CharToUInt(&pForwOut->unForw_out_bnc_connection_type, pForwIn->Forw_out_bnc_connection_type, 1) )
	{
		nRe = 86;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_out_bnc_connection_type);
	}

// 	unsigned int unForw_radio_network_type;                             
	if( 0 != CharToUInt(&pForwOut->unForw_radio_network_type, pForwIn->Forw_radio_network_type, 1) )
	{
		nRe = 87;
		goto Exit;
	}
	if (g_nDebug)
	{
		printf("%d\n", pForwOut->unForw_radio_network_type);
	}

/************************************************************************/
/* 出口 */
/************************************************************************/
Exit:
	return nRe;
}


/************************************************************************/
/* 生成moccsv函数 */
/************************************************************************/
static int GenerateMocCsvFile(char* lpSavePath, char* lpFileString, char* lpSrcFileName, SMocOut* pMocOut, FILE ** pFile, char* lpDevice)
{
	int   nRe = 0;
	char  szOutFileName[256];
	char  szTemp[256];
	char  szFileName[256];
	char* lpOutTemp1;
	char* lpOutTemp2;
	int   nI = 0;

	if(NULL == *pFile)
	{
		//  .../../000002_nsn_nbogs1_xxxxxx.dat
		lpOutTemp1 = szTemp;
		strcpy(lpOutTemp1, lpSrcFileName);
		//拼凑文件名
		while( NULL != ( lpOutTemp2 = strstr(lpOutTemp1, "/") ) )
		{
			lpOutTemp2 ++ ;
			lpOutTemp1 = lpOutTemp2;		
		}
		//截掉'.'后面的
		if ( NULL != ( lpOutTemp2 = strstr(lpOutTemp1, ".") ) )
		{
			lpOutTemp2[0] = '\0';
		}
		//lpOutTemp1:000002_nsn_nbogs1_xxxxxx
		lpOutTemp2 = lpOutTemp1;
		for ( ; nI < 3; nI ++)
		{
			if (NULL == ( lpOutTemp2 = strstr(lpOutTemp2, "_") ))
			{
				//文件名有问题
				nRe = 1 ; 
				goto Exit;				
			}
			lpOutTemp2 ++;
		}
		//lpOutTemp2:_xxxxxx
		lpOutTemp2 --;
		lpOutTemp2[0] = '\0';
		//lpOutTemp1:000002_nsn_nbogs1
		lpOutTemp2 ++;
		//lpOutTemp2:xxxxxx
		//拼凑
		sprintf(szFileName, 
			"%s_moc_%s.csv",
			lpOutTemp1, lpOutTemp2);
		sprintf(szOutFileName, 
			"%s/%s",
			lpSavePath, szFileName);
		
		//打开或建立文件
		if ( NULL == ( *pFile = fopen( szOutFileName, "a" ) ) )
		{ 
			//文件打开失败
			nRe = 1 ; 
			goto Exit;
		} 	
		strcat(lpFileString, szFileName);
		strcat(lpFileString, ";");
	}

	//写文件
	fprintf
	(
		*pFile, 
		"%s,%d,%d,%s,%s,%d,%s,%s,%s,%d,%s,%s,%s,%s,%d,%d,%s,%s,\
		%d,%s,%d,%d,%s,%d,%d,%s,%d,%d,%d,%d,%s,%d,%d,%s,\
		%s,%d,%d,%d,%d,%s,%s,%s,%s,%d,%d,%s,%d,%s,%s,%d,%d,\
		%d,%s,%d,%s,%d,%d,%d,%d,%s,%d,%s,%s,%d,%d,%d,%s,%d,\
		%s,%d,%d,%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\
		%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%d,%s,%s,%d,%d\n",
		lpDevice,
		pMocOut->unMoc_record_length,                                     
		pMocOut->eMoc_record_type,                                          
		pMocOut->szMoc_record_number,                               
		pMocOut->szMoc_record_status,                              
		pMocOut->unMoc_check_sum,                                          
		pMocOut->szMoc_call_reference,                             
		pMocOut->szMoc_exchange_id,                                
		pMocOut->szMoc_intermediate_record_number,                  
		pMocOut->unMoc_intermediate_charging_ind,                          
		pMocOut->szMoc_number_of_ss_records,                        
		pMocOut->szMoc_calling_imsi,                               
		pMocOut->szMoc_calling_imei,                               
		pMocOut->szMoc_calling_number,                             
		pMocOut->unMoc_calling_category,                                   
		pMocOut->unMoc_calling_ms_classmark,                               
		pMocOut->szMoc_called_imsi,                                
		pMocOut->szMoc_called_imei,                                
		pMocOut->unMoc_dialled_digits_ton,                                 
		pMocOut->szMoc_called_number,                              
		pMocOut->unMoc_called_category,                                    
		pMocOut->unMoc_called_ms_classmark,                                
		pMocOut->szMoc_dialled_digits,                             
		pMocOut->unMoc_calling_subs_first_lac,                             
		pMocOut->unMoc_calling_subs_first_ci,                              
		pMocOut->szMoc_calling_subs_last_ex_id,                    
		pMocOut->unMoc_calling_subs_last_lac,                              
		pMocOut->unMoc_calling_subs_last_ci,                               
		pMocOut->unMoc_called_subs_first_lac,                              
		pMocOut->unMoc_called_subs_first_ci,                               
		pMocOut->szMoc_called_subs_last_ex_id,                     
		pMocOut->unMoc_called_subs_last_lac,                               
		pMocOut->unMoc_called_subs_last_ci,                                
		pMocOut->szMoc_out_circuit_group,                           
		pMocOut->szMoc_out_circuit,                                 
		pMocOut->unMoc_basic_service_type,                                 
		pMocOut->unMoc_basic_service_code,                                 
		pMocOut->unMoc_non_transparency_indicator,                         
		pMocOut->unMoc_channel_rate_indicator,                             
		pMocOut->szMoc_set_up_start_time,                          
		pMocOut->szMoc_in_channel_allocated_time,                  
		pMocOut->szMoc_charging_start_time,                        
		pMocOut->szMoc_charging_end_time,                          
		pMocOut->unMoc_orig_mcz_duration,                          
		pMocOut->unMoc_cause_for_termination,                              
		pMocOut->szMoc_data_volume,                                 
		pMocOut->unMoc_call_type,                                          
		pMocOut->szMoc_orig_mcz_tariff_class,                       
		pMocOut->szMoc_orig_mcz_pulses,                             
		pMocOut->unMoc_dtmf_indicator,                                     
		pMocOut->unMoc_aoc_indicator,                                      
		pMocOut->unMoc_called_msrn_ton,                                    
		pMocOut->szMoc_called_msrn,                                
		pMocOut->unMoc_called_number_ton,                                  
		pMocOut->szMoc_facility_usage,                              
		pMocOut->unMoc_orig_mcz_chrg_type,                                 
		pMocOut->unMoc_calling_number_ton,                                 
		pMocOut->unMoc_routing_category,                                   
		pMocOut->unMoc_intermediate_chrg_cause,                            
		pMocOut->szMoc_camel_call_reference,                       
		pMocOut->unMoc_camel_exchange_id_ton,                              
		pMocOut->szMoc_camel_exchange_id,                          
		pMocOut->szMoc_calling_modify_parameters,                  
		pMocOut->unMoc_orig_mcz_modify_percent,                            
		pMocOut->unMoc_orig_mcz_modify_direction,                          
		pMocOut->unMoc_orig_dialling_class,                                
		pMocOut->szMoc_virtual_msc_id,                             
		pMocOut->unMoc_scf_address_ton,                                    
		pMocOut->szMoc_scf_address,                                
		pMocOut->unMoc_destination_number_ton,                             
		pMocOut->unMoc_destination_number_npi,                             
		pMocOut->szMoc_destination_number,                         
		pMocOut->szMoc_camel_service_key,                           
		pMocOut->szMoc_calling_imeisv,                      
		pMocOut->szMoc_called_imeisv,                       
		pMocOut->unMoc_emergency_call_category,         
		pMocOut->unMoc_used_air_interface_user_rate,      
		pMocOut->unMoc_req_air_interface_user_rate,       
		pMocOut->unMoc_used_fixed_nw_user_rate,          
		pMocOut->unMoc_req_fixed_nw_user_rate,           
		pMocOut->unMoc_rate_adaption,                   
		pMocOut->unMoc_stream_identifier,                 
		pMocOut->unMoc_ms_classmark3,                 
		pMocOut->unMoc_calling_cell_band,       
		pMocOut->unMoc_calling_subs_last_ex_id_ton,       
		pMocOut->unMoc_called_subs_last_ex_id_ton,       
		pMocOut->unMoc_calling_subs_first_mcc,            
		pMocOut->unMoc_calling_subs_first_mnc,            
		pMocOut->unMoc_calling_subs_last_mcc,             
		pMocOut->unMoc_calling_subs_last_mnc,             
		pMocOut->unMoc_called_subs_first_mcc,             
		pMocOut->unMoc_called_subs_first_mnc,             
		pMocOut->unMoc_called_subs_last_mcc,             
		pMocOut->unMoc_called_subs_last_mnc,             
		pMocOut->szMoc_camel_service_data,	     
		pMocOut->unMoc_selected_codec,			           
		pMocOut->szMoc_outside_user_plane_index,	
		pMocOut->szMoc_outside_control_plane_index,
		pMocOut->unMoc_out_bnc_connection_type,	          
		pMocOut->unMoc_radio_network_type
	);

Exit:
	return nRe;

}

/************************************************************************/
/* 生成mtccsv函数 */
/************************************************************************/
static int GenerateMtcCsvFile(char* lpSavePath, char* lpFileString, char* lpSrcFileName, SMtcOut* pMtcOut, FILE * *pFile, char* lpDevice)
{
	int nRe = 0;
	char szOutFileName[256];
	char szTemp[256];
	char szFileName[256];
	char* lpOutTemp1;
	char* lpOutTemp2;
	int nI = 0;

	if(NULL == *pFile)
	{
		//  .../../000002_nsn_nbogs1_xxxxxx.dat
		lpOutTemp1 = szTemp;
		strcpy(lpOutTemp1, lpSrcFileName);
		//拼凑文件名
		while( NULL != ( lpOutTemp2 = strstr(lpOutTemp1, "/") ) )
		{
			lpOutTemp2 ++ ;
			lpOutTemp1 = lpOutTemp2;		
		}
		//截掉'.'后面的
		if ( NULL != ( lpOutTemp2 = strstr(lpOutTemp1, ".") ) )
		{
			lpOutTemp2[0] = '\0';
		}
		//lpOutTemp1:000002_nsn_nbogs1_xxxxxx
		lpOutTemp2 = lpOutTemp1;
		for ( ; nI < 3; nI ++)
		{
			if (NULL == ( lpOutTemp2 = strstr(lpOutTemp2, "_") ))
			{
				//文件名有问题
				nRe = 1 ; 
				goto Exit;				
			}
			lpOutTemp2 ++;
		}
		//lpOutTemp2:_xxxxxx
		lpOutTemp2 --;
		lpOutTemp2[0] = '\0';
		//lpOutTemp1:000002_nsn_nbogs1
		lpOutTemp2 ++;
		//lpOutTemp2:xxxxxx
		//拼凑
		sprintf(szFileName, 
			"%s_mtc_%s.csv",
			lpOutTemp1, lpOutTemp2);
		sprintf(szOutFileName, 
			"%s/%s",
			lpSavePath, szFileName);

		//打开或建立文件
		if ( NULL == ( *pFile = fopen( szOutFileName, "a" ) ) )
		{ 
			//文件打开失败
			nRe = 1 ; 
			goto Exit;
		} 	
		strcat(lpFileString, szFileName);
		strcat(lpFileString, ";");
	}

	//写文件
	fprintf
	(
		*pFile, 
		"%s,%d,%d,%s,%s,%d,%s,%s,%s,%d,%s,%s,%s,%s,%s,%d,%d,%s,%s,\
		%d,%d,%s,%d,%d,%d,%d,%d,%d,%s,%s,%s,%s,%d,%d,%s,%d,\
		%s,%s,%d,%d,%s,%d,%d,%d,%d,%d,%s,%d,%s,%s,%d,%d,%s,\
		%d,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\
		%s,%s,%d,%d\n",
		lpDevice,
		pMtcOut->unMtc_record_length,                                           
		pMtcOut->eMtc_record_type,                                             
		pMtcOut->szMtc_record_number,                                           
		pMtcOut->szMtc_record_status,                                           
		pMtcOut->unMtc_check_sum,                                               
		pMtcOut->szMtc_call_reference,                                          
		pMtcOut->szMtc_exchange_id,                                            
		pMtcOut->szMtc_intermediate_record_number,                             
		pMtcOut->unMtc_intermediate_charging_ind,                              
		pMtcOut->szMtc_number_of_ss_records,                                    
		pMtcOut->szMtc_calling_number,                                          
		pMtcOut->szMtc_called_imsi,                                            
		pMtcOut->szMtc_called_imei,  
		pMtcOut->szMtc_called_number,                                          
		pMtcOut->unMtc_called_category,                                         
		pMtcOut->unMtc_called_ms_classmark,                                     
		pMtcOut->szMtc_in_circuit_group,                      
		pMtcOut->szMtc_in_circuit,                            
		pMtcOut->unMtc_called_subs_first_lac,                   
		pMtcOut->unMtc_called_subs_first_ci,                    
		pMtcOut->szMtc_called_subs_last_ex_id,               
		pMtcOut->unMtc_called_subs_last_lac,                    
		pMtcOut->unMtc_called_subs_last_ci,                     
		pMtcOut->unMtc_basic_service_type,                      
		pMtcOut->unMtc_basic_service_code,                      
		pMtcOut->unMtc_non_transparency_indicator,              
		pMtcOut->unMtc_channel_rate_indicator,                  
		pMtcOut->szMtc_set_up_start_time,                    
		pMtcOut->szMtc_in_channel_allocated_time,            
		pMtcOut->szMtc_charging_start_time,                  
		pMtcOut->szMtc_charging_end_time,                    
		pMtcOut->unMtc_term_mcz_duration,                    
		pMtcOut->unMtc_cause_for_termination,                   
		pMtcOut->szMtc_data_volume,                           
		pMtcOut->unMtc_call_type,                               
		pMtcOut->szMtc_term_mcz_tariff_class,                 
		pMtcOut->szMtc_term_mcz_pulses,                       
		pMtcOut->unMtc_dtmf_indicator,                          
		pMtcOut->unMtc_aoc_indicator,                           
		pMtcOut->szMtc_facility_usage,                        
		pMtcOut->unMtc_term_mcz_chrg_type,                      
		pMtcOut->unMtc_calling_number_ton,                      
		pMtcOut->unMtc_called_number_ton,                       
		pMtcOut->unMtc_routing_category,                        
		pMtcOut->unMtc_intermediate_chrg_cause,                 
		pMtcOut->szMtc_camel_call_reference,                 
		pMtcOut->unMtc_camel_exchange_id_ton,                   
		pMtcOut->szMtc_camel_exchange_id,                    
		pMtcOut->szMtc_calling_modify_parameters,            
		pMtcOut->unMtc_term_mcz_modify_percent,                 
		pMtcOut->unMtc_term_mcz_modify_direction,               
		pMtcOut->szMtc_redirecting_number,                   
		pMtcOut->unMtc_redirecting_number_ton,                  
		pMtcOut->szMtc_virtual_msc_id,                       
		pMtcOut->szMtc_called_imeisv,                        
		pMtcOut->unMtc_used_air_interface_user_rate,			
		pMtcOut->unMtc_req_air_interface_user_rate,				
		pMtcOut->unMtc_used_fixed_nw_user_rate,					
		pMtcOut->unMtc_req_fixed_nw_user_rate,					
		pMtcOut->unMtc_rate_adaption,							
		pMtcOut->unMtc_stream_identifier,						
		pMtcOut->unMtc_ms_classmark3,							
		pMtcOut->unMtc_called_cell_band,						
		pMtcOut->unMtc_called_subs_last_ex_id_ton,				
		pMtcOut->unMtc_called_subs_first_mcc,					
		pMtcOut->unMtc_called_subs_first_mnc,					
		pMtcOut->unMtc_called_subs_last_mcc,					
		pMtcOut->unMtc_called_subs_last_mnc,					
		pMtcOut->unMtc_selected_codec,							
		pMtcOut->szMtc_inside_user_plane_index,				
		pMtcOut->szMtc_inside_control_plane_index,			
		pMtcOut->unMtc_in_bnc_connection_type,					
		pMtcOut->unMtc_radio_network_type						
	);

Exit:
	return nRe;

}

/************************************************************************/
/* 生成Forwcsv函数 */
/************************************************************************/
static int GenerateForwCsvFile(char* lpSavePath, char* lpFileString, char* lpSrcFileName, SForwOut* pForwOut, FILE * *pFile, char* lpDevice)
{
	int nRe = 0;
	char szOutFileName[256];
	char szTemp[256];
	char szFileName[256];
	char* lpOutTemp1;
	char* lpOutTemp2;
	int nI = 0;
	
	if(NULL == *pFile)
	{
		//  .../../000002_nsn_nbogs1_xxxxxx.dat
		lpOutTemp1 = szTemp;
		strcpy(lpOutTemp1, lpSrcFileName);
		//拼凑文件名
		while( NULL != ( lpOutTemp2 = strstr(lpOutTemp1, "/") ) )
		{
			lpOutTemp2 ++ ;
			lpOutTemp1 = lpOutTemp2;		
		}
		//截掉'.'后面的
		if ( NULL != ( lpOutTemp2 = strstr(lpOutTemp1, ".") ) )
		{
			lpOutTemp2[0] = '\0';
		}
		//lpOutTemp1:000002_nsn_nbogs1_xxxxxx
		lpOutTemp2 = lpOutTemp1;
		for ( ; nI < 3; nI ++)
		{
			if (NULL == ( lpOutTemp2 = strstr(lpOutTemp2, "_") ))
			{
				//文件名有问题
				nRe = 1 ; 
				goto Exit;				
			}
			lpOutTemp2 ++;
		}
		//lpOutTemp2:_xxxxxx
		lpOutTemp2 --;
		lpOutTemp2[0] = '\0';
		//lpOutTemp1:000002_nsn_nbogs1
		lpOutTemp2 ++;
		//lpOutTemp2:xxxxxx
		//拼凑
		sprintf(szFileName, 
			"%s_forw_%s.csv",
			lpOutTemp1, lpOutTemp2);
		sprintf(szOutFileName, 
			"%s/%s",
			lpSavePath, szFileName);
		
		//打开或建立文件
		if ( NULL == ( *pFile = fopen( szOutFileName, "a" ) ) )
		{ 
			//文件打开失败
			nRe = 1 ; 
			goto Exit;
		} 	
		strcat(lpFileString, szFileName);
		strcat(lpFileString, ";");
	}

	//写文件
	fprintf
	(
		*pFile, 
		"%s,%d,%d,%s,%s,%d,%s,%s,%s,%d,%s,%d,%s,%s,%s,%d,%d,%s,\
		%s,%s,%d,%s,%s,%s,%d,%d,%s,%d,%d,%d,%d,%s,%d,%d,%s,\
		%s,%d,%d,%d,%d,%s,%s,%s,%s,%d,%d,%s,%d,%s,%s,%d,%d,\
		%d,%d,%s,%s,%d,%d,%d,%d,%d,%s,%d,%s,%d,%s,%d,%s,%d,\
		%d,%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,\
		,%d,%d,%d,%d,%d,%d,%d\n",
		lpDevice,
		pForwOut->unForw_record_length,                            
		pForwOut->eForw_record_type,                                 
		pForwOut->szForw_record_number,                           
		pForwOut->szForw_record_status,                          
		pForwOut->unForw_check_sum,                                 
		pForwOut->szForw_call_reference,                         
		pForwOut->szForw_exchange_id,                            
		pForwOut->szForw_intermediate_record_number,              
		pForwOut->unForw_intermediate_charging_ind,                 
		pForwOut->szForw_number_of_ss_records,                    
		pForwOut->unForw_cause_for_forwarding,                      
		pForwOut->szForw_forwarding_imsi,                        
		pForwOut->szForw_forwarding_imei,                        
		pForwOut->szForw_forwarding_number,                      
		pForwOut->unForw_forwarding_category,                       
		pForwOut->unForw_forwarding_ms_classmark,                   
		pForwOut->szForw_forwarded_imsi,                         
		pForwOut->szForw_forwarded_imei,                         
		pForwOut->szForw_forwarded_number,                       
		pForwOut->unForw_forwarded_ms_classmark,                    
		pForwOut->szForw_orig_calling_number,                    
		pForwOut->szForw_in_circuit_group,                        
		pForwOut->szForw_in_circuit,                              
		pForwOut->unForw_forwarding_subs_first_lac,                 
		pForwOut->unForw_forwarding_subs_first_ci,                  
		pForwOut->szForw_forwarding_subs_last_ex_id,                    
		pForwOut->unForw_forwarding_subs_last_lac,                  
		pForwOut->unForw_forwarding_subs_last_ci,                   
		pForwOut->unForw_forwarded_subs_first_lac,                  
		pForwOut->unForw_forwarded_subs_first_ci,                   
		pForwOut->szForw_forwarded_subs_last_ex_id,              
		pForwOut->unForw_forwarded_subs_last_lac,                   
		pForwOut->unForw_forwarded_subs_last_ci,                    
		pForwOut->szForw_out_circuit_group,                       
		pForwOut->szForw_out_circuit,                             
		pForwOut->unForw_basic_service_type,                        
		pForwOut->unForw_basic_service_code,                        
		pForwOut->unForw_non_transparency_indicator,                
		pForwOut->unForw_channel_rate_indicator,                    
		pForwOut->szForw_set_up_start_time,                      
		pForwOut->szForw_in_channel_allocated_time,              
		pForwOut->szForw_charging_start_time,                    
		pForwOut->szForw_charging_end_time,                      
		pForwOut->unForw_forw_mcz_duration,                      
		pForwOut->unForw_cause_for_termination,                     
		pForwOut->szForw_data_volume,                             
		pForwOut->unForw_call_type,                                 
		pForwOut->szForw_forw_mcz_tariff_class,                   
		pForwOut->szForw_forw_mcz_pulses,                         
		pForwOut->unForw_dtmf_indicator,                            
		pForwOut->unForw_aoc_indicator,                             
		pForwOut->unForw_forwarded_number_ton,                      
		pForwOut->unForw_forwarded_msrn_ton,                        
		pForwOut->szForw_forwarded_msrn,                         
		pForwOut->szForw_facility_usage,                          
		pForwOut->unForw_forw_mcz_chrg_type,                        
		pForwOut->unForw_forwarding_number_ton,                     
		pForwOut->unForw_orig_calling_number_ton,                   
		pForwOut->unForw_routing_category,                          
		pForwOut->unForw_intermediate_chrg_cause,                   
		pForwOut->szForw_camel_call_reference,                   
		pForwOut->unForw_camel_exchange_id_ton,                     
		pForwOut->szForw_camel_exchange_id,                      
		pForwOut->unForw_orig_dialling_class,                       
		pForwOut->szForw_virtual_msc_id,                         
		pForwOut->unForw_scf_address_ton,                           
		pForwOut->szForw_scf_address,                            
		pForwOut->unForw_destination_number_ton,                    
		pForwOut->unForw_destination_number_npi,                    
		pForwOut->szForw_destination_number,                     
		pForwOut->szForw_camel_service_key,                       
		pForwOut->szForw_forwarding_imeisv,                      
		pForwOut->szForw_forwarded_imeisv,                       
		pForwOut->unForw_rate_adaption,                                  
		pForwOut->unForw_ms_classmark3,                                  
		pForwOut->unForw_forwarding_cell_band,                           
		pForwOut->unForw_forwarding_last_ex_id_ton,                      
		pForwOut->unForw_forwarded_to_last_ex_id_ton,                    
		pForwOut->unForw_forwarding_first_mcc,                           
		pForwOut->unForw_forwarding_first_mnc,                           
		pForwOut->unForw_forwarding_last_mcc,                            
		pForwOut->unForw_forwarding_last_mnc,                            
		pForwOut->unForw_forwarded_to_last_mcc,                          
		pForwOut->unForw_forwarded_to_last_mnc,                          
		pForwOut->szForw_camel_service_data,                                  
		pForwOut->unForw_inside_user_plane_index,                        
		pForwOut->unForw_inside_control_plane_index,                     
		pForwOut->unForw_in_bnc_connection_type,                         
		pForwOut->unForw_outside_user_plane_index,                       
		pForwOut->unForw_outside_control_plane_index,                    
		pForwOut->unForw_out_bnc_connection_type,                        
		pForwOut->unForw_radio_network_type                      
	);

Exit:
	return nRe;

}
/************************************************************************/
/* 生成Roamcsv函数 */
/************************************************************************/
static int GenerateRoamCsvFile(char* lpSavePath, char* lpFileString, char* lpSrcFileName, SRoamOut* pRoamOut, FILE * *pFile, char* lpDevice)
{
	int nRe = 0;
	char szOutFileName[256];
	char szTemp[256];
	char szFileName[256];
	char* lpOutTemp1;
	char* lpOutTemp2;
	int nI = 0;
	
	if(NULL == *pFile)
	{
		//  .../../000002_nsn_nbogs1_xxxxxx.dat
		lpOutTemp1 = szTemp;
		strcpy(lpOutTemp1, lpSrcFileName);
		//拼凑文件名
		while( NULL != ( lpOutTemp2 = strstr(lpOutTemp1, "/") ) )
		{
			lpOutTemp2 ++ ;
			lpOutTemp1 = lpOutTemp2;		
		}
		//截掉'.'后面的
		if ( NULL != ( lpOutTemp2 = strstr(lpOutTemp1, ".") ) )
		{
			lpOutTemp2[0] = '\0';
		}
		//lpOutTemp1:000002_nsn_nbogs1_xxxxxx
		lpOutTemp2 = lpOutTemp1;
		for ( ; nI < 3; nI ++)
		{
			if (NULL == ( lpOutTemp2 = strstr(lpOutTemp2, "_") ))
			{
				//文件名有问题
				nRe = 1 ; 
				goto Exit;				
			}
			lpOutTemp2 ++;
		}
		//lpOutTemp2:_xxxxxx
		lpOutTemp2 --;
		lpOutTemp2[0] = '\0';
		//lpOutTemp1:000002_nsn_nbogs1
		lpOutTemp2 ++;
		//lpOutTemp2:xxxxxx
		//拼凑
		sprintf(szFileName, 
			"%s_roam_%s.csv",
			lpOutTemp1,  lpOutTemp2);
		sprintf(szOutFileName, 
			"%s/%s",
			lpSavePath, szFileName);
		
		//打开或建立文件
		if ( NULL == ( *pFile = fopen( szOutFileName, "a" ) ) )
		{ 
			//文件打开失败
			nRe = 1 ; 
			goto Exit;
		} 	
		strcat(lpFileString, szFileName);
		strcat(lpFileString, ";");
	}


	//写文件
	fprintf
	(
		*pFile, 
		"%s,%d,%d,%s,%s,%d,%s,%s,%s,%d,%s,%s,%s,%s,%d,%s,%s,%s,\
		%s,%s,%d,%d,%s,%s,%s,%s,%d,%d,%s,%d,%s,%s,%d,%s,%d,\
		%d,%d,%d,%d,%d,%s,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d\n",
		lpDevice,
		pRoamOut->unRoam_record_length,               
		pRoamOut->eRoam_record_type,                        
		pRoamOut->szRoam_record_number,                         
		pRoamOut->szRoam_record_status,                        
		pRoamOut->unRoam_check_sum,                        
		pRoamOut->szRoam_call_reference,                       
		pRoamOut->szRoam_exchange_id,                          
		pRoamOut->szRoam_intermediate_record_number,            
		pRoamOut->unRoam_intermediate_charging_ind,        
		pRoamOut->szRoam_number_of_ss_records,                  
		pRoamOut->szRoam_calling_number,                       
		pRoamOut->szRoam_called_imsi,                          
		pRoamOut->szRoam_called_number,                        
		pRoamOut->unRoam_called_category,                  
		pRoamOut->szRoam_called_msrn,                          
		pRoamOut->szRoam_in_circuit_group,                      
		pRoamOut->szRoam_in_circuit,                            
		pRoamOut->szRoam_out_circuit_group,                     
		pRoamOut->szRoam_out_circuit,                           
		pRoamOut->unRoam_basic_service_type,               
		pRoamOut->unRoam_basic_service_code,               
		pRoamOut->szRoam_set_up_start_time,                    
		pRoamOut->szRoam_in_channel_allocated_time,            
		pRoamOut->szRoam_charging_start_time,                  
		pRoamOut->szRoam_charging_end_time,                    
		pRoamOut->unRoam_roam_mcz_duration,                    
		pRoamOut->unRoam_cause_for_termination,            
		pRoamOut->szRoam_data_volume,                           
		pRoamOut->unRoam_call_type,                        
		pRoamOut->szRoam_roam_mcz_tariff_class,                 
		pRoamOut->szRoam_roam_mcz_pulses,                       
		pRoamOut->unRoam_called_msrn_ton,                  
		pRoamOut->szRoam_facility_usage,                        
		pRoamOut->unRoam_roam_mcz_chrg_type,               
		pRoamOut->unRoam_calling_number_ton,               
		pRoamOut->unRoam_called_number_ton,                
		pRoamOut->unRoam_routing_category,                 
		pRoamOut->unRoam_cf_information,                   
		pRoamOut->unRoam_intermediate_chrg_cause,          
		pRoamOut->szRoam_camel_call_reference,                 
		pRoamOut->unRoam_camel_exchange_id_ton,            
		pRoamOut->szRoam_camel_exchange_id, 
		pRoamOut->unRoam_rate_adaption,               		
		pRoamOut->unRoam_selected_codec,             
		pRoamOut->unRoam_inside_user_plane_index,     
		pRoamOut->unRoam_inside_control_plane_index,  
		pRoamOut->unRoam_in_bnc_connection_type,      
		pRoamOut->unRoam_outside_user_plane_index,    
		pRoamOut->unRoam_outside_control_plane_index, 
		pRoamOut->unRoam_out_bnc_connection_type    
		);

Exit:
	return nRe;

}
/************************************************************************/
/* 转化函数 */
/************************************************************************/
//char2 To int
static int CharToUInt(unsigned int * npOut, unsigned char* lpIn, int nLen )
{
	int nRe = 0;
	char temp[4];
	char temp2[4];

	if (NULL == lpIn || NULL == npOut)
	{
		nRe = -1;
		goto Exit;
	}

	//大端算法
	memset( temp, 0, 4);
	memset( temp2, 0, 4);
	memcpy( temp , lpIn , nLen);
	temp2[0] = temp[3];
	temp2[1] = temp[2];
	temp2[2] = temp[1];
	temp2[3] = temp[0];
	memcpy( npOut , temp2 , sizeof(temp2));

/* 出口 */
Exit:
	return nRe;
}

//CharToStringForward
static int CharToStringForward(char * lpOut, unsigned char * lpIn, int nLen )
{
	int nRe = 0;
	int i = 0;

	if (NULL == lpOut || NULL == lpIn)
	{
		nRe = -1;
		goto Exit;
	}

	for(; i < nLen ; i++)
	{
		
		sprintf(lpOut, "%02X", *lpIn);
		lpOut += 2;
		lpIn ++;
	}			

/* 出口 */
Exit:
	return nRe;
}

//Char To DateTime
static int CharToDateTime(char * lpOut, unsigned char * lpIn)
{
	int nRe = 0;

	if (NULL == lpOut || NULL == lpIn)
	{
		nRe = -1;
		goto Exit;
	}
 	sprintf(lpOut, "%02X ", *(lpIn + 6) );
	sprintf( (lpOut + 2), "%02X ", *(lpIn + 5) );
	lpOut[4] = '-';
	sprintf((lpOut + 5), "%02X ", *(lpIn + 4) );
	lpOut[7] = '-';
	sprintf((lpOut + 8), "%02X ", *(lpIn + 3) );
	lpOut[10] = ' ';
	sprintf((lpOut + 11), "%02X ", *(lpIn + 2) );
	lpOut[13] = ':';
	sprintf((lpOut + 14), "%02X ", *(lpIn + 1) );
	lpOut[16] = ':';
	sprintf((lpOut + 17), "%02X ", *lpIn );
	lpOut[19] = '\0';

/* 出口 */
Exit:
	return nRe;
}

//CharToCallNumber
static int CharToCallNumber(char * lpOut, unsigned char * lpIn, int nLen)
{
	int nRe = 0;
	char szTemp[2];
	int i = 0;
	int j = 0;
	
	if (NULL == lpOut || NULL == lpIn)
	{
		nRe = -1;
		goto Exit;
	}
	
	for (; i < nLen; i++)
	{	
		sprintf(szTemp, "%02X ", *(lpIn) );
		lpOut[0] = szTemp[1];
		lpOut[1] = szTemp[0];
		for (; j < 2; j++)
		{
			switch(lpOut[j])
			{
			case 'B':
				lpOut[j] = '*';
				break;
			case 'C':
				lpOut[j] = '#';
				break;
			case 'D':
				lpOut[j] = 'B';
				break;
			case 'E':			
				lpOut[j] = 'C';
				break;
			case 'F':
				lpOut[j] = '\0';
				break;
			default:
				break;
			}				
		}
		lpOut += 2;
		lpIn ++;	
		j = 0;
	}
	lpOut[0] = '\0';
	
	/* 出口 */
Exit:
	return nRe;
}

//CharToStringForward
static int CharToStringReverse(char * lpOut, unsigned char * lpIn, int nLen)
{
	int nRe = 0;
	char szTemp[2];
	int i = 0;
	int j = 0;

	if (NULL == lpOut || NULL == lpIn)
	{
		nRe = -1;
		goto Exit;
	}
	
	for (; i < nLen; i++)
	{	
		sprintf(szTemp, "%02X ", *(lpIn) );
		lpOut[0] = szTemp[1];
		lpOut[1] = szTemp[0];
		for (; j < 2; j++)
		{
			switch(lpOut[j])
			{
  		case 'F':
				lpOut[j] = '\0';
				break;
			default:
				break;
			}				
		}
		lpOut += 2;
		lpIn ++;	
		j = 0;
	}
	lpOut[0] = '\0';

	/* 出口 */
Exit:
	return nRe;
}

//CharToBcdHalfWord
/************************************************************************/
// 00 05
// 1 BCD word
// 0500
/************************************************************************/
static int CharToBcdHalfWord(char * lpOut, unsigned char * lpIn, int nLen)
{
	int nRe = 0;
	int i = 0;
	
	if (NULL == lpOut || NULL == lpIn)
	{
		nRe = -1;
		goto Exit;
	}
	
	lpIn += (nLen - 1);
	
	for (; i < nLen; i++)
	{
		sprintf(lpOut, "%02X", *(lpIn) );
		lpIn --;	
		lpOut += 2;
	}
	
	/* 出口 */
Exit:
	return nRe;
}

