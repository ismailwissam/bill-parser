/**************************************************************************/
/* collect.c                                                              */
/*                                                                        */ 
/* ftp�ɼ�����                                                            */ 
/*                                                                        */
/* create by liuyang at 2010.4                                            */
/*                                                                        */
/* modify by wangxiaohui at 2010.6                                        */
/*     �����ع�����������Bug                                              */
/*     ������־����������Բɼ���Ϊ��ŵ�λ���Ϊ��MSC�豸����Ϊ��ŵ�λ  */
/*     �����ɼ���ʼʱ�䷽�������Ϊʵʱ���ݲɼ�                           */
/*                                                                        */
/* modify by wangxiaohui at 2010.7                                        */
/*     ���ɰ汾��                                                         */
/*     ͬʱ�Ѳɼ����򡢽����������������뵽ͳһ����£��ع�����ṹ   */
/*                                                                        */
/* modify by wangxiaohui at 2010.8                                        */
/*     ���������������Ftp�ɼ�����Ԫhzgs4��tzhds2��Ftp�Ĵ���ģʽ��passive */
/*     ģʽ�޸�Ϊportģʽ��                                               */
/**************************************************************************/

/************************************************************************/
/* include ͷ�ļ� */
/************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include "Ftp.h"
#include "common.h"
#include "collect.h"

/************************************************************************/
/* �궨�� */
/************************************************************************/
#define      MAX_COMPANY         64
#define      MAX_DEVICE          64
#define      MAX_FTP_IP          32
#define      MAX_FTP_USER        32
#define      MAX_FTP_PWD         32
#define      FTP_TIME_OUT        300
#define      PREFIX_RUN_LOG_FILE "./log/collect_run"

/************************************************************************/
/* �ṹ�嶨�� */
/************************************************************************/
//FTP�ɼ���Ϣ
typedef struct {
	//�ɼ�����
	int   collect_point;
	//���̺��豸
	char  company[MAX_COMPANY];
	char  device[MAX_DEVICE];
	//��½��Ϣ,v1.1��Ӷ˿�port
	char  ip[MAX_FTP_IP];
	int   port; //�˿�port:��Ϊ6621
	char  usr[MAX_FTP_USER];
	char  password[MAX_FTP_PWD];
	//����Ŀ¼
	char  path_str[MAX_FILENAME];
	char  path_up[MAX_FILENAME];
	char  path_last[MAX_FILENAME];
	char  path_pre[MAX_FILENAME];
	char  path_suf[MAX_FILENAME];
	int   is_multi_path;  
	//�����ļ�
	char  file_str[MAX_FILENAME];
	char  file_pre[MAX_FILENAME];
	char  file_suf[MAX_FILENAME];
	//����Ŀ¼
	char  backup_path[MAX_FILENAME];
	//���ֱ�ʶ
	int   is_backup;
	int   is_commit;
	int   is_interval_file;
    char  start_time[MAX_TIME];
    char  end_time[MAX_TIME];
	int   current_process_number;
} collect_conf; 

/************************************************************************/
/* �������� */
/************************************************************************/
static void         collect_task(void);
static parse_code_e get_collect_conf(int nCollectPointNo, int nCurrentProcessNumber, collect_conf * pCollectConf);
static int          process_collect(int current_number, int parallel_number);
static int			point_collect(int nCollectPointNo, int nCurrentProcessNo);
static int		 	convert_date_hw(char* lpOutDateTime, const char* lpInTime, const char* lpInDate);
static int			convert_date_hw_sp6(char* lpOutDateTime, char* lpInTime, const char* lpInDate);
static int			convert_date_nsn(char* lpOutDateTime, const char* lpInTime, const char* lpInDate);
static int			get_file_passive(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long * file_size);
static int			get_file_port(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long * file_size);
static int			backup_file(collect_conf * p_collect_conf, const char * remote_file_name, const char* lpTimeStamp);
static int			commit_file(collect_conf * p_collect_conf, const char * remote_file_name, const char* lpTimeStamp);
static int			get_backup_name(collect_conf * pCollectConf, const char * szRemoteFileName, const char* lpTimeStamp, char * szBackupName);
static int			run_log(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long file_size, const char* lpTimeStamp);

/************************************************************************/
/* ��ں���*/
/************************************************************************/
int start_collect_task(void)
{
    int pid;

    pid = fork();
    if(pid == 0) //child process
    {
        collect_task();
        exit(0);
    }

    return pid;
}

static void collect_task(void)
{
	int     i;
	t_child_process_status oChildProcessStatus[MAX_CHILD_PROCESS];
	pid_t   pidChild;
	pid_t   pidReValWait;
    BOOL    bAllCollectProcessIdle;

	if(collect_parallel_num<=0)                collect_parallel_num=1;
	if(collect_parallel_num>MAX_CHILD_PROCESS) collect_parallel_num=MAX_CHILD_PROCESS;
    if(collect_parallel_num>collect_point_num) collect_parallel_num=collect_point_num;

	/*��ʼ�� �ӽ��̽ṹ����*/
	for(i=0;i<MAX_CHILD_PROCESS;i++)
		memset(&oChildProcessStatus[i],0,sizeof(t_child_process_status));
	
	/*�ɼ�����*/
	while(1)
	{
		/*�����ӽ���*/
		for(i=0;i<collect_parallel_num;i++)
		{
			if(oChildProcessStatus[i].pid==0&&oChildProcessStatus[i].sleep_time<=0)
			{
				if((pidChild=fork())<0) /*fork error*/
				{
					err_log("main: fork error\n");
					exit(1);
				}
				else if(pidChild>0)     /**/ 
				{
					oChildProcessStatus[i].pid        = pidChild;
					oChildProcessStatus[i].sleep_time = PROCESS_SLEEP_TIME;
				}
				else if(pidChild==0)    /*child process*/
				{
					process_collect(i+1, collect_parallel_num);
					exit(0);
				}
			}
		}
		
		/*�����ӽ���*/
		for(i=0;i<collect_parallel_num;i++)
		{
			if(oChildProcessStatus[i].pid>0)
			{
				pidReValWait = waitpid(oChildProcessStatus[i].pid,NULL,WNOHANG);
				if(pidReValWait>0)
				{
					oChildProcessStatus[i].pid=0;
				}
				if(pidReValWait<0)
				{
					err_log("main: waitpid fail\n");
					exit(1);
				}
			}
		}
		
        /* �ж��Ƿ����вɼ����̶��Ѿ����� */
        bAllCollectProcessIdle = TRUE;
        for(i=0;i<collect_parallel_num;i++)
        {
            if(oChildProcessStatus[i].pid > 0) 
            {
                bAllCollectProcessIdle = FALSE;
                break;
            }
        }

        /* ������вɼ����̶��ѿ��У����˳��ɼ����� */
        if(bAllCollectProcessIdle)
        {
            break;
        }

		sleep(1);

		/*ÿ��ݼ�sleep_time*/
		for( i=0 ; i < collect_parallel_num; i++ )
		{
			if( oChildProcessStatus[i].pid == 0 && oChildProcessStatus[i].sleep_time > 0 )
			{	
				oChildProcessStatus[i].sleep_time--;
			}
		}
	}
}

/************************************************************************/
/* ��֤�ɼ����ú��� */
/************************************************************************/
/*
* verify_collect_conf()
*                      return 1 fail, 0 success
*/
int verify_collect_conf(int * ret_point_num)
{
	int           ret = 0;
	FILE          *pFile=NULL;
	char          szTempFileName[MAX_FILENAME];
	char	      szBuff[MAX_LONG_BUFFER];
	char          szContent[14][MAX_FILENAME];
	int           nLen;
    int           line;
    int           i;
	
	sprintf(szTempFileName,"%s/%s",CONF_DIR, COLLECT_CONF);

	pFile = fopen(szTempFileName,"r");
	if(pFile==NULL)
	{
		err_log("verify_collect_conf: fopen %s fail\n",szTempFileName);
		ret = 1;
		goto Exit_Pro;
	}
	
    line = 0;

	while(1)
	{
		memset(szBuff,0,sizeof(szBuff));
		memset(szContent,0,sizeof(szContent));
		if(fgets(szBuff,sizeof(szBuff),pFile)==NULL)
        {
			break;
        }
		
		line++;

        //��������п�ͷ��#ע�ͣ�����Բ����
        if(szBuff[0] == '#')
        {
            continue;
        }
    
        //��ȡ��������Ϣ
        if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],\
            szContent[4],szContent[5],szContent[6],szContent[7],szContent[8],szContent[9],szContent[10], \
            szContent[11], szContent[12], szContent[13]) != 14 )
        {
            err_log("verify_collect_conf: collect.conf line %d incorrect\n",line);
            ret=1;
            goto Exit_Pro;
        }
        
        //��֤�ɼ�����
        nLen=strlen(szContent[0]);
        if(nLen!=8 || szContent[0][0]!='<' || szContent[0][nLen-1]!='>')
        {
            err_log("verify_collect_conf: collect.conf line %d incorrect\n",line);
            ret=1;
            goto Exit_Pro;
        }
        
        for(i = 1; i < nLen -1; i++)
        {
            if(!isdigit(szContent[0][i]))
            {
                err_log("verify_collect_conf: collect.conf line %s collect point incorrect\n", line);
                ret = 1;
                goto Exit_Pro;
            }
        }

        //��֤�ɼ���ʼʱ��
        nLen = strlen(szContent[12]);
        if(nLen != 14)
        {
            err_log("verify_collect_conf: collect.conf line %s start time incorrect\n", line);
            ret = 1;
            goto Exit_Pro;
        }
        for(i = 0; i < nLen; i++)
        {
            if(!isdigit(szContent[12][i]))
            {
                err_log("verify_collect_conf: collect.conf line %s start time incorrect\n", line);
                ret = 1;
                goto Exit_Pro;
            }
        }

        //��֤�ɼ�����ʱ��
        nLen = strlen(szContent[13]);
        if(nLen != 14)
        {
            err_log("verify_collect_conf: collect.conf line %s end time incorrect\n", line);
            ret = 1;
            goto Exit_Pro;
        }
        for(i = 0; i < nLen; i++)
        {
            if(!isdigit(szContent[13][i]))
            {
                err_log("verify_collect_conf: collect.conf line %s end time incorrect\n", line);
                ret = 1;
                goto Exit_Pro;
            }
        }

	}

    (*ret_point_num) = line;
	
Exit_Pro:
	if(pFile!=NULL)
	{
		fclose(pFile);
	}
	return ret;
}

/************************************************************************/
/* ��ȡ�ɼ����� */
/************************************************************************/
static parse_code_e get_collect_conf(int nCollectPointNo,int nCurrentProcessNumber,collect_conf * pCollectConf)
{
	parse_code_e  ret;
	FILE        * pFile=NULL;
	char          szTempFileName[MAX_FILENAME];
	char	      szBuff[MAX_LONG_BUFFER];
	char          szContent[14][MAX_FILENAME];
	char        * szStr=NULL;
	int           nLen;
	int           nLineNo;
	
	ret = PARSE_UNMATCH;
	
	sprintf(szTempFileName,"%s/%s",CONF_DIR, COLLECT_CONF);
	pFile = fopen(szTempFileName,"r");
	if(pFile == NULL)
	{
		err_log("get_collect_conf: fopen %s fail\n",szTempFileName);
		ret = PARSE_FAIL;
		goto Exit_Pro;
	}
	
	nLineNo = 0;
	while(1)
	{
		memset(szBuff,0,sizeof(szBuff));
		if(fgets(szBuff,sizeof(szBuff),pFile)==NULL)
		{
			if(nLineNo < nCollectPointNo)
			{
				err_log("get_collect_conf: collect.conf line less %d\n",nCollectPointNo);
				ret = PARSE_FAIL;
				goto Exit_Pro;
			}
			break;
		}
       	
		nLineNo++;

        if(nLineNo != nCollectPointNo)
            continue;
    
        if(szBuff[0] == '#')
        {
            ret = PARSE_UNMATCH;
            break;
        }
        
        memset(szContent,0,sizeof(szContent));
        if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],\
            szContent[5],szContent[6],szContent[7],szContent[8],szContent[9],szContent[10],szContent[11],szContent[12],szContent[13]) != 14 )
        {
            err_log("get_collect_conf: collect.conf format incorrect\n");
            ret = PARSE_FAIL;
            goto Exit_Pro;
        }

        szContent[0][strlen(szContent[0]) - 1] = '\0';
       
        pCollectConf->collect_point = atoi(&szContent[0][1]);
        strcpy(pCollectConf->ip,szContent[3]);
        strcpy(pCollectConf->company,szContent[1]);
        strcpy(pCollectConf->device,szContent[2]);
        pCollectConf->port = atoi(szContent[4]);
        strcpy(pCollectConf->usr,szContent[5]);
        strcpy(pCollectConf->password,szContent[6]);
        strcpy(pCollectConf->path_str,szContent[7]);
        strcpy(pCollectConf->file_str,szContent[8]);
        strcpy(pCollectConf->backup_path,szContent[9]);
        strcpy(pCollectConf->start_time,szContent[12]);
        strcpy(pCollectConf->end_time, szContent[13]);
        pCollectConf->current_process_number = nCurrentProcessNumber;

        /*path_up,path_last,path_pre,path_suf,is_multi_path*/
        nLen = strlen(szContent[7]);
        if(nLen < 1)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret = PARSE_FAIL;
            goto Exit_Pro;
        }
        szStr = strrchr(szContent[7],'/');
        if(szStr == NULL)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret=PARSE_FAIL;
            goto Exit_Pro;
        }
        strncpy(pCollectConf->path_up,szContent[7],szStr-szContent[7]+1);
        if( &szContent[7][nLen-1]-szStr>0 )
        {
            strncpy(pCollectConf->path_last,szStr+1,&szContent[7][nLen-1]-szStr);
        }
        
        if( (nLen=strlen(pCollectConf->path_last))>0 )
        {
            szStr=strchr(pCollectConf->path_last,'*');
            
            if(szStr==NULL)
            {
                pCollectConf->is_multi_path=0;
            }
            else
            {
                pCollectConf->is_multi_path=1;
                if(szStr>pCollectConf->path_last)
                {
                    strncpy(pCollectConf->path_pre,pCollectConf->path_last,szStr-pCollectConf->path_last);
                }
            }
            
            szStr=strrchr(pCollectConf->path_last,'*');
            if(szStr!=NULL)
            {
                if( &(pCollectConf->path_last[nLen-1])-szStr>0 )
                {
                    strncpy(pCollectConf->path_suf,szStr+1,&(pCollectConf->path_last[nLen-1])-szStr);
                }
            }
        }
        else
        {
            pCollectConf->is_multi_path=0;
        }	
        
        /*file_pre,file_suf*/
        nLen = strlen(szContent[8]);
        if(nLen < 1)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret=PARSE_FAIL;
            goto Exit_Pro;
        }
        szStr=strchr(szContent[8],'*');
        if(szStr == NULL)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret=PARSE_FAIL;
            goto Exit_Pro;
        }
        if(szStr>szContent[8])
        {
            strncpy(pCollectConf->file_pre,szContent[8],szStr-szContent[8]);
        }
        szStr=strrchr(szContent[8],'*');
        if(szStr == NULL)
        {
            err_log("get_collect_conf: collect config format incorrect\n");
            ret=PARSE_FAIL;
            goto Exit_Pro;
        }
        if( &szContent[8][nLen-1]-szStr>0 )
        {
            strncpy(pCollectConf->file_suf,szStr+1,&szContent[8][nLen-1]-szStr);
        }
        
        /*is_backup,is_commit,is_interval_file*/
        if(strcasecmp(szContent[9],"not")==0)
        {
            pCollectConf->is_backup=0;
        }
        else
        {
            pCollectConf->is_backup=1;
        }
        
        if(strcasecmp(szContent[10],"yes")==0)
        {
            pCollectConf->is_commit=1;
        }
        else
        {
            pCollectConf->is_commit=0;
        }
        
        if(strcasecmp(szContent[11],"yes")==0)
        {
            pCollectConf->is_interval_file=1;
        }
        else
        {
            pCollectConf->is_interval_file=0;
        }
        
        ret = PARSE_MATCH;

        break;
	}

Exit_Pro:	
	if(pFile!=NULL)
		fclose(pFile);
	return ret;
}

/*
*process_task()
*
*
*/
static int process_collect(int current_number,int parallel_number)
{
	int     factor;
    int     collect_point_no;
	
	factor = 0;
	while(1)
	{
		collect_point_no = factor * parallel_number + current_number;
		if(collect_point_no > collect_point_num)  break;
		point_collect(collect_point_no, current_number);
		factor++;
	}
	
	return 0;
}

/************************************************************************/
/* �㽭�����ɼ����ר�òɼ����� */
/************************************************************************/
static int point_collect(int nCollectPointNo,int nCurrentProcessNo)
{
	int nRet = 0;
	int nRetVal;
	//�����ļ��ṹ
	collect_conf curCollectConf;
	//Ŀ¼�б�,�ļ��б�,ʱ��˵��ļ���
	char szDirListFile[MAX_FILENAME];
	char szFileListFile[MAX_FILENAME];
	char szTimePointFile[MAX_FILENAME];
	char szBackupFile[MAX_FILENAME];
	//�ļ����
	FILE * pFileDirList = NULL;
	FILE * pFileFileList = NULL;
	FILE * pFileTimePoint = NULL;
	//��ʱbuffer
	char szBuff[MAX_LONG_BUFFER];
	//��������
	char szContent[9][MAX_FILENAME];
	//ʱ��˵����ʱʱ��˵�
	char szCollectStartTime[MAX_TIME];
	char szCollectStartDate[MAX_DATE];
    char szCollectEndTime[MAX_TIME];
    char szCollectEndDate[MAX_DATE];
	char szTimePoint[MAX_TIME];
	char szFileTimeStamp[MAX_TIME];
	char szFileDateStamp[MAX_DATE];
	long lFileSize;
	char pre[MAX_DATE];

    //��ȡ��������
	memset(&curCollectConf,0,sizeof(curCollectConf));
	nRetVal = get_collect_conf(nCollectPointNo,nCurrentProcessNo,&curCollectConf);
	if(nRetVal == PARSE_FAIL)
	{
		err_log("point_collect: get_collect_conf fail, nCollectPointNo=%d\n",nCollectPointNo);
		nRet = 1;
		goto Exit_Pro;
	}
	if(nRetVal == PARSE_UNMATCH) 
	{
		/*collect.conf�����ļ���,������#��ͷ,������ֱ������*/
		nRet = 2;		
		goto Exit_Pro;
	}

	//��ȡ�ɼ���ʼʱ��
	sprintf(szTimePointFile, "%s/%s[%s]_TimePoint", WORK_DIR, curCollectConf.device, curCollectConf.ip);
	pFileTimePoint = fopen(szTimePointFile , "r"); 
	if( NULL == pFileTimePoint )
	{
		strcpy(szCollectStartTime, curCollectConf.start_time);
	}
	else if ( NULL == fgets(szCollectStartTime, sizeof(szCollectStartTime), pFileTimePoint) )
	{
		strcpy(szCollectStartTime, curCollectConf.start_time);
	}
    else
    {
        if(strncmp(szCollectStartTime, curCollectConf.start_time, 14) < 0)
        {
            strcpy(szCollectStartTime, curCollectConf.start_time);
        }
        else if(strncmp(szCollectStartTime, curCollectConf.end_time, 14) > 0)
        {
            /* ���������õĲɼ�����ʱ�䣬�˳��ɼ����� */
            nRet = 3;
            goto Exit_Pro;
        }
    }
    strcpy(szTimePoint, szCollectStartTime);
	strncpy(szCollectStartDate, szCollectStartTime, 8);
	szCollectStartDate[8] = '\0';
    if(pFileTimePoint != NULL)
    {
        fclose(pFileTimePoint);
        pFileTimePoint = NULL;
    }

    //��ȡ�ɼ��Ľ���ʱ��
    strcpy(szCollectEndTime, curCollectConf.end_time);
    strncpy(szCollectEndDate, szCollectEndTime, 8);
    szCollectEndDate[8] = '\0';

    //������ʱ�ļ���
	sprintf(szDirListFile, "%s/%06d_DirList", WORK_DIR, curCollectConf.collect_point);
	sprintf(szFileListFile, "%s/%06d_FileList", WORK_DIR, curCollectConf.collect_point);

    //�����ļ� 
	if (0 == strncmp("hzgs6", curCollectConf.device, 5) || 
		0 == strncmp("hzgs11", curCollectConf.device, 6) || 
		0 == strncmp("hzgs20", curCollectConf.device, 6) || 
		0 == strncmp("nbogs22", curCollectConf.device, 7) || 
		0 == strncmp("wzhgs26", curCollectConf.device, 7) || 
		0 == strncmp("shxgs7", curCollectConf.device, 6) || 
		0 == strncmp("jihgs2", curCollectConf.device, 6) || 
		0 == strncmp("jihgs11", curCollectConf.device, 7) || 
		0 == strncmp("zshgs1", curCollectConf.device, 6) || 
		0 == strncmp("lshgs1", curCollectConf.device, 6) || 
		0 == strncmp("lshgs2", curCollectConf.device, 6) || 
		0 == strncmp("tzhds4", curCollectConf.device, 6) || 
		0 == strncmp("huzds1", curCollectConf.device, 6) || 
		0 == strncmp("hzgs9", curCollectConf.device, 5))
	{
		/* ɾ���ɵ�Ŀ¼�б��ļ� */
		if( 0 == access(szDirListFile, F_OK) )
		{
			if(0 != unlink(szDirListFile))
			{
				err_log("point_collect: unlink %s fail\n",szDirListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* ����Ftp���� */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,1,debug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* ��ȡĿ¼�б� */
		nRetVal = Ftp_Dir(szDirListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* �ر�Ftp���� */
		Ftp_Close();

		//���ļ�
		pFileDirList = fopen( szDirListFile , "r" );
		if(pFileDirList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szDirListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//ѭ������Ŀ¼
		while (1)
		{		
			//��ȡһ������
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileDirList) == NULL )
				break;

			//Ӧ���� 4 ������
			if( sscanf(szBuff,"%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3]) != 4 )
				continue;

			//�ж��Ƿ�Ҫ����
			if ( NULL == strstr(szContent[2], "DIR") ) //Ŀ¼
			{
				continue;	
			}
			if ( '2' != szContent[3][0] || '0' != szContent[3][1] )	//��20��ͷ
			{
				continue;	
			}
			if ( 8 != strlen(szContent[3])) //8λ��
			{
				continue;	
			}

            //��ȡ�����ļ�����������
			strcpy(szFileDateStamp, szContent[3]);

            //��������ļ�����������С�ڲɼ���ʼ���ڣ����������ļ���
			if(strncmp(szFileDateStamp, szCollectStartDate, 8) < 0)
			{
				continue;	
			}
	
            // ��������ļ����������ڴ��ڲɼ��������ڣ���ֱ���˳����˲ɼ�
            if(strncmp(szFileDateStamp, szCollectEndDate, 8) > 0)
            {
                break;
            }

			/* ɾ���ɵ������ļ��б��ļ� */
			if( 0 == access(szFileListFile, F_OK) )
			{
				if(0 != unlink(szFileListFile))
				{
					err_log("point_collect: unlink %s fail\n",szDirListFile);
					nRet = 1;		
					goto Exit_Pro;
				}
			}			

			/* ����Ftp���� */
			nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
				curCollectConf.port,FTP_TIME_OUT,1,1,debug);
			if(nRetVal != 0)
			{
				err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
					curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
				nRet = 1;		
				goto Exit_Pro;
			}

			//����Ŀ¼
			nRetVal = Ftp_Cd(szContent[3]);
			if( 0 != nRetVal )
			{
				err_log("point_collect: Ftp_Cd fail,collect_point=%d,dir=%s\n",curCollectConf.collect_point,szContent[8]);
				nRet = 1;		
				goto Exit_Pro;
			}

			/* ��ȡ���µ������ļ��б� */
			nRetVal = Ftp_Dir(szFileListFile);
			if( 0 != nRetVal )
			{
				err_log("point_collect: collect_point=%d,Ftp_Dir fail",curCollectConf.collect_point);
				nRet = 1;		
				goto Exit_Pro;
			}

            /* �ر�Ftp���� */
			Ftp_Close();

			//���ļ�
			pFileFileList = fopen( szFileListFile , "r" );
			if(szFileListFile == NULL)
			{
				err_log("point_collect: fopen %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}

			//ѭ������
			while (1)
			{
				//��ȡһ������
				memset(szBuff, 0 , sizeof(szBuff));
				memset(szContent, 0 , sizeof(szContent));
				if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				{
					break;
				}

				//Ӧ����4������
				if(sscanf(szBuff,"%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3]) != 4 )
					continue;

                //����ǾɵĻ����ļ����Ͳ�������
                if(strncmp(szContent[3], "gz", 2) != 0)
                {
                    continue;
                }

				//��ȡ�����ļ�������ʱ���
				if (0 != convert_date_hw_sp6(szFileTimeStamp, szContent[1], szFileDateStamp))
				{
					continue;
				}

                //�����ļ�ʱ�������ʱʱ���
				if ( strncmp(szTimePoint, szFileTimeStamp, 14) < 0 )
				{
					strcpy(szTimePoint, szFileTimeStamp);
				}

                //��������ļ�������ʱ��С�ڲɼ���ʼʱ�䣬�����أ���ȡ��һ�������ļ�
				if(strncmp(szFileTimeStamp, szCollectStartTime, 14) < 0)
				{
					continue;	
				}

                //��������ļ�������ʱ����ڲɼ�����ʱ�䣬���˳����˲ɼ�
                if(strncmp(szFileTimeStamp, curCollectConf.end_time, 14) > 0)
                {
                    break;
                }

                //�����������ļ�֮ǰ�Ѿ����ع�
                //���û������ǿ�Ƹ��±�־-f, �������أ���ȡ��һ���ļ�
                //���������ǿ�Ƹ��±�־-f����ǿ����������
				if(get_backup_name(&curCollectConf,szContent[3],szFileTimeStamp,szBackupFile)!=0) //��ȡ�����ļ���
				{
					err_log("point_collect: get_backup_name fail\n");
					nRet = 1;
					goto Exit_Pro;
				}

				if(0 == access(szBackupFile,F_OK))  //�ļ�����
				{
                    if(!force_update)
                    {
                        continue;
                    }
				}

                //�����ļ�
				if(get_file_passive(&curCollectConf, szFileDateStamp, szContent[3], &lFileSize)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_passive FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //�����ļ�
				if(backup_file(&curCollectConf, szContent[3], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //�ύ�ļ�
				if(commit_file(&curCollectConf, szContent[3], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //��¼��־
				if(run_log(&curCollectConf,szFileDateStamp,szContent[3],lFileSize,szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}
			}
		}
	}
    else if(0 == strncmp("hzgs4", curCollectConf.device, 5))
    {
		/* ��Ϊ����MSC�豸�Ĳɼ� */

		/* ɾ���ɵ�Ŀ¼�б��ļ� */
		if( 0 == access(szDirListFile, F_OK) )
		{
			if(0 != unlink(szDirListFile))
			{
				err_log("point_collect: unlink %s fail\n",szDirListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* ����Ftp���� */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,0,debug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* ��ȡĿ¼�б� */
		nRetVal = Ftp_Dir(szDirListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* �ر�Ftp���� */
		Ftp_Close();

		//���ļ�
		pFileDirList = fopen( szDirListFile , "r" );
		if(pFileDirList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szDirListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//ѭ������Ŀ¼
		while (1)
		{		
			//��ȡһ������
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileDirList) == NULL )
				break;

			//Ӧ���� 4 ������
			if( sscanf(szBuff,"%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3]) != 4 )
				continue;

			//�ж��Ƿ�Ҫ����
			if ( NULL == strstr(szContent[2], "DIR") ) //Ŀ¼
			{
				continue;	
			}
			if ( '2' != szContent[3][0] || '0' != szContent[3][1] )	//��20��ͷ
			{
				continue;	
			}
			if ( 8 != strlen(szContent[3])) //8λ��
			{
				continue;	
			}

            //��ȡ�����ļ�����������
			strcpy(szFileDateStamp, szContent[3]);

            //��������ļ�����������С�ڲɼ���ʼ���ڣ����������ļ���
			if(strncmp(szFileDateStamp, szCollectStartDate, 8) < 0)
			{
				continue;	
			}
	
            // ��������ļ����������ڴ��ڲɼ��������ڣ���ֱ���˳����˲ɼ�
            if(strncmp(szFileDateStamp, szCollectEndDate, 8) > 0)
            {
                break;
            }

			/* ɾ���ɵ������ļ��б��ļ� */
			if( 0 == access(szFileListFile, F_OK) )
			{
				if(0 != unlink(szFileListFile))
				{
					err_log("point_collect: unlink %s fail\n",szDirListFile);
					nRet = 1;		
					goto Exit_Pro;
				}
			}			

			/* ����Ftp���� */
			nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
				curCollectConf.port,FTP_TIME_OUT,1,0,debug);
			if(nRetVal != 0)
			{
				err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
					curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
				nRet = 1;		
				goto Exit_Pro;
			}

			//����Ŀ¼
			nRetVal = Ftp_Cd(szContent[3]);
			if( 0 != nRetVal )
			{
				err_log("point_collect: Ftp_Cd fail,collect_point=%d,dir=%s\n",curCollectConf.collect_point,szContent[8]);
				nRet = 1;		
				goto Exit_Pro;
			}

			/* ��ȡ���µ������ļ��б� */
			nRetVal = Ftp_Dir(szFileListFile);
			if( 0 != nRetVal )
			{
				err_log("point_collect: collect_point=%d,Ftp_Dir fail",curCollectConf.collect_point);
				nRet = 1;		
				goto Exit_Pro;
			}

            /* �ر�Ftp���� */
			Ftp_Close();

			//���ļ�
			pFileFileList = fopen( szFileListFile , "r" );
			if(szFileListFile == NULL)
			{
				err_log("point_collect: fopen %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}

			//ѭ������
			while (1)
			{
				//��ȡһ������
				memset(szBuff, 0 , sizeof(szBuff));
				memset(szContent, 0 , sizeof(szContent));
				if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				{
					break;
				}

				//Ӧ����4������
				if(sscanf(szBuff,"%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3]) != 4 )
					continue;

                //����ǾɵĻ����ļ����Ͳ�������
                if(strncmp(szContent[3], "gz", 2) != 0)
                {
                    continue;
                }

				//��ȡ�����ļ���ʱ���
				if (0 != convert_date_hw_sp6(szFileTimeStamp, szContent[1], szFileDateStamp))
				{
					continue;
				}

                //�����ļ�ʱ�������ʱʱ���
				if ( strncmp(szTimePoint, szFileTimeStamp, 14) < 0 )
				{
					strcpy(szTimePoint, szFileTimeStamp);
				}

                //��������ļ�������ʱ��С�ڲɼ���ʼʱ�䣬�����أ���ȡ��һ�������ļ�
				if(strncmp(szFileTimeStamp, szCollectStartTime, 14) < 0)
				{
					continue;	
				}

                //��������ļ�������ʱ����ڲɼ�����ʱ�䣬���˳����˲ɼ�
                if(strncmp(szFileTimeStamp, curCollectConf.end_time, 14) > 0)
                {
                    break;
                }

                //�����������ļ�֮ǰ�Ѿ����ع�
                //���������ǿ�Ƹ��±�־-f����ǿ����������
                //���û������ǿ�Ƹ��±�־-f�����Թ���������ȡ��һ��
				if(get_backup_name(&curCollectConf,szContent[3],szFileTimeStamp,szBackupFile)!=0) //��ȡ�����ļ���
				{
					err_log("point_collect: get_backup_name fail\n");
					nRet = 1;
					goto Exit_Pro;
				}

				if(0 == access(szBackupFile,F_OK))  //�ļ�����
				{
                    if(!force_update)
                    {
                          continue;
                    }
				}

                //�����ļ�
				if(get_file_port(&curCollectConf, szFileDateStamp, szContent[3], &lFileSize)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_port FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //�����ļ�
				if(backup_file(&curCollectConf, szContent[3], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //�ύ�ļ�
				if(commit_file(&curCollectConf, szContent[3], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //��¼��־
				if(run_log(&curCollectConf,szFileDateStamp,szContent[3],lFileSize,szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[3]);
					nRet = 1;		
					goto Exit_Pro;
				}
			}
		}
    }
    else if(0 == strncmp("tzhds2", curCollectConf.device, 6))
    {
        //hw:��ȡĿ¼�б�,��������Ŀ¼��ȡ�ļ��б�,Ȼ��Ա��ļ�ʱ���ʱ��˵�,�����
		/* ɾ���ɵ�Ŀ¼�б��ļ� */
		if( 0 == access(szDirListFile, F_OK) )
		{
			if(0 != unlink(szDirListFile))
			{
				err_log("point_collect: unlink %s fail\n",szDirListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* ����Ftp���� */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,0,debug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* ��ȡ���µ�Ŀ¼�б� */
		nRetVal = Ftp_Dir(szDirListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* �ر�Ftp���� */
		Ftp_Close();

		//���ļ�
		pFileDirList = fopen( szDirListFile , "r" );
		if(pFileDirList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szDirListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//ѭ������Ŀ¼
		while (1)
		{		
			//��ȡһ������
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileDirList) == NULL )
				break;

			//Ӧ����9������
			if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
				szContent[5],szContent[6],szContent[7],szContent[8]) != 9 )
				continue;

			//�ж��Ƿ�Ҫ����
			if ( 'd' != szContent[0][0] ) //Ŀ¼
			{
				continue;	
			}
			if ( '2' != szContent[8][0] || '0' != szContent[8][1] )	//��20��ͷ
			{
				continue;	
			}
			if ( 8 != strlen(szContent[8])) //8λ��
			{
				continue;	
			}

            //��ȡ�����ļ�����������
			strcpy(szFileDateStamp, szContent[8]);

            //��������ļ�����������С�ڲɼ���ʼ���ڣ��������òɼ��ļ��� 
			if (strncmp(szFileDateStamp, szCollectStartDate, 8) < 0)
			{
				continue;	
			}
		
            // ��������ļ����������ڴ��ڲɼ��������ڣ���ֱ���˳����˲ɼ�
            if(strncmp(szFileDateStamp, szCollectEndDate, 8) > 0)
            {
                break;
            }

			/* ɾ���ɵ������ļ��б��ļ� */
			if( 0 == access(szFileListFile, F_OK) )
			{
				if(0 != unlink(szFileListFile))
				{
					err_log("point_collect: unlink %s fail\n",szDirListFile);
					nRet = 1;		
					goto Exit_Pro;
				}
			}			
		
			/* ����Ftp���� */
			nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
				curCollectConf.port,FTP_TIME_OUT,1,0,debug);
			if(nRetVal != 0)
			{
				err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
					curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
				nRet = 1;		
				goto Exit_Pro;
			}

			//����Ŀ¼
			nRetVal = Ftp_Cd(szContent[8]);
			if( 0 != nRetVal )
			{
				err_log("point_collect: Ftp_Cd fail,collect_point=%d,dir=%s\n",curCollectConf.collect_point,szContent[8]);
				nRet = 1;		
				goto Exit_Pro;
			}

			/* ��ȡ���µ������ļ��б� */
			nRetVal = Ftp_Dir(szFileListFile);
			if( 0 != nRetVal )
			{
				err_log("point_collect: collect_point=%d,Ftp_Dir fail",curCollectConf.collect_point);
				nRet = 1;		
				goto Exit_Pro;
			}

            /* �ر�Ftp���� */
			Ftp_Close();

			//���ļ�
			pFileFileList = fopen( szFileListFile , "r" );
			if(szFileListFile == NULL)
			{
				err_log("point_collect: fopen %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}

			//ѭ������
			while (1)
			{
				//��ȡһ������
				memset(szBuff, 0 , sizeof(szBuff));
				memset(szContent, 0 , sizeof(szContent));
				if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				{
					break;
				}

				//Ӧ����9������
				if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
					szContent[5],szContent[6],szContent[7],szContent[8]) != 9 )
					continue;

                //����ǾɵĻ����ļ����Ͳ�������
                if(strncmp(szContent[8], "gz", 2) != 0)
                {
                    continue;
                }

				//�ж��Ƿ�Ϊ�ļ���Ϣ
				if ( '-' != szContent[0][0] ) //�ļ�
				{
					continue;	
				}

                //��ȡ�����ļ�������ʱ��
				if (0 != convert_date_hw(szFileTimeStamp, szContent[7], szFileDateStamp))
				{
					continue;
				}

                //�����ļ�ʱ�������ʱʱ���
				if ( strncmp(szTimePoint, szFileTimeStamp, 14) < 0 )
				{
					strcpy(szTimePoint, szFileTimeStamp);
				}

                //��������ļ�������ʱ��С�ڲɼ���ʼʱ�䣬�����أ��ж���һ�������ļ�
				if (strncmp(szFileTimeStamp, szCollectStartTime, 14) < 0)
				{
					continue;	
				}

                //��������ļ�������ʱ����ڲɼ�����ʱ�䣬���˳���������
                if(strncmp(szFileTimeStamp, curCollectConf.end_time, 14) > 0)
                {
                    break;
                }

                //����û����ļ�֮ǰ�Ѿ����ع�
                //���������ǿ�Ƹ��±�־-f����ǿ����������
                //���û������ǿ�Ƹ��±�־-f�����Թ��û�����ȡ��һ��
				if(get_backup_name(&curCollectConf,szContent[8],szFileTimeStamp,szBackupFile)!=0) //��ȡ�����ļ���
				{
					err_log("point_collect: get_backup_name fail\n");
					nRet = 1;
					goto Exit_Pro;
				}

				if(0 == access(szBackupFile,F_OK))  //�ļ�����
				{
                    if(!force_update)
                    {
                        continue;
                    }
				}

                //�����ļ�
				if(get_file_port(&curCollectConf, szFileDateStamp, szContent[8], &lFileSize)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_port FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //�����ļ�
				if(backup_file(&curCollectConf, szContent[8], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //�ύ�ļ�
				if(commit_file(&curCollectConf, szContent[8], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //��¼��־
				if(run_log(&curCollectConf,szFileDateStamp,szContent[8],lFileSize,szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}
			}
		}
    } 
	else if(0 == strncmp("hw", curCollectConf.company, 2))
	{
        //hw:��ȡĿ¼�б�,��������Ŀ¼��ȡ�ļ��б�,Ȼ��Ա��ļ�ʱ���ʱ��˵�,�����
		/* ɾ���ɵ�Ŀ¼�б��ļ� */
		if( 0 == access(szDirListFile, F_OK) )
		{
			if(0 != unlink(szDirListFile))
			{
				err_log("point_collect: unlink %s fail\n",szDirListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* ����Ftp���� */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,1,debug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* ��ȡ���µ�Ŀ¼�б� */
		nRetVal = Ftp_Dir(szDirListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* �ر�Ftp���� */
		Ftp_Close();

		//���ļ�
		pFileDirList = fopen( szDirListFile , "r" );
		if(pFileDirList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szDirListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//ѭ������Ŀ¼
		while (1)
		{		
			//��ȡһ������
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileDirList) == NULL )
				break;

			//Ӧ����9������
			if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
				szContent[5],szContent[6],szContent[7],szContent[8]) != 9 )
				continue;

			//�ж��Ƿ�Ҫ����
			if ( 'd' != szContent[0][0] ) //Ŀ¼
			{
				continue;	
			}
			if ( '2' != szContent[8][0] || '0' != szContent[8][1] )	//��20��ͷ
			{
				continue;	
			}
			if ( 8 != strlen(szContent[8])) //8λ��
			{
				continue;	
			}

            //��ȡ�����ļ�����������
			strcpy(szFileDateStamp, szContent[8]);

            //��������ļ�����������С�ڲɼ���ʼ���ڣ��������òɼ��ļ��� 
			if (strncmp(szFileDateStamp, szCollectStartDate, 8) < 0)
			{
				continue;	
			}
		
            // ��������ļ����������ڴ��ڲɼ��������ڣ���ֱ���˳����˲ɼ�
            if(strncmp(szFileDateStamp, szCollectEndDate, 8) > 0)
            {
                break;
            }

			/* ɾ���ɵ������ļ��б��ļ� */
			if( 0 == access(szFileListFile, F_OK) )
			{
				if(0 != unlink(szFileListFile))
				{
					err_log("point_collect: unlink %s fail\n",szDirListFile);
					nRet = 1;		
					goto Exit_Pro;
				}
			}			
		
			/* ����Ftp���� */
			nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
				curCollectConf.port,FTP_TIME_OUT,1,1,debug);
			if(nRetVal != 0)
			{
				err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
					curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
				nRet = 1;		
				goto Exit_Pro;
			}

			//����Ŀ¼
			nRetVal = Ftp_Cd(szContent[8]);
			if( 0 != nRetVal )
			{
				err_log("point_collect: Ftp_Cd fail,collect_point=%d,dir=%s\n",curCollectConf.collect_point,szContent[8]);
				nRet = 1;		
				goto Exit_Pro;
			}

			/* ��ȡ���µ������ļ��б� */
			nRetVal = Ftp_Dir(szFileListFile);
			if( 0 != nRetVal )
			{
				err_log("point_collect: collect_point=%d,Ftp_Dir fail",curCollectConf.collect_point);
				nRet = 1;		
				goto Exit_Pro;
			}

            /* �ر�Ftp���� */
			Ftp_Close();

			//���ļ�
			pFileFileList = fopen( szFileListFile , "r" );
			if(szFileListFile == NULL)
			{
				err_log("point_collect: fopen %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}

			//ѭ������
			while (1)
			{
				//��ȡһ������
				memset(szBuff, 0 , sizeof(szBuff));
				memset(szContent, 0 , sizeof(szContent));
				if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				{
					break;
				}

				//Ӧ����9������
				if( sscanf(szBuff,"%s%s%s%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],szContent[4],
					szContent[5],szContent[6],szContent[7],szContent[8]) != 9 )
					continue;

                //����ǾɵĻ����ļ����Ͳ�������
                if(strncmp(szContent[8], "gz", 2) != 0)
                {
                    continue;
                }

				//�ж��Ƿ�Ҫ����
				if ( '-' != szContent[0][0] ) //�ļ�
				{
					continue;	
				}

                //��ȡ�����ļ�������ʱ��
				if (0 != convert_date_hw(szFileTimeStamp, szContent[7], szFileDateStamp))
				{
					continue;
				}

                //�����ļ�ʱ�������ʱʱ���
				if ( strncmp(szTimePoint, szFileTimeStamp, 14) < 0 )
				{
					strcpy(szTimePoint, szFileTimeStamp);
				}

                //��������ļ�������ʱ��С�ڲɼ���ʼʱ�䣬�����أ��ж���һ�������ļ�
				if (strncmp(szFileTimeStamp, szCollectStartTime, 14) < 0)
				{
					continue;	
				}

                //��������ļ�������ʱ����ڲɼ�����ʱ�䣬���˳���������
                if(strncmp(szFileTimeStamp, curCollectConf.end_time, 14) > 0)
                {
                    break;
                }

                //����û����ļ�֮ǰ�Ѿ����ع���������
                //���������ǿ�Ƹ��±�־-f������������
                //���û������ǿ�Ƹ��±�־-f�����Թ��û����ж���һ��
				if(get_backup_name(&curCollectConf,szContent[8],szFileTimeStamp,szBackupFile)!=0) //��ȡ�����ļ���
				{
					err_log("point_collect: get_backup_name fail\n");
					nRet = 1;
					goto Exit_Pro;
				}

				if(0 == access(szBackupFile,F_OK))  //�ļ�����
				{
                    if(!force_update)
                    {
                        continue;
                    }
				}

                //�����ļ�
				if(get_file_passive(&curCollectConf, szFileDateStamp, szContent[8], &lFileSize)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_passive FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //�����ļ�
				if(backup_file(&curCollectConf, szContent[8], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //�ύ�ļ�
				if(commit_file(&curCollectConf, szContent[8], szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}

                //��¼��־
				if(run_log(&curCollectConf,szFileDateStamp,szContent[8],lFileSize,szFileTimeStamp)!=0)
				{
					err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[8]);
					nRet = 1;		
					goto Exit_Pro;
				}
			}
		}
	}
	else if(0 == strncmp("nsn", curCollectConf.company, 3))
	{
        //nsn:��ȡ�ļ��б�,�Ա��ļ�ʱ���ʱ��˵�,�����
		/* ɾ���ɵ�Ŀ¼�б��ļ� */
		if( 0 == access(szFileListFile, F_OK) )
		{
			if(0 != unlink(szFileListFile))
			{
				err_log("point_collect: unlink %s fail\n",szFileListFile);
				nRet = 1;		
				goto Exit_Pro;
			}
		}

        /* ����Ftp���� */
        nRetVal = Ftp_Init(curCollectConf.usr,curCollectConf.password,curCollectConf.ip,
                            curCollectConf.port,FTP_TIME_OUT,1,1,debug);
        if(nRetVal != 0)
        {
            err_log("point_collect: Ftp_Init fail,collect_point=%d\n%s\n%s\n%s\n%s\n",
                curCollectConf.collect_point,curCollectConf.ip,curCollectConf.port,curCollectConf.usr,curCollectConf.password);
            nRet = 1;		
            goto Exit_Pro;
        }

		/* ��ȡ���µ�Ŀ¼�б� */
		nRetVal = Ftp_Dir(szFileListFile);
		if( 0 != nRetVal )
		{
			err_log("point_collect: collect_point=%d,Ftp_Dir fail,%s",curCollectConf.collect_point,szDirListFile);
			nRet = 1;		
			goto Exit_Pro;
		}

        /* �ر�Ftp���� */
		Ftp_Close();

		//���ļ�
		pFileFileList = fopen( szFileListFile , "r" );
		if(pFileFileList == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szFileListFile);
			nRet = 1;
			goto Exit_Pro;
		}

		//ѭ������
		while (1)
		{
			//��ȡһ������
			memset(szBuff, 0 , sizeof(szBuff));
			memset(szContent, 0 , sizeof(szContent));
			if(fgets(szBuff, sizeof(szBuff), pFileFileList) == NULL )
				break;

			//Ӧ����6������
			if( sscanf(szBuff,"%s%s%s%s%s%s",szContent[0],szContent[1],szContent[2],szContent[3],
				szContent[4],szContent[5]) != 6 )
				continue;

			//�ж��Ƿ�Ҫ����
			if ( 10 != strlen(szContent[5]) )//�ļ���10���ַ�
			{
				continue;	
			}			

			if ( '.' != szContent[5][6] || 'D' != szContent[5][7] || 
				 'A' != szContent[5][8] || 'T' != szContent[5][9] ) //�ļ���β�̶�.DAT
			{
				continue;	
			}

            //��ȡ�����ļ�������ʱ��
			if (0 != convert_date_nsn(szFileTimeStamp, szContent[0], szContent[1]))//�õ��ļ�ʱ���
			{
				continue;
			}
	
			//�����ļ�ʱ�������ʱʱ���
			if ( strncmp(szTimePoint, szFileTimeStamp, 14) < 0 )
			{
				strcpy(szTimePoint, szFileTimeStamp);
			}

            //��������ļ�������ʱ��С�ڲɼ���ʼʱ�䣬�����أ��ж���һ��
			if (strncmp(szFileTimeStamp, szCollectStartTime, 14) < 0)
			{
				continue;	
			}

            //��������ļ�������ʱ����ڲɼ�����ʱ�䣬���˳���������
            if(strncmp(szFileTimeStamp, curCollectConf.end_time, 14) > 0)
            {
                break;
            }

            //����û����ļ�֮ǰ�Ѿ����ع�
            //���������ǿ�Ƹ��±�־-f������������
            //���û������ǿ�Ƹ��±�־-f�����Թ��������ж���һ��
			if(get_backup_name(&curCollectConf,szContent[5],szFileTimeStamp,szBackupFile)!=0) //��ȡ�����ļ���
			{
				err_log("point_collect: get_backup_name fail\n");
				nRet = 1;
				goto Exit_Pro;
			}

			if(0 == access(szBackupFile,F_OK))  //�ļ�����
			{
                if(!force_update)
                {
                    continue;
                }
			}

			//�����ļ�
			if(get_file_passive(&curCollectConf, NULL, szContent[5], &lFileSize) != 0 )
			{
				err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,get_file_passive FAIL\n",nCollectPointNo,szContent[5]);
				nRet = 1;
				goto Exit_Pro;
			}
			
			//�����ļ�
			if(backup_file(&curCollectConf, szContent[5], szFileTimeStamp)!=0)
			{
				err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,backup_file FAIL\n",nCollectPointNo,szContent[5]);
				nRet = 1;
				goto Exit_Pro;
			}

			//�ύ�ļ�
			if(commit_file(&curCollectConf, szContent[5], szFileTimeStamp)!=0)
			{
				err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,commit_file FAIL\n",nCollectPointNo,szContent[5]);
				nRet = 1;
				goto Exit_Pro;
			}

			//��¼��־
			if(run_log(&curCollectConf,NULL,szContent[5],lFileSize,szFileTimeStamp)!=0)
			{
				err_log("point_collect: nCollectPointNo=%d,szRemoteFileName=%s,run_log FAIL\n",nCollectPointNo,szContent[5]);
				nRet = 1;
				goto Exit_Pro;
			}
        }	
	}
    else
    {
        err_log("point_collect: unknown vendor device, vendor = %s, msc = %s\n", curCollectConf.company, curCollectConf.device);
        nRet = 1;
        goto Exit_Pro;
    }

    //����	
	if (strncmp(szCollectStartTime, szTimePoint ,14) < 0)
	{
		if (pFileTimePoint != NULL)
		{
			fclose(pFileTimePoint);
			pFileTimePoint = NULL;
		}

		pFileTimePoint = fopen( szTimePointFile , "w" );
		if(pFileTimePoint == NULL)
		{
			err_log("point_collect: fopen %s fail\n",szFileListFile);
			nRet = 1;
			goto Exit_Pro;
		}
		fprintf(pFileTimePoint, "%s", szTimePoint);
		fclose(pFileTimePoint);
		pFileTimePoint = NULL;
	}

	//����workĿ¼��,xxx_ ��ͷ�ļ�,xxxΪ��ǰ�ӽ��̱��
	sprintf(pre,"%03d",nCurrentProcessNo);
    if(clear_dir_file(WORK_DIR, pre, NULL) != 0)
    {
        err_log("point_collect: clear_dir_file fail: %s\n", WORK_DIR);
        nRet=1;
        goto Exit_Pro;
    }

Exit_Pro:
	Ftp_Close();
	if (pFileDirList != NULL)
	{
		fclose(pFileDirList);
		pFileDirList = NULL;
	}
	if (pFileFileList != NULL)
	{
		fclose(pFileFileList);
		pFileFileList = NULL;
	}
	if (pFileTimePoint != NULL)
	{
		fclose(pFileTimePoint);
		pFileTimePoint = NULL;
	}
	return nRet;
}

static int convert_date_hw(char* lpOutDateTime, const char* lpInTime, const char* lpInDate)
{
	/************************************************************************/
	/* lpInDate: 20100520 */
	/* lpInTime: 1:55 */
	/* lpOutDateTime: 20100520015500 */
	/************************************************************************/
	if ( 4 == strlen(lpInTime)) //1:55
	{
		sprintf(lpOutDateTime,"%s0%c%c%c00", 
			lpInDate, lpInTime[0], lpInTime[2], lpInTime[3]);
	}
	else if (5 == strlen(lpInTime)) //12:14
	{
		sprintf(lpOutDateTime,"%s%c%c%c%c00", 
			lpInDate, lpInTime[0], lpInTime[1], lpInTime[3], lpInTime[4]);
	}
	else
	{
        err_log("convert_date_hw: time format incorrect: %s\n", lpInTime);
        return 1;
	}
	return 0;
}

static int convert_date_hw_sp6(char* lpOutDateTime, char* lpInTime, const char* lpInDate)
{
	/************************************************************************/
	/* lpInDate: 20100520 */
	/* lpInTime: 09:07AM 12:45AM 12:45PM*/
	/* lpOutDateTime: 20100520015500 */
	/************************************************************************/
	if ('1' == lpInTime[0] && '2' == lpInTime[1] )
	{
		//12�� Ϊ 0��
		lpInTime[0] = '0';
		lpInTime[1] = '0';
	}
	if ('P' == lpInTime[5])
	{
		//����
		//ʱ�� + 12
		lpInTime[0] += 1; 
		lpInTime[1] += 2;
		if (lpInTime[1] > '9')
		{
			lpInTime[0] += 1;
			lpInTime[1] -= 10;
		}
	}
	sprintf(lpOutDateTime, "%s%c%c%c%c00", 
		lpInDate, lpInTime[0], lpInTime[1], lpInTime[3], lpInTime[4]);

	return 0;
}

static int convert_date_nsn(char* lpOutDateTime, const char* lpInTime, const char* lpInDate)
{
/************************************************************************/
/* lpInDate: 29.08.2005 */
/* lpInTime: 13.58:57 */
/* lpOutDateTime: 20100520015500 */
/************************************************************************/
	sprintf(lpOutDateTime,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 
		lpInDate[6],lpInDate[7],lpInDate[8],lpInDate[9],lpInDate[3],lpInDate[4],lpInDate[0],lpInDate[1],
		lpInTime[0], lpInTime[1], lpInTime[3],lpInTime[4], lpInTime[6], lpInTime[7]);
	return 0;
}

static int get_file_passive(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long * file_size)
{
	int  ret;
	int  ftp_ret;
	long succ_bytes;
	char tmp_file_name[MAX_LONG_FILENAME];
	
	ret=0;

    /* ��������ڱ����Ѿ����ڣ���ɾ����*/
	sprintf(tmp_file_name,"%s/%03d_%s",WORK_DIR,p_collect_conf->current_process_number,remote_file_name);
	if( 0 == access(tmp_file_name, F_OK) )
	{
		if(0 != unlink(tmp_file_name))
		{
			err_log("get_file_passive: unlink %s fail\n",tmp_file_name);
			ret = 1;		
			goto Exit_Pro;
		}
	}

	/* ����Ftp���� */
	ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,p_collect_conf->port,FTP_TIME_OUT,1,1,debug);
	if(ftp_ret!=0)
	{
		err_log("get_file_passive: Ftp_Init fail,collect_point=%d\n",p_collect_conf->collect_point);
		ret=1;
		goto Exit_Pro;
	}

	/* ���ָ����Զ��Ŀ¼������ת����Ŀ¼ */
	if(remote_path != NULL && strlen(remote_path) > 0)
	{
		ftp_ret = Ftp_Cd(remote_path);
		if(ftp_ret!=0)
		{
			err_log("get_file_passive: Ftp_Cd fail,collect_point=%d,dir=%s\n",p_collect_conf->collect_point,remote_path);
			ret=1;
			goto Exit_Pro;
		}
	}

	/* Ftp_Receive() */
	succ_bytes=0;
	ftp_ret = Ftp_Receive(remote_file_name, tmp_file_name, 0, &succ_bytes);
	if(ftp_ret!=0)
	{
		err_log("get_file_passive: Ftp_Receive fail,collect_point=%d,file=%s\n",p_collect_conf->collect_point,remote_file_name);
		ret=1;
		goto Exit_Pro;
	}
	
	*file_size=succ_bytes;

Exit_Pro:
	Ftp_Close();
	return ret;
}
static int get_file_port(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long * file_size)
{
	int  ret;
	int  ftp_ret;
	long succ_bytes;
	char tmp_file_name[MAX_LONG_FILENAME];
	
	ret=0;

    /* ��������ڱ����Ѿ����ڣ���ɾ����*/
	sprintf(tmp_file_name,"%s/%03d_%s",WORK_DIR,p_collect_conf->current_process_number,remote_file_name);
	if( 0 == access(tmp_file_name, F_OK) )
	{
		if(0 != unlink(tmp_file_name))
		{
			err_log("get_file_port: unlink %s fail\n",tmp_file_name);
			ret = 1;		
			goto Exit_Pro;
		}
	}

	/* ����Ftp���� */
	ftp_ret=Ftp_Init(p_collect_conf->usr,p_collect_conf->password,p_collect_conf->ip,p_collect_conf->port,FTP_TIME_OUT,1,0,debug);
	if(ftp_ret!=0)
	{
		err_log("get_file_port: Ftp_Init fail,collect_point=%d\n",p_collect_conf->collect_point);
		ret=1;
		goto Exit_Pro;
	}

	/* ���ָ����Զ��Ŀ¼������ת����Ŀ¼ */
	if(remote_path != NULL && strlen(remote_path) > 0)
	{
		ftp_ret = Ftp_Cd(remote_path);
		if(ftp_ret!=0)
		{
			err_log("get_file_port: Ftp_Cd fail,collect_point=%d,dir=%s\n",p_collect_conf->collect_point,remote_path);
			ret=1;
			goto Exit_Pro;
		}
	}

	/* Ftp_Receive() */
	succ_bytes=0;
	ftp_ret = Ftp_Receive(remote_file_name, tmp_file_name, 0, &succ_bytes);
	if(ftp_ret!=0)
	{
		err_log("get_file_port: Ftp_Receive fail,collect_point=%d,file=%s\n",p_collect_conf->collect_point,remote_file_name);
		ret=1;
		goto Exit_Pro;
	}
	
	*file_size=succ_bytes;

Exit_Pro:
	Ftp_Close();
	return ret;
}

static int backup_file(collect_conf * pCollectConf, const char * szRemoteFileName, const char* lpTimeStamp)
{
	char szBackupName[MAX_LONG_FILENAME];
	char szTempFileName[MAX_LONG_FILENAME];
    char szCommand[MAX_LONG_BUFFER];
	
	/*�ж��Ƿ񱸷�*/ 
	if(pCollectConf->is_backup)
	{
		sprintf(szTempFileName,"%s/%03d_%s",WORK_DIR,pCollectConf->current_process_number,szRemoteFileName);
		
		memset(szBackupName,0,sizeof(szBackupName));
		if(get_backup_name(pCollectConf,szRemoteFileName,lpTimeStamp,szBackupName)!=0)
		{
			err_log("backup_file: get_backup_name fail\n");
            return 1;
		}

        /* Ŀ���ļ������ļ��Ѿ����ڣ����������force_update��־��
         * ����ɾ���ٱ��ݣ������Թ������� */
        if(access(szBackupName, F_OK) == 0)
        {
            if(force_update)
            {
                /* ɾ���ɵ��ļ� */
                if(unlink(szBackupName) != 0)
                {
                    err_log("backup_file: unlink old backup file fail: %s\n", szBackupName);
                    return 1;
                }
            }
            else
            {
                //err_log("backup file: old backup file already exist: %s\n", szBackupName);
                return 0;
            }
        }

        sprintf(szCommand, "cp %s %s", szTempFileName, szBackupName);
        if(system(szCommand) == -1)
        {
			err_log("backup_file: link %s to %s fail\n", szTempFileName, szBackupName);
            return 1;
        }
	} 	

	return 0;
}

static int get_backup_name(collect_conf * pCollectConf,const char * szRemoteFileName,const char* lpTimeStamp,char * szBackupName)
{
	char   szTemp[MAX_LONG_FILENAME];

	sprintf(szTemp,"%s/%s/%s",pCollectConf->backup_path,pCollectConf->company,pCollectConf->device);
	sprintf(szBackupName,"%s/%06d_%s_%s_%s_%s",szTemp,pCollectConf->collect_point,pCollectConf->company,pCollectConf->device,lpTimeStamp,szRemoteFileName);
	if(access(szTemp,F_OK) == -1)
	{
		if(mkdir(szTemp,0755)!=0)
		{
			err_log("get_backup_name: mkdir %s fail\n",szTemp);
            return 1;
		}
	}
	return 0;
}

static int commit_file(collect_conf * p_collect_conf, const char * remote_file_name, const char* lpTimeStamp)
{
	char  commit_name[MAX_FILENAME];
	char  tmp_file_name[MAX_LONG_FILENAME];
	char  tmp_file_name_2[MAX_LONG_FILENAME];
    char  szCommand[MAX_LONG_BUFFER];

    //�ж��Ƿ��ύ
    if(p_collect_conf->is_commit) 
    {
        //link����Ŀ¼�ļ����ύĿ¼
        memset(commit_name,0,sizeof(commit_name));
		sprintf(commit_name,"@%06d_%s_%s_%s_%s",p_collect_conf->collect_point,p_collect_conf->company,p_collect_conf->device,lpTimeStamp,remote_file_name);

		sprintf(tmp_file_name,"%s/%03d_%s",WORK_DIR,p_collect_conf->current_process_number,remote_file_name);
		sprintf(tmp_file_name_2,"%s/%s",collect_dir,commit_name);
  
        /* ���Ŀ���ļ��Ѿ����ڣ���������force_update��־��
         * ����ɾ���ɵ��ļ����ύ��������Թ����ύ */
        if(access(tmp_file_name_2, F_OK) == 0)
        {
            if(force_update)
            {
                if(unlink(tmp_file_name_2) != 0)
                {
                    err_log("commit_file: unlink old file fail: %s\n", tmp_file_name_2);
                    return 1;
                }
            }
            else
            {
                //err_log("commit_file: target file %s exist\n",tmp_file_name_2);
                return 0;
            }
        }

        /* ����ļ���gz��ͷ,����н�ѹ */
        if( 'g' == remote_file_name[0] && 'z' == remote_file_name[1] )
        {
            sprintf(szCommand, "gzip -S .dat -d %s", tmp_file_name);
            if(system(szCommand) == -1)
            {
                err_log("commit_file: uncompress %s fail\n", tmp_file_name);
                return 1;
            }
            tmp_file_name[strlen(tmp_file_name) - 4] = '\0';
        }
 
        /* �ύ�ļ� */
        if(link(tmp_file_name,tmp_file_name_2)!=0)
        {
            err_log("commit_file: link %s to %s fail\n",tmp_file_name,tmp_file_name_2);
            return 1;
        }
    }

	return 0;
}

//��¼collect_run.{ne_name}.{YYYYMMDD}�ļ�
static int run_log(collect_conf * p_collect_conf, const char * remote_path, const char * remote_file_name, long file_size, const char* lpTimeStamp)
{
	char  local_file_name[MAX_FILENAME];
	char  tmp_file_name[MAX_LONG_FILENAME];
	char  run_log_time[MAX_TIME];
	char  commit_time[MAX_TIME];
	FILE* fp = NULL;

	get_time(run_log_time);
	get_time(commit_time);
	run_log_time[8]='\0';
	
    sprintf(local_file_name,"%06d_%s_%s_%s_%s",p_collect_conf->collect_point,p_collect_conf->company,p_collect_conf->device,lpTimeStamp,remote_file_name);
	sprintf(tmp_file_name,"%s.%s.%s",PREFIX_RUN_LOG_FILE,p_collect_conf->device,run_log_time);
	
	fp=fopen(tmp_file_name,"a+");
	if(fp==NULL)
	{
		err_log("run_log: fopen %s fail\n",tmp_file_name);
        return 1;
	}
	if(remote_path != NULL && strlen(remote_path) > 0)
	{
		fprintf(fp, "<%06d> %s %s %s %s %s %ld %s %s %d %d\n",
                p_collect_conf->collect_point,
                p_collect_conf->device,
                p_collect_conf->ip,
                remote_path,
			    remote_file_name,
                local_file_name,
                file_size,
                lpTimeStamp,
                commit_time,
                p_collect_conf->is_backup,
                p_collect_conf->is_commit);
	}
	else
	{
		fprintf(fp, "<%06d> %s %s . %s %s %ld %s %s %d %d\n",
                p_collect_conf->collect_point,
                p_collect_conf->device,
                p_collect_conf->ip,
			    remote_file_name,
                local_file_name,
                file_size,
                lpTimeStamp,
                commit_time,
                p_collect_conf->is_backup,
                p_collect_conf->is_commit);
	}
	fclose(fp);
	fp=NULL;
    return 0;
}

/************************************************************************/
/* �鵵�ɼ����� */
/************************************************************************/
/*
* archive_collect_conf()
*                      return 1 fail, 0 success
*/
int archive_collect_conf(void)
{
    char   cur_time[MAX_TIME];
    char   config_file[MAX_FILENAME];
    char   archive_file[MAX_FILENAME];

    /* ��ȡ��ǰʱ�� */
    get_time(cur_time);

    /* �Ե�ǰʱ��Ϊ��׺���ɹ鵵�ļ��� */
    sprintf(config_file, "%s/%s", CONF_DIR, COLLECT_CONF);
    sprintf(archive_file, "%s/%s.%s", ARCHIVE_DIR, COLLECT_CONF, cur_time);

    /* �鵵�ļ� */
    if(access(config_file, F_OK) == 0)
    {
        if(access(ARCHIVE_DIR, F_OK) == -1)
        {
            if(mkdir(ARCHIVE_DIR, 0755) != 0)
            {
                err_log("archive_collect_conf: mkdir %s fail\n", ARCHIVE_DIR);
                return 1;
            }
        }

        if(rename(config_file, archive_file) != 0)
        {
            err_log("archive_collect_conf: archive %s fail\n", config_file);
            return 1;
        }
    }
    
    return 0;
}

