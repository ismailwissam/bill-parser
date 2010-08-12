
/*
 * FTP Static Librarie
 */

/*
int        return -1   fail,0 success.
Ftp_Pwd    return null fail,not null success.
Ftp_size   return -1   fail,>=0 success.
*/

#ifndef __Ftp_
#define __Ftp_

int  Ftp_Init(const char *user_name,const char *passwd,const 
              char *ip,int port,unsigned int time_out,
              int yesno_binary,int yesno_passive,int yesno_debug);
void Ftp_Close(void);

char *Ftp_Pwd(void);
int  Ftp_Cd(const char *directory);
int  Ftp_Cdup(void);

int  Ftp_Delete(const char *file_name);
int  Ftp_Rename(const char *from_name,const char *to_name);
int  Ftp_Mkdir(const char *dir_name);
int  Ftp_Rmdir(const char *dir_name);

int  Ftp_Dir(const char *outfile_name);
int  Ftp_Nlist(const char *outfile_name);

int  Ftp_Send(const char *local_name,const char *remote_name,
              off_t rpoint,long *suc_bytes);
int  Ftp_Receive(const char *remote_name,const char *local_name,
                 off_t rpoint,long *suc_bytes);

int  Ftp_Size(const char *remote_name);

#endif
