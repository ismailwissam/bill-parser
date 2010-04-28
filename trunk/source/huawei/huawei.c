/************************************************************************/
/*                                                                      */
/* huawei.c                                                             */
/*                                                                      */ 
/*     huawie C9000L gsm gateway话单预处理程序 v1.0 2005.09 haojianting */
/************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>

/*-----------------------------------------------------------------------------------*/
#define      ERR_LOG_FILE        "./log/pretreat.huawei.log"

//------------------------------------------------------------------------------------------------
typedef unsigned int   u_32;
typedef unsigned short u_16;
typedef unsigned char  u_8;

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


//------------------------------------------------------------------------------------------------
static void err_log(char * format,...);
static void p_32_swap(u_32 *p_a);
static void p_16_swap(u_16 *p_a);
static void p_8_swap(u_8 *p_a);
static int  hextoint(u_8 * hex,int hex_num);
static void hextostr(u_8 * out_str,u_8 * hex,int hex_num);
static void type_hextostr(u_8 * out_str,u_8 * hex,int hex_num);
static void bcdhextostr(u_8 * out_str,u_8 * hex,int hex_num);
static void type_bcdhextostr(u_8 * out_str,u_8 * hex,int hex_num);
static int  diff_time(int yy1,int mm1,int dd1,int yy2,int mm2,int dd2);

int         huawei(char * in_file_name, char * out_file_name, int * rec_num);

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

static int diff_time(int yy1,int mm1,int dd1,int yy2,int mm2,int dd2)
{
	int        ret;
	int        j;
	int        day1,day2;
	static int mon[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	
	ret=0;
	
	if( mm1<=12&&mm1>=1&&mm2<=12&&mm2>=1&&dd1<=31&&dd1>=1&&dd2<=31&&dd2>=1&& \
            yy1>=0&&yy2>=0 && ( (yy1-yy2)*12+mm1-mm2 )*31+dd1-dd2>=0 )
        {
                day1=0;
                day2=0;
                			
                j=1;
                while(j<mm2)
                {
                	day2+=mon[j-1];
                	if(j==2)
                	{
                		if( yy2%4==0&&(yy2%100!=0||yy2%400==0) )
                			day2+=1;
                	}
                	j++;
                }
                day2+=dd2;
                			
                while(yy1>yy2)
                {
                	if( yy2%4==0&&(yy2%100!=0||yy2%400==0) ) 
                		day1+=366;
                	else
                		day1+=365;
                				
                	yy2++;
                }
                j=1;
                while(j<mm1)
                {
                	day1+=mon[j-1];
                	if(j==2)
                	{
                		if( yy1%4==0&&(yy1%100!=0||yy1%400==0) )
                			day1+=1;
                	}
                	j++;
                }
                day1+=dd1;
                ret=day1-day2;
        }
        return ret;
}


/*
 *huawei()
 *          return 0 success, 1 fail
 *
 */
int huawei(char * in_file_name, char * out_file_name, int * rec_num)
{
	int                    ret;

	FILE                  *fp_i=NULL;
	FILE                  *fp_o=NULL;

	t_cdr                  cdr;
	char                   file_id[16];
	int                    serial_num;
	char                   add_cdr_switch[11];
	
	u_8                    tmp_buf[64];
	u_8                   *tmp_p=NULL;
	u_8                   *tmp_p_2=NULL;
	int                    yy1,mm1,dd1,yy2,mm2,dd2,hh2,mi2,ss2,layout_mi;

	u_8                   *ptr=NULL;
	int                    i,cdr_len,cdr_num_once,r_cdr_num;

	/*每次读100条话单, 每条252字节定长*/
	u_8                    buff[100][252];     

	
	ret          = 0;
	serial_num   = 0;
	(*rec_num)   = 0;

	cdr_num_once = 100;
	cdr_len      = 252;
	

	fp_i=fopen(in_file_name,"r");
	if(fp_i==NULL)
	{
        	err_log("huawei: fopen %s fail\n",in_file_name);
        	ret=1;
        	goto Exit_Pro;
	}
	
	fp_o=fopen(out_file_name,"w+");
	if(fp_o==NULL)
	{
        	err_log("huawei: fopen %s fail\n",out_file_name);
        	ret=1;
        	goto Exit_Pro;
	}

	/* get file_id , add_cdr_switch */
	memset(file_id,0,sizeof(file_id));
	memset(add_cdr_switch,0,sizeof(add_cdr_switch));
	
	//file_id
	tmp_p=strrchr(in_file_name,'/');
	if(tmp_p==NULL)
	{
		tmp_p=in_file_name;
	}
	else
	{
		tmp_p++;
	}
	if(strlen(tmp_p)<16)
	{
		err_log("huawei: get file_id fail");
		ret=1;
		goto Exit_Pro;
	}

	tmp_p+=7;
	strncpy(file_id,tmp_p,4);
	tmp_p+=4;
	strncat(file_id,tmp_p,2);
	tmp_p+=2;
	strncat(file_id,tmp_p,2);
	strcat(file_id,"_");
	
	tmp_p_2=strrchr(tmp_p,'.');
	if(tmp_p_2==NULL)
	{
		err_log("huawei: get file_id fail");
		ret=1;
		goto Exit_Pro;
	}
	else
	{
		strncat(file_id,tmp_p_2-4,4);
	}
	
	tmp_p   = NULL;
	tmp_p_2 = NULL;

	//add_cdr_switch
	tmp_p=strrchr(in_file_name,'/');
	if(tmp_p==NULL)
	{
		strncpy(add_cdr_switch,in_file_name,6);
	}
	else
	{
		strncpy(add_cdr_switch,tmp_p+1,6);
		tmp_p=NULL;
	}
	
	//yy1,mm1,dd1
	yy1=0;
	mm1=0;
	dd1=0;
	if(strlen(file_id)>=8)
	{
        	sscanf(file_id,"%4d%2d%2d",&yy1,&mm1,&dd1);
	}  
	

	while(1)
	{
		memset(buff,0,sizeof(buff));
		
		r_cdr_num=fread(buff,cdr_len,cdr_num_once,fp_i);
		
		if(r_cdr_num>0)
		{
			for(i=0;i<r_cdr_num;i++)
			{
				serial_num++;
				memset(&cdr,0,sizeof(cdr));

//--------------------------------------------------------------------------------------------------------
				/* 提取话单记录 */
		        	ptr=(u_8 *)&buff[i][0];

				//cdr.file_id
				strcpy(cdr.file_id,file_id);

				//cdr.serial_num
				cdr.serial_num=serial_num;

				//cdr.add_cdr_switch
				strcpy(cdr.add_cdr_switch,add_cdr_switch);

		        	//cdr.call_type
				ptr+=9;
				cdr.call_type=*ptr;

				//cdr.charging_date
				//cdr.answer_time
				//cdr.add_call_begin_time
				ptr++;
				yy2=0;
				mm2=0;
				dd2=0;
				hh2=0;
				mi2=0;
				ss2=0;
				
				yy2=*ptr+*(ptr+1)*0x100;
				ptr+=2;
				mm2=*ptr++;
				dd2=*ptr++;
				hh2=*ptr++;
				mi2=*ptr++;
				ss2=*ptr++;
				
				sprintf(cdr.charging_date,"%04d%02d%02d",yy2,mm2,dd2);
				sprintf(cdr.answer_time,"%02d:%02d:%02d",hh2,mi2,ss2);
				sprintf(cdr.add_call_begin_time,"%s%02d%02d%02d",cdr.charging_date,hh2,mi2,ss2);
				
				//cdr.served_dn               本端号码,临时放主叫号码
				type_hextostr(cdr.served_dn,ptr,14);
				if( (tmp_p=strchr(cdr.served_dn,'F'))!=NULL )
				{
					*tmp_p='\0';
					tmp_p=NULL;
				}
				
				//cdr.dialled_other_party     对端号码,临时放被叫号码
				ptr+=14;
				type_hextostr(cdr.dialled_other_party,ptr,14);
				if( (tmp_p=strchr(cdr.dialled_other_party,'F'))!=NULL )
				{
					*tmp_p='\0';
					tmp_p=NULL;
				}
				
				//cdr.third_party
				ptr+=14;
				type_hextostr(cdr.third_party,ptr,14);
				if( (tmp_p=strchr(cdr.third_party,'F'))!=NULL )
				{
					*tmp_p='\0';
					tmp_p=NULL;
				}
				
				//cdr.add_my_msrn
				//cdr.add_other_msrn
				ptr+=14;
				hextostr(cdr.add_my_msrn,ptr,8);
				if( (tmp_p=strchr(cdr.add_my_msrn,'F'))!=NULL )
				{
					*tmp_p='\0';
					tmp_p=NULL;
				}
				strcpy(cdr.add_other_msrn,cdr.add_my_msrn);
				
				//cdr.translated_other_party  对端号码[处理], 临时放实际的本端号码
				ptr+=8;
				//hextostr(cdr.translated_other_party,ptr,8);
				
				//cdr.add_my_imsi
				ptr+=8;
				hextostr(cdr.add_my_imsi,ptr,8);
				if( (tmp_p=strchr(cdr.add_my_imsi,'F'))!=NULL )
				{
					*tmp_p='\0';
					tmp_p=NULL;
				}
				
				//cdr.add_my_imei
				ptr+=8;
				hextostr(cdr.add_my_imei,ptr,8);
				if( (tmp_p=strchr(cdr.add_my_imei,'F'))!=NULL )
				{
					*tmp_p='\0';
					tmp_p=NULL;
				}
				
				//cdr.add_in_trunk_group_id
				ptr+=8;
				if( !(*ptr==0xff && *(ptr+1)==0xff) )
				{
					sprintf(cdr.add_in_trunk_group_id,"%d",*ptr+*(ptr+1)*0x100);
				}
				
				//cdr.add_out_trunk_group_id
				ptr+=2;
				if( !(*ptr==0xff && *(ptr+1)==0xff) )
				{
					sprintf(cdr.add_out_trunk_group_id,"%d",*ptr+*(ptr+1)*0x100);
				}
				
				//cdr.add_talk_duration
				ptr+=2;
				memcpy(&cdr.add_talk_duration,ptr,4);
				p_32_swap(&cdr.add_talk_duration);
	        	
	        		//cdr.billing_id
	        		ptr+=4;
	        		hextostr(cdr.billing_id,ptr,4);
	        		
	        		//cdr.record_type
	        		ptr+=5;
	        		cdr.record_type=(*ptr)&0x7;
	        		
	        		//cdr.reason_for_termination
	        		ptr++;
	        		hextostr(cdr.reason_for_termination,ptr,1);
	        		
	        		//cdr.exchange_id
	        		ptr+=2;
	        		hextostr(cdr.exchange_id,ptr,8);
				if( (tmp_p=strchr(cdr.exchange_id,'F'))!=NULL )
				{
					*tmp_p='\0';
					tmp_p=NULL;
				}
	        		
	        		//cdr.ss_code
	        		ptr+=37;
	        		hextostr(cdr.ss_code,ptr,1);
	        		
	        		//cdr.service_centre_address
	        		ptr+=23;
	        		hextostr(cdr.service_centre_address,ptr,8);
				if( (tmp_p=strchr(cdr.service_centre_address,'F'))!=NULL )
				{
					*tmp_p='\0';
					tmp_p=NULL;
				}

	        		//------------------------------------------------------------------------
				//调整
				//cdr.served_dn,  cdr.dialled_other_party
				//cdr.add_my_msrn,  cdr.add_other_msrn
	        		//
	        		
	        		if(cdr.call_type==1||cdr.call_type==2)
	        		{
	        			strcpy(tmp_buf,cdr.served_dn);
	        			strcpy(cdr.served_dn,cdr.dialled_other_party);
	        			strcpy(cdr.dialled_other_party,tmp_buf);
	        			
	        			memset(cdr.add_other_msrn,0,sizeof(cdr.add_other_msrn));
	        		}
	        		else
	        		{
	        			memset(cdr.add_my_msrn,0,sizeof(cdr.add_my_msrn));
	        		}
	        		
	        		//MOC, MTC, ROAM, SMS MO, SMS MT 
	        		//cdr.served_dn 8613xxx->13xxx
	        		if( cdr.call_type==0||cdr.call_type==1||cdr.call_type==2||
	        		    cdr.call_type==7||cdr.call_type==6  )
	        		{
	        			if(strncmp(cdr.served_dn,"8613",4)==0)
	        			{
		        			strcpy(tmp_buf,cdr.served_dn);
	        				strcpy(cdr.served_dn,&tmp_buf[2]);
	        			}
	        		}
	        		
	        		//呼转话单, cdr.dialled_other_party 86xxx->0086xxx
	        		if( cdr.call_type==18 )
	        		{
	        			if(strncmp(cdr.dialled_other_party,"86",2)==0)
	        			{
	        				strcpy(tmp_buf,"00");
	        				strcat(tmp_buf,cdr.dialled_other_party);
	        				strcpy(cdr.dialled_other_party,tmp_buf);
	        			}
	        		}
	        		
	        		//------------------------------------------------------------------------
	        		//赋值
				
				//cdr.add_record_type
				if(cdr.record_type==0)
				{
					strcpy(cdr.add_record_type,"00");
				}
				if(cdr.record_type==1)
				{
					strcpy(cdr.add_record_type,"01");
				}
				if(cdr.record_type==2)
				{
					strcpy(cdr.add_record_type,"02");
				}
				if(cdr.record_type==3)
				{
					strcpy(cdr.add_record_type,"03");
				}

				//cdr.add_call_type
				strcpy(cdr.add_call_type,"S8");
				if(cdr.call_type==0)
				{
					strcpy(cdr.add_call_type,"S1");
				}
				if(cdr.call_type==1||cdr.call_type==18)
				{
					strcpy(cdr.add_call_type,"S2");
				}
				if(cdr.call_type==7)
				{
					strcpy(cdr.add_call_type,"S3");
				}
				if(cdr.call_type==6)
				{
					strcpy(cdr.add_call_type,"S4");
				}
				if(cdr.call_type==2)
				{
					strcpy(cdr.add_call_type,"S5");
				}
				if(cdr.call_type==4 || cdr.call_type==10)
				{
					strcpy(cdr.add_call_type,"S6");
				}

				//cdr.add_cf_type
				strcpy(cdr.add_cf_type,"00");
				if(strcmp(cdr.ss_code,"00")==0)
				{
					strcpy(cdr.add_cf_type,"21");
				}
				if(strcmp(cdr.ss_code,"01")==0)
				{
					strcpy(cdr.add_cf_type,"29");
				}
				if(strcmp(cdr.ss_code,"02")==0)
				{
					strcpy(cdr.add_cf_type,"2A");
				}
				if(strcmp(cdr.ss_code,"03")==0)
				{
					strcpy(cdr.add_cf_type,"2B");
				}
				
				//cdr.disconnect_reason ???
				
				//cdr.add_my_dn
				if(strncmp(cdr.served_dn,"86",2)==0 &&
				   strncmp(cdr.add_my_imsi,"460",3)==0 )
				{
					strcpy(cdr.add_my_dn,&cdr.served_dn[2]);
					strcpy(cdr.served_dn,cdr.add_my_dn);
				}
				else
				{
					strcpy(cdr.add_my_dn,cdr.served_dn);
				}   
				
				//cdr.add_other_dn
				if(strlen(cdr.dialled_other_party)>0)
				{
					strcpy(cdr.add_other_dn,cdr.dialled_other_party);
				}
				else
				{
					strcpy(cdr.add_other_dn,cdr.translated_other_party);
				}
				
				//cdr.add_third_party
				strcpy(cdr.add_third_party,cdr.third_party);
				
				//cdr.add_occupy_duration
				cdr.add_occupy_duration=cdr.add_talk_duration;
				
				//cdr.add_layout_time
				//cdr.add_time_diff
				if(strlen(cdr.add_call_begin_time)>=12)
				{
					yy2=0;
					mm2=0;
					dd2=0;
					hh2=0;
					mi2=0;
					layout_mi=0;
				    if(sscanf(cdr.add_call_begin_time,"%4d%2d%2d%2d%2d",&yy2,&mm2,&dd2,&hh2,&mi2)==5)
				    {
					if(mi2>=0)   layout_mi=0;
					if(mi2>=15)  layout_mi=15;
					if(mi2>=30)  layout_mi=30;
					if(mi2>=45)  layout_mi=45;
				
					sprintf(cdr.add_layout_time,"%04d%02d%02d%02d%02d",yy2,mm2,dd2,hh2,layout_mi);
					cdr.add_time_diff=diff_time(yy1,mm1,dd1,yy2,mm2,dd2);
				    }
				}		

				//cdr.add_gc
				if( cdr.call_type==0 ||cdr.call_type==1 ||cdr.call_type==2 ||
				    cdr.call_type==6 ||cdr.call_type==7 ||cdr.call_type==18  )
				{
					if( strncmp(cdr.add_my_dn,"133",3)==0 )
					{
						strcpy(cdr.add_gc,"1");
					}
				}
	        		
	        		
//--------------------------------------------------------------------------------------------------------
				/* 输出话单记录到文件 */
	                        fprintf(fp_o,"%s,",cdr.file_id);
	                        fprintf(fp_o,"%d,",cdr.serial_num);
                                
	                        fprintf(fp_o,"%s,",cdr.add_cdr_switch);
                                
	                        fprintf(fp_o,"%s,",cdr.add_hlr);
                                
	                        fprintf(fp_o,"%s,",cdr.add_my_area_code);
	                        fprintf(fp_o,"%s,",cdr.add_my_area_name);
                                
	                        fprintf(fp_o,"%s,",cdr.add_my_scp);
	                        fprintf(fp_o,"%s,",cdr.add_product_package);
	                        fprintf(fp_o,"%s,",cdr.add_a_flag);
	                        fprintf(fp_o,"%s,",cdr.add_a_name);
	                        fprintf(fp_o,"%s,",cdr.add_u_type);
	                        fprintf(fp_o,"%s,",cdr.add_u_group);
	                        fprintf(fp_o,"%s,",cdr.add_gc);
                                
	                        fprintf(fp_o,"%d,",cdr.add_time_diff);
                                
	                        fprintf(fp_o,"%s,",cdr.add_layout_time);
	                        fprintf(fp_o,"%s,",cdr.add_call_begin_time);
	                        fprintf(fp_o,"%d,",cdr.add_occupy_duration);
	                        fprintf(fp_o,"%d,",cdr.add_talk_duration);
                                
	                        fprintf(fp_o,"%s,",cdr.add_record_type);
                                
	                        fprintf(fp_o,"%s,",cdr.add_call_type);
                                
	                        fprintf(fp_o,"%s,",cdr.add_cf_type);
                                
	                        fprintf(fp_o,"%s,",cdr.add_my_imsi);
	                        fprintf(fp_o,"%s,",cdr.add_my_imei);
                                
	                        fprintf(fp_o,"%s,",cdr.add_my_dn);
	                        fprintf(fp_o,"%s,",cdr.add_other_dn);
	                        fprintf(fp_o,"%s,",cdr.add_third_party);
                                
	                        fprintf(fp_o,"%s,",cdr.add_my_msrn);
	                        fprintf(fp_o,"%s,",cdr.add_other_msrn);
                                
	                        fprintf(fp_o,"%s,",cdr.add_other_a_flag);
	                        fprintf(fp_o,"%s,",cdr.add_other_a_name);
                                
	                        fprintf(fp_o,"%s,",cdr.add_sub_my_dn);
	                        fprintf(fp_o,"%s,",cdr.add_sub_other_dn);
                                
	                        fprintf(fp_o,"%s,",cdr.add_other_direction);
                                
	                        fprintf(fp_o,"%d,",cdr.add_my_cell_num);
	                        fprintf(fp_o,"%d,",cdr.add_other_cell_num);
                                
	                        fprintf(fp_o,"%s,",cdr.add_in_trunk_group_id);
	                        fprintf(fp_o,"%d,",cdr.add_in_cic_pcm);
	                        fprintf(fp_o,"%d,",cdr.add_in_cic_ts);
                                
	                        fprintf(fp_o,"%s,",cdr.add_out_trunk_group_id);
	                        fprintf(fp_o,"%d,",cdr.add_out_cic_pcm);
	                        fprintf(fp_o,"%d,",cdr.add_out_cic_ts);
                                
	                        fprintf(fp_o,"%s,",cdr.add_phone_manufacturer);
	                        fprintf(fp_o,"%s,",cdr.add_phone_type);
                                
	                        fprintf(fp_o,"%s,",cdr.charging_date);
	                        fprintf(fp_o,"%s,",cdr.answer_time);
	                        fprintf(fp_o,"%s,",cdr.disconnect_reason);
	                        fprintf(fp_o,"%s,",cdr.billing_id);
                                
	                        fprintf(fp_o,"%d,",cdr.record_type);
	                        fprintf(fp_o,"%d,",cdr.call_type);
	                        fprintf(fp_o,"%s,",cdr.reason_for_termination);
	                        fprintf(fp_o,"%s,",cdr.served_dn);
	                        fprintf(fp_o,"%s,",cdr.dialled_other_party);
	                        fprintf(fp_o,"%s,",cdr.translated_other_party);
	                        fprintf(fp_o,"%s,",cdr.third_party);
	                        fprintf(fp_o,"%s,",cdr.service_centre_address);
	                        fprintf(fp_o,"%d,",cdr.call_hold_count);
	                        fprintf(fp_o,"%d,",cdr.call_wait_count);
	                        fprintf(fp_o,"%s,",cdr.ss_code);
	                        fprintf(fp_o,"%s\n" ,cdr.exchange_id);
			}
		}

		if(r_cdr_num<cdr_num_once)
		{
			if( ferror(fp_i) )
			{
				err_log("huawei: fread %s fail\n",in_file_name);
				ret=1;
				goto Exit_Pro;
			}
			break;
		}
	}

	fclose(fp_i); 
	fp_i=NULL;
	fclose(fp_o); 
	fp_o=NULL;
	
Exit_Pro:
	if(fp_i!=NULL) fclose(fp_i);
	if(fp_o!=NULL) fclose(fp_o);

	(*rec_num) = serial_num;

	return ret;
}

