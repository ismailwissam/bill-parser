/*****************************************************************/
/*                                                               */
/* siemens.c                                                     */
/*                                                               */ 
/*       siemens gsm SR9 话单预处理程序 v1.0 2005.07 haojianting */
/*****************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>

#include <asn1parser.h>

#include <asn_application.h>
#include <constraints.h>
#include <ber_tlv_tag.h>
#include <ber_tlv_length.h>
#include <asn_codecs_prim.h>
#include <OBJECT_IDENTIFIER.c>
#include <RELATIVE-OID.c>
#include <Sr9Siemens.h>
#include "sys-common.h"

/*-----------------------------------------------------------------------------------*/
#define STDERR_OUT   stdout

#define BUFFSIZE     2048

typedef enum pd_code {
	PD_FAILED	= -1,
	PD_FINISHED	= 0,
	PD_EOF		= 1,
} pd_code_e;

typedef struct content_buff {
	int             length;
        int             position;
	unsigned char   buff[BUFFSIZE];
} cont_buff;

/*-----------------------------------------------------------------------------------*/
static int       my_debug = 1;                  /* 0 not debug */

static int       single_type_decoding = 0;	/* -1 enables that */
static int       pretty_printing = 1;		/* -p disables that */
static char    * indent_buffer = "    ";	/* -i controls that */

static int       decode_tlv_from_string(const char *datastring);

static int       cont_buff_get(cont_buff * pcont_buff);
static pd_code_e process_deeper(const char *fname, cont_buff* pcont_buff, int level, ssize_t limit, ssize_t *decoded, int expect_eoc);
static void      print_TL(int fin, int level, int constr, ssize_t tlen, ber_tlv_tag_t, ber_tlv_len_t);
static int       print_V(const char *fname, cont_buff * pcont_buff, ber_tlv_tag_t tlv_tag, ber_tlv_len_t tlv_len);

//------------------------------------------------------------------------------------------------
#define      ERR_LOG_FILE        "./log/pretreat.siemens.log"

//------------------------------------------------------------------------------------------------
typedef unsigned int   u_32;
typedef unsigned short u_16;
typedef unsigned char  u_8;

typedef struct {
        char  add_cdr_switch[11];	//话单数据来源, pretreat为采集点编号, mark改为交换机名称MSC1, GMSC1, TMSC1等

        int   recordType;               //记录类型 4, Single Billing Record
                                        //         5, First Intermediate Billing Record
                                        //         6, Intermediate Billing Record
                                        //         7, Last Billing Record
        int   callTransactionType;      //呼叫类型
        char  servedIMSI[17];           //IMSI, TBCD-String 
        char  servedMSIsdn[17];         //用户号码, TBCD-String
        char  servedIMEI[17];           //IMEI
        char  servedMSRN[17];           //用户动态漫游号, TBCD-String
        char  startOfChargingdate[9];   //话单开始日期yyyymmdd
        char  answer_time[9];           //应答开始时间hh:mi:ss
        int   callDuration;             //通话时长秒
        char  dialledOtherParty[35];    //另一端的号码(dialled), 去掉第一字节TBCD-String
        char  translatedOtherParty[35]; //另一端的号码(translated), 去掉第一字节TBCD-String
        char  otherMSRN[17];            //另一端的动态漫游号, TBCD-String
        char  otherParty[35];           //如果translatedOtherParty以0086开头取该字段,否则取dialledOtherParty
        char  thirdParty[35];           //第三方号码, 去掉第一字节TBCD-String
        char  cellId[15];               //cellId
        char  exchangeId[12];           //exchangeId
} t_cdr;

//------------------------------------------------------------------------------------------------
static void err_log(char * format,...);
static void p_32_swap(u_32 *p_a);
static void p_16_swap(u_16 *p_a);
static void p_8_swap(u_8 *p_a);
static int  hextoint(u_8 * hex,int hex_num);
static void hextostr(u_8 * out_str,u_8 * hex,int hex_num);
static void bcdhextostr(u_8 * out_str,u_8 * hex,int hex_num);
int siemens(char * in_file_name, char * out_file_name, int * rec_num);

//------------------------------------------------------------------------------------------------
//--main for test
/*
int main(int argc,char * argv[])
{
	int rec_num;
	int r;
	
	r=siemens(argv[1],argv[2],&rec_num);
	
	printf("siemens return %d,rec_num=%d\n",r,rec_num);
	
	return 0;
}
*/
//------------------------------------------------------------------------------------------------

static void err_log(char * format,...)
{
	time_t t;
	struct tm *systime;
	FILE *fp;
	
	char   tmp[256];

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
 *bcdhextostr()
 */
static void bcdhextostr(u_8 * out_str,u_8 * hex,int hex_num)
{
	int i;
	for(i=0;i<hex_num;i++)
	{
		p_8_swap(hex);
		sprintf(out_str,"%02X",*hex);
		out_str+=2;
		hex++;
	}	
}

/*
 *siemens()
 *          return 0 success, 1 fail
 *
 */
int siemens(char * in_file_name, char * out_file_name, int * rec_num)
{
	int                    ret;

	FILE                  *fp_i=NULL;
	FILE                  *fp_o=NULL;
	t_cdr                  cdr;

	cont_buff              my_cont_buff;
	int                    once_read;

	pd_code_e              pdc;
	ssize_t                decoded;

	Sr9Siemens_t          *sr9Siemens = 0;
	
        int                    total;
	int                    begin;
	int                    yes_decode;
        asn_dec_rval_t         rval;
	asn_TYPE_descriptor_t *pduType = &asn_DEF_Sr9Siemens;
	
	u_8                    tmp_buf[64];
	u_8                   *tmp_p=NULL;

	
	ret        = 0;
	(*rec_num) = 0;

	fp_i=fopen(in_file_name,"r");
	if(fp_i==NULL)
	{
        	err_log("siemens: fopen %s fail\n",in_file_name);
        	ret=1;
        	goto Exit_Pro;
	}
	
	fp_o=fopen(out_file_name,"w+");
	if(fp_o==NULL)
	{
        	err_log("siemens: fopen %s fail\n",out_file_name);
        	ret=1;
        	goto Exit_Pro;
	}

	/*
	 * Fetch out BER-encoded data until EOF or error.
	 */
	once_read=sizeof(my_cont_buff.buff);
	
	total=0;
	 
	while(1)
	{
		memset(my_cont_buff.buff,0,sizeof(my_cont_buff.buff));
		
		my_cont_buff.length=fread(my_cont_buff.buff,1,once_read,fp_i);
		
		if( my_cont_buff.length > 0 )
		{
     			my_cont_buff.position=0;
     			decoded=0;
			
			do {
				memset(&cdr,0,sizeof(cdr));
		
				yes_decode=0;
				if(decoded<my_cont_buff.length)
				{
					begin=decoded;
					if(my_cont_buff.buff[decoded]==0xe1)
					{
						yes_decode=1;
					}	
				}
				pdc = process_deeper(in_file_name, &my_cont_buff, 0, -1, &decoded, 0);
				printf("## process_deeper r,begin,decodeed,total: %d,%d,%d,%d\n",pdc,begin,decoded,total);

				/* ber_decode */
				if(yes_decode && pdc != PD_FAILED)
				{
					if(decoded-begin>0)
					{
			  			rval=ber_decode(0,pduType,(void **)&sr9Siemens,&my_cont_buff.buff[begin],decoded-begin);
			  			if(rval.code==RC_OK)
			  			{
							(*rec_num)++;
							if(my_debug)
								asn_fprint(stdout,pduType,sr9Siemens);
			  			}
			  			else
			  			{
			  				err_log("siemens: ber_decode fail\n");
			  			}
					}  
					if(sr9Siemens!=0)
					{
						/* 提取话单记录 */
						//cdr.add_cdr_switch
						tmp_p=strrchr(in_file_name,'/');
						if(tmp_p==NULL)
							strncpy(cdr.add_cdr_switch,in_file_name,6);
						else
						{
							strncpy(cdr.add_cdr_switch,tmp_p+1,6);
							tmp_p=NULL;
						}
						//cdr.recordType
						if(sr9Siemens->recordType!=NULL)
						{
							if(sr9Siemens->recordType->buf!=NULL)
							{
								cdr.recordType=hextoint(sr9Siemens->recordType->buf,sr9Siemens->recordType->size);
							}	
						}
						//cdr.callTransactionType
						if(sr9Siemens->callTransactionType!=NULL)
						{
							if(sr9Siemens->callTransactionType->buf!=NULL)
							{
							        cdr.callTransactionType=hextoint(sr9Siemens->callTransactionType->buf,sr9Siemens->callTransactionType->size);
							}	
						}
						if(sr9Siemens->servedMobileNumber!=NULL)
						{
						  //cdr.servedIMSI
						  if(sr9Siemens->servedMobileNumber->servedIMSI!=NULL)
						  {
							if(sr9Siemens->servedMobileNumber->servedIMSI->buf!=NULL)
							{
								memcpy(tmp_buf,sr9Siemens->servedMobileNumber->servedIMSI->buf,sr9Siemens->servedMobileNumber->servedIMSI->size);
								bcdhextostr(cdr.servedIMSI,tmp_buf,sr9Siemens->servedMobileNumber->servedIMSI->size);
								if( (tmp_p=strchr(cdr.servedIMSI,'F'))!=NULL )
								{
									*tmp_p='\0';
									tmp_p=NULL;
								}
							}
						  }
						  //cdr.servedMSIsdn
						  if(sr9Siemens->servedMobileNumber->servedMSIsdn!=NULL)
						  {
							if(sr9Siemens->servedMobileNumber->servedMSIsdn->buf!=NULL)
							{
								memcpy(tmp_buf,sr9Siemens->servedMobileNumber->servedMSIsdn->buf,sr9Siemens->servedMobileNumber->servedMSIsdn->size);
								bcdhextostr(cdr.servedMSIsdn,tmp_buf,sr9Siemens->servedMobileNumber->servedMSIsdn->size);
								if( (tmp_p=strchr(cdr.servedMSIsdn,'F'))!=NULL )
								{
									*tmp_p='\0';
									tmp_p=NULL;
								}
							}
						  }
						  //cdr.servedIMEI
						  if(sr9Siemens->servedMobileNumber->servedIMEI!=NULL)
						  {
							if(sr9Siemens->servedMobileNumber->servedIMEI->buf!=NULL)
							{
								memcpy(tmp_buf,sr9Siemens->servedMobileNumber->servedIMEI->buf,sr9Siemens->servedMobileNumber->servedIMEI->size);
								hextostr(cdr.servedIMEI,tmp_buf,sr9Siemens->servedMobileNumber->servedIMEI->size);
							}
						  }
						  //cdr.servedMSRN
						  if(sr9Siemens->servedMobileNumber->servedMSRN!=NULL)
						  {
							if(sr9Siemens->servedMobileNumber->servedMSRN->buf!=NULL)
							{
								memcpy(tmp_buf,sr9Siemens->servedMobileNumber->servedMSRN->buf,sr9Siemens->servedMobileNumber->servedMSRN->size);
								bcdhextostr(cdr.servedMSRN,tmp_buf,sr9Siemens->servedMobileNumber->servedMSRN->size);
								if( (tmp_p=strchr(cdr.servedMSRN,'F'))!=NULL )
								{
									*tmp_p='\0';
									tmp_p=NULL;
								}
							}
						  }
						}

						if(sr9Siemens->chargingtimeData!=NULL)
						{
						  //cdr.startOfChargingdate
						  if(sr9Siemens->chargingtimeData->startOfChargingdate!=NULL)
						  {
						  	if(sr9Siemens->chargingtimeData->startOfChargingdate->buf!=NULL)
						  	{
						  		memset(tmp_buf,0,sizeof(tmp_buf));
								memcpy(tmp_buf,sr9Siemens->chargingtimeData->startOfChargingdate->buf,sr9Siemens->chargingtimeData->startOfChargingdate->size);
								sprintf(cdr.startOfChargingdate,"20%02X%02X%02X",tmp_buf[0],tmp_buf[1],tmp_buf[2]);
						  	}
						  }
						  //cdr.answer_time
						  if(sr9Siemens->chargingtimeData->startOfChargingtime!=NULL)
						  {
						    if(sr9Siemens->chargingtimeData->startOfChargingtime->timeStamp!=NULL)
						    {
						    	if(sr9Siemens->chargingtimeData->startOfChargingtime->timeStamp->buf!=NULL)
						    	{
						  		memset(tmp_buf,0,sizeof(tmp_buf));
						    		memcpy(tmp_buf,sr9Siemens->chargingtimeData->startOfChargingtime->timeStamp->buf,sr9Siemens->chargingtimeData->startOfChargingtime->timeStamp->size);
						    		sprintf(cdr.answer_time,"%02X:%02X:%02X",tmp_buf[0],tmp_buf[1],tmp_buf[2]);
						    	}
						    }
						  }
						  //cdr.callDuration
						  if(sr9Siemens->chargingtimeData->callDuration!=NULL)
						  {
						  	if(sr9Siemens->chargingtimeData->callDuration->buf!=NULL)
						  	{
						  		cdr.callDuration=hextoint(sr9Siemens->chargingtimeData->callDuration->buf,sr9Siemens->chargingtimeData->callDuration->size);
						  	}
						  }
						}
						if(sr9Siemens->otherPartySequence!=NULL)
						{
						  //cdr.dialledOtherParty
						  if(sr9Siemens->otherPartySequence->dialledOtherParty!=NULL)
						  {
						  	if(sr9Siemens->otherPartySequence->dialledOtherParty->buf!=NULL)
						  	{
						  		memcpy(tmp_buf,sr9Siemens->otherPartySequence->dialledOtherParty->buf,sr9Siemens->otherPartySequence->dialledOtherParty->size);
								bcdhextostr(cdr.dialledOtherParty,&tmp_buf[1],sr9Siemens->otherPartySequence->dialledOtherParty->size-1);
								if( (tmp_p=strchr(cdr.dialledOtherParty,'F'))!=NULL )
								{
									*tmp_p='\0';
									tmp_p=NULL;
								}
						  	}
						  }
						  //cdr.translatedOtherParty
						  if(sr9Siemens->otherPartySequence->translatedOtherParty!=NULL)
						  {
						  	if(sr9Siemens->otherPartySequence->translatedOtherParty->buf!=NULL)
						  	{
						  		memcpy(tmp_buf,sr9Siemens->otherPartySequence->translatedOtherParty->buf,sr9Siemens->otherPartySequence->translatedOtherParty->size);
								bcdhextostr(cdr.translatedOtherParty,&tmp_buf[1],sr9Siemens->otherPartySequence->translatedOtherParty->size-1);
								if( (tmp_p=strchr(cdr.translatedOtherParty,'F'))!=NULL )
								{
									*tmp_p='\0';
									tmp_p=NULL;
								}
						  	}
						  }
						}
						//cdr.otherMSRN
						if(sr9Siemens->otherMSRN!=NULL)
						{
							if(sr9Siemens->otherMSRN->buf!=NULL)
							{
								memcpy(tmp_buf,sr9Siemens->otherMSRN->buf,sr9Siemens->otherMSRN->size);
								bcdhextostr(cdr.otherMSRN,tmp_buf,sr9Siemens->otherMSRN->size);
								if( (tmp_p=strchr(cdr.otherMSRN,'F'))!=NULL )
								{
									*tmp_p='\0';
									tmp_p=NULL;
								}
							}
						}
						//cdr.otherParty
						if( strncmp("0086",cdr.translatedOtherParty,4)==0 )
						{
							strcpy(cdr.otherParty,cdr.translatedOtherParty);
						}
						else
						{
							strcpy(cdr.otherParty,cdr.dialledOtherParty);
						}
						//cdr.thirdParty
						if(sr9Siemens->thirdParty!=NULL)
						{
							if(sr9Siemens->thirdParty->buf!=NULL)
							{
								memcpy(tmp_buf,sr9Siemens->thirdParty->buf,sr9Siemens->thirdParty->size);
								bcdhextostr(cdr.thirdParty,&tmp_buf[1],sr9Siemens->thirdParty->size-1);
								if( (tmp_p=strchr(cdr.thirdParty,'F'))!=NULL )
								{
									*tmp_p='\0';
									tmp_p=NULL;
								}
								
							}
						}
						//cdr.cellId
						if(sr9Siemens->cellId!=NULL)
						{
							if(sr9Siemens->cellId->buf!=NULL)
							{
								memcpy(tmp_buf,sr9Siemens->cellId->buf,sr9Siemens->cellId->size);
								hextostr(cdr.cellId,tmp_buf,sr9Siemens->cellId->size);
							}
						}
						//cdr.exchangeId
						if(sr9Siemens->exchangeId!=NULL)
						{
							if(sr9Siemens->exchangeId->buf!=NULL)
							{
								memcpy(cdr.exchangeId,sr9Siemens->exchangeId->buf,sr9Siemens->exchangeId->size);
							}
						}
						
						/* 输出话单记录到文件 */
						fprintf(fp_o,"%s,", cdr.add_cdr_switch);
						fprintf(fp_o,"%d,", cdr.recordType);
						fprintf(fp_o,"%d,", cdr.callTransactionType);
						fprintf(fp_o,"%s,", cdr.servedIMSI);
						fprintf(fp_o,"%s,", cdr.servedMSIsdn);
						fprintf(fp_o,"%s,", cdr.servedIMEI);
						fprintf(fp_o,"%s,", cdr.servedMSRN);
						fprintf(fp_o,"%s,", cdr.startOfChargingdate);
						fprintf(fp_o,"%s,", cdr.answer_time);
						fprintf(fp_o,"%d,", cdr.callDuration);
						fprintf(fp_o,"%s,", cdr.dialledOtherParty);
						fprintf(fp_o,"%s,", cdr.translatedOtherParty);
						fprintf(fp_o,"%s,", cdr.otherMSRN);
						fprintf(fp_o,"%s,", cdr.otherParty);
						fprintf(fp_o,"%s,", cdr.thirdParty);
						fprintf(fp_o,"%s,", cdr.cellId);
						fprintf(fp_o,"%s\n",cdr.exchangeId);

						
	                        		pduType->free_struct(pduType,sr9Siemens,0);
	                        		sr9Siemens=0;
					}
				}
			} while( pdc == PD_FINISHED && !single_type_decoding );

			if(pdc == PD_FAILED)
			{
				if(decoded<my_cont_buff.length)
				{
					if(my_cont_buff.buff[decoded]==0xff)
					{
						if(my_debug)
							printf("## fill\n");
						pdc=PD_EOF;
					}
					else
					{
						err_log("siemens: process_deeper fail\n");
					}
				}		
			}
		}
		
		if( my_cont_buff.length < once_read )
		{
			if( ferror(fp_i) )
			{
				err_log("siemens: fread %s fail\n",in_file_name);
				ret=1;
				goto Exit_Pro;
			}
			break;
		}
		total+=my_cont_buff.length;
	}

	fclose(fp_i); 
	fp_i=NULL;
	fclose(fp_o); 
	fp_o=NULL;
	
Exit_Pro:
	if(fp_i!=NULL) fclose(fp_i);
	if(fp_o!=NULL) fclose(fp_o);

	return ret;
}

//------------------------------------------------------------------------------------------------
/*
 *imitate fgetc
 */
static int cont_buff_get(cont_buff * pcont_buff)
{
	if(pcont_buff->position>=pcont_buff->length)
		return -1;
	return(pcont_buff->buff[pcont_buff->position++]);
}

/*
 * Process the TLV recursively.
 */
static pd_code_e process_deeper(const char *fname, cont_buff * pcont_buff, int level, ssize_t limit, ssize_t *decoded, int expect_eoc)
{
	unsigned char tagbuf[32];
	ssize_t tblen = 0;
	pd_code_e pdc = PD_FINISHED;
	ber_tlv_tag_t tlv_tag;
	ber_tlv_len_t tlv_len;
	ssize_t t_len;
	ssize_t l_len;

	do {
		int constr;
		int ch;

		if(limit == 0)
			return PD_FINISHED;

		if(limit >= 0 && tblen >= limit) {
			fprintf(STDERR_OUT,
				"%s: Too long TL sequence (%ld >= %ld). "
					"Dangerous file\n",
				fname, (long)tblen, (long)limit);
			return PD_FAILED;
		}

		ch = cont_buff_get(pcont_buff);
		if(ch == -1) {
			if(tblen) {
				fprintf(STDERR_OUT,
					"%s: Unexpected end of file (TL)\n",
					fname);
				return PD_FAILED;
			} else {
				return PD_EOF;
			}
		}

		tagbuf[tblen++] = ch;

		/*
		 * Decode the TLV tag.
		 */
		t_len = ber_fetch_tag(tagbuf, tblen, &tlv_tag);
		switch(t_len) {
		case -1:
			fprintf(STDERR_OUT, "%s: Fatal error deciphering tag\n",
				fname);
			return PD_FAILED;
		case 0:
			/* More data expected */
			continue;
		}

                /* 
                 * Redefine Primitive PRIVATE 13 to Primitive APPLICATION 13
                 */ 
                if(tagbuf[0]==0xcd)
                {
                	tagbuf[0]=0x4d;
                	pcont_buff->buff[pcont_buff->position-tblen]=tagbuf[0];
                	tlv_tag= ((tlv_tag>>2)<<2)|0x1;
                }

		/*
		 * Decode the TLV length.
		 */
		constr = BER_TLV_CONSTRUCTED(tagbuf);
		l_len = ber_fetch_length(constr,
				tagbuf + t_len, tblen - t_len, &tlv_len);
		switch(l_len) {
		case -1:
			fprintf(STDERR_OUT, "%s: Fatal error deciphering length\n",
				fname);
			return PD_FAILED;
		case 0:
			/* More data expected */
			continue;
		}

		/* Make sure the T & L decoders took exactly the whole buffer */
		assert((t_len + l_len) == tblen);

		if(!expect_eoc || tagbuf[0] || tagbuf[1])
			print_TL(0, level, constr, tblen, tlv_tag, tlv_len);

		if(limit != -1) {
			/* If limit is set, account for the TL sequence */
			limit -= (t_len + l_len);
			assert(limit >= 0);

			if(tlv_len > limit) {
				fprintf(STDERR_OUT,
				"%s: Structure advertizes length (%ld) "
				"greater than of a parent container (%ld)\n",
					fname, (long)tlv_len, (long)limit);
				return PD_FAILED;
			}
		}

		*decoded += t_len + l_len;

		if(expect_eoc && tagbuf[0] == '\0' && tagbuf[1] == '\0') {
			/* End of content octets */
			print_TL(1, level - 1, 1, 2, 0, -1);
			return PD_FINISHED;
		}

		if(constr) {
			ssize_t dec = 0;
			/*
			 * This is a constructed type. Process recursively.
			 */
			printf(">\n");	/* Close the opening tag */
			if(tlv_len != -1 && limit != -1) {
				assert(limit >= tlv_len);
			}
			pdc = process_deeper(fname, pcont_buff, level + 1,
				tlv_len == -1 ? limit : tlv_len,
				&dec, tlv_len == -1);
			if(pdc == PD_FAILED) return pdc;
			if(limit != -1) {
				assert(limit >= dec);
				limit -= dec;
			}
			*decoded += dec;
			if(tlv_len == -1) {
				tblen = 0;
				continue;
			}
		} else {
			assert(tlv_len >= 0);
			if(print_V(fname, pcont_buff, tlv_tag, tlv_len))
				return PD_FAILED;

			if(limit != -1) {
				assert(limit >= tlv_len);
				limit -= tlv_len;
			}
			*decoded += tlv_len;
		}

		print_TL(1, level, constr, tblen, tlv_tag, tlv_len);

		tblen = 0;
		
		if(level==0)
			return pdc;
	} while(1);

	return pdc;
}

static void print_TL(int fin, int level, int constr, ssize_t tlen, ber_tlv_tag_t tlv_tag, ber_tlv_len_t tlv_len)
{
	if(fin && !constr) {
		printf("</P>\n");
		return;
	}

	while(level-- > 0) printf(indent_buffer);  /* Print indent */
	printf(fin ? "</" : "<");

	printf(constr ? ((tlv_len == -1) ? "I" : "C") : "P");

	printf(" T=\"");
	ber_tlv_tag_fwrite(tlv_tag, stdout);
	printf("\"");

	if(!fin || tlv_len == -1)
		printf(" TL=\"%ld\"", (long)tlen);
	if(!fin) {
		if(tlv_len == -1)
			printf(" V=\"Indefinite\"");
		else
			printf(" V=\"%ld\"", (long)tlv_len);
	}

	if(BER_TAG_CLASS(tlv_tag) == ASN_TAG_CLASS_UNIVERSAL) {
		const char *str;
		ber_tlv_tag_t tvalue = BER_TAG_VALUE(tlv_tag);
		str = ASN_UNIVERSAL_TAG2STR(tvalue);
		if(str) printf(" A=\"%s\"", str);
	}

	if(fin) printf(">\n");
}

/*
 * Print the value in binary form, or reformat for pretty-printing.
 */
static int print_V(const char *fname, cont_buff * pcont_buff, ber_tlv_tag_t tlv_tag, ber_tlv_len_t tlv_len)
{
    
	asn1c_integer_t *arcs = 0;	/* Object identifier arcs */
	unsigned char *vbuf = 0;
	asn1p_expr_type_e etype = 0;
	asn1c_integer_t collector = 0;
	int special_format = 0;
	ssize_t i;

	/* Figure out what type is it */
	if(BER_TAG_CLASS(tlv_tag) == ASN_TAG_CLASS_UNIVERSAL
	&& pretty_printing) {
		ber_tlv_tag_t tvalue = BER_TAG_VALUE(tlv_tag);
		etype = ASN_UNIVERSAL_TAG2TYPE(tvalue);
	}

	/*
	 * Determine how to print the value, either in its native binary form,
	 * encoded with &xNN characters, or using pretty-printing.
	 * The basic string types (including "useful types", like UTCTime)
	 * are excempt from this determination logic, because their alphabets
	 * are subsets of the XML's native UTF-8 encoding.
	 */
	switch(etype) {
	case ASN_BASIC_BOOLEAN:
		if(tlv_len == 1)
			special_format = 1;
		else
			etype = 0;
		break;
	case ASN_BASIC_INTEGER:
	case ASN_BASIC_ENUMERATED:
		if((size_t)tlv_len <= sizeof(collector))
			special_format = 1;
		else
			etype = 0;
		break;
	case ASN_BASIC_OBJECT_IDENTIFIER:
	case ASN_BASIC_RELATIVE_OID:
		if(tlv_len > 0 && tlv_len < 128*1024 /* VERY long OID! */) {
			arcs = malloc(sizeof(*arcs) * (tlv_len + 1));
			if(arcs) {
				vbuf = malloc(tlv_len + 1);
				/* Not checking is intentional */
			}
		}
	case ASN_BASIC_UTCTime:
	case ASN_BASIC_GeneralizedTime:
	case ASN_STRING_NumericString:
	case ASN_STRING_PrintableString:
	case ASN_STRING_VisibleString:
	case ASN_STRING_IA5String:
	case ASN_STRING_UTF8String:
		break;	/* Directly compatible with UTF-8 */
	case ASN_STRING_BMPString:
	case ASN_STRING_UniversalString:
		break;	/* Not directly compatible with UTF-8 */
	default:
		/* Conditionally compatible with UTF-8 */
		if((
			(etype & ASN_STRING_MASK)
			||
			(etype == ASN_BASIC_OCTET_STRING)
			||
			/*
			 * AUTOMATIC TAGS or IMPLICIT TAGS in effect,
			 * Treat this primitive type as OCTET_STRING.
			 */
			(BER_TAG_CLASS(tlv_tag) != ASN_TAG_CLASS_UNIVERSAL
				&& pretty_printing)
		) && (tlv_len > 0 && tlv_len < 128 * 1024)) {
			vbuf = malloc(tlv_len + 1);
			/* Not checking is intentional */
		}
		break;
	}

	/* If collection vbuf is present, defer printing the F flag. */
	if(!vbuf) printf(special_format ? " F>" : ">");

	/*
	 * Print the value in binary or text form,
	 * or collect the bytes into vbuf.
	 */
	for(i = 0; i < tlv_len; i++) {
		int ch=cont_buff_get(pcont_buff);
		if(ch == -1) {
			fprintf(STDERR_OUT,
			"%s: Unexpected end of file (V)\n", fname);
			if(vbuf) free(vbuf);
			if(arcs) free(arcs);
			return -1;
		}
		switch(etype) {
		case ASN_BASIC_UTCTime:
		case ASN_BASIC_GeneralizedTime:
		case ASN_STRING_NumericString:
		case ASN_STRING_PrintableString:
		case ASN_STRING_VisibleString:
		case ASN_STRING_IA5String:
		case ASN_STRING_UTF8String:
			switch(ch) {
			default:
				if(((etype == ASN_STRING_UTF8String)
					|| !(ch & 0x80))
				&& (ch >= 0x20)
				) {
					printf("%c", ch);
					break;
				}
				/* Fall through */
			case 0x3c: case 0x3e: case 0x26:
				printf("&#x%02x;", ch);
			}
			break;
		case ASN_BASIC_BOOLEAN:
			switch(ch) {
			case 0: printf("<false/>"); break;
			case 0xff: printf("<true/>"); break;
			default: printf("<true value=\"&#x%02x\"/>", ch);
			}
			break;
		case ASN_BASIC_INTEGER:
		case ASN_BASIC_ENUMERATED:
			if(i)	collector = collector * 256 + ch;
			else	collector = (int)(signed char)ch;
			break;
		default:
			if(vbuf) {
				vbuf[i] = ch;
			} else {
				printf("&#x%02x;", ch);
			}
		}
	}

	/* Do post-processing */
	switch(etype) {
	case ASN_BASIC_INTEGER:
	case ASN_BASIC_ENUMERATED:
		printf("%" PRIdASN, collector);
		break;
	case ASN_BASIC_OBJECT_IDENTIFIER:
		if(vbuf) {
			OBJECT_IDENTIFIER_t oid;
			int arcno;

			oid.buf = vbuf;
			oid.size = tlv_len;

			arcno = OBJECT_IDENTIFIER_get_arcs(&oid, arcs,
				sizeof(*arcs), tlv_len + 1);
			if(arcno >= 0) {
				assert(arcno <= (tlv_len + 1));
				printf(" F>");
				for(i = 0; i < arcno; i++) {
					if(i) printf(".");
					printf("%" PRIuASN, arcs[i]);
				}
				free(vbuf);
				vbuf = 0;
			}
		}
		break;
	case ASN_BASIC_RELATIVE_OID:
		if(vbuf) {
			RELATIVE_OID_t oid;
			int arcno;

			oid.buf = vbuf;
			oid.size = tlv_len;
	
			arcno = RELATIVE_OID_get_arcs(&oid, arcs,
				sizeof(*arcs), tlv_len);
			if(arcno >= 0) {
				assert(arcno <= (tlv_len + 1));
				printf(" F>");
				for(i = 0; i < arcno; i++) {
					if(i) printf(".");
					printf("%" PRIuASN, arcs[i]);
				}
				free(vbuf);
				vbuf = 0;
			}
		}
		break;
	default: break;
	}

	/*
	 * If the buffer was not consumed, print it out.
	 * It might be an OCTET STRING or other primitive type,
	 * which might actually be printable, but we need to figure it out.
	 */
	if(vbuf) {
		int binary;

		/*
		 * Check whether the data could be represented as text
		 */
		binary = -1 * (tlv_len >> 3); /* Threshold is 12.5% binary */
		for(i = 0; i < tlv_len; i++) {
			switch(vbuf[i]) {
			case 0x1b: binary = 1; break;
			case 0x09: case 0x0a: case 0x0d: continue;
			default:
				if(vbuf[i] < 0x20 || vbuf[i] >= 0x7f)
					if(++binary > 0)  /* Way too many */
						break;
				continue;
			}
			break;
		}
		printf(">");
		for(i = 0; i < tlv_len; i++) {
			if(binary > 0 || vbuf[i] < 0x20 || vbuf[i] >= 0x7f
				|| vbuf[i] == 0x26	/* '&' */
				|| vbuf[i] == 0x3c	/* '<' */
				|| vbuf[i] == 0x3e	/* '>' */
			)
				printf("&#x%02x;", vbuf[i]);
			else
				printf("%c", vbuf[i]);
		}
		free(vbuf);
	}

	if(arcs) free(arcs);
	return 0;
}

static int decode_tlv_from_string(const char *datastring)
{
	unsigned char *data, *dp;
	size_t dsize;	/* Data size */
	ssize_t len;
	ber_tlv_tag_t tlv_tag;
	ber_tlv_len_t tlv_len;
	const char *p;
	int half;

	dsize = strlen(datastring) + 1;
	dp = data = calloc(1, dsize);
	assert(data);

	for(half = 0, p = datastring; *p; p++) {
		switch(*p) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			*dp |= *p - '0'; break;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			*dp |= *p - 'A' + 10; break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			*dp |= *p - 'a' + 10; break;
		case ' ': case '\t': case '\r': case '\n':
			continue;
		default:
			fprintf(STDERR_OUT, "Unexpected symbols in data string:\n");
			fprintf(STDERR_OUT, "%s\n", datastring);
			for(dp = data; datastring < p; datastring++, dp++)
				*dp = ' ';
			*dp = '\0';
			fprintf(STDERR_OUT, "%s^ <- here\n", (char *)data);
			return -1;
		}
		if(half) dp++; else (*dp) <<= 4;
		half = !half;
	}

	assert((size_t)(dp - data) <= dsize);
	dsize = dp - data;

	printf("BER: ");
	for(dp = data; dp < data + dsize; dp++)
		printf("%02X", *dp);
	printf("\n");

	len = ber_fetch_tag(data, dsize, &tlv_tag);
	switch(len) {
	case -1:
		fprintf(STDERR_OUT, "TAG: Fatal error deciphering tag\n");
		return -1;
	case 0:
		fprintf(STDERR_OUT, "TAG: More data expected\n");
		return -1;
	default:
		printf("TAG: ");
		ber_tlv_tag_fwrite(tlv_tag, stdout);
		if(BER_TLV_CONSTRUCTED(data)) {
			printf(" (constructed)");
		} else if(dsize >= 2 && data[0] == 0 && data[1] == 0) {
			printf(" (end-of-content)");
		} else {
			printf(" (primitive)");
		}
		if(BER_TAG_CLASS(tlv_tag) == ASN_TAG_CLASS_UNIVERSAL) {
			const char *str;
			ber_tlv_tag_t tvalue = BER_TAG_VALUE(tlv_tag);
			str = ASN_UNIVERSAL_TAG2STR(tvalue);
			if(str) printf(" \"%s\"", str);
		}
		printf("\n");
	}

	if(dsize > (size_t)len) {
		len = ber_fetch_length(BER_TLV_CONSTRUCTED(data),
			data + len, dsize - len, &tlv_len);
		switch(len) {
		case -1:
			fprintf(STDERR_OUT,
				"LEN: Fatal error deciphering length\n");
			return -1;
		case 0:
			fprintf(STDERR_OUT, "LEN: More data expected\n");
			return -1;
		default:
			if(tlv_len == (ber_tlv_len_t)-1)
				printf("LEN: Indefinite length encoding\n");
			else
				printf("LEN: %ld bytes\n", (long)tlv_len);
		}
	}
	return 0;
}
