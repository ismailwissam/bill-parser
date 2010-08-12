#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include "common.h"
#include "semctl.h"

#define      SEMKEY                 878

int sem_init(void)
{
	int        sem_id;
	int        sem_in;

	/* 初始化信号量对象 */
	sem_id = semget(SEMKEY, 1, 0666|IPC_CREAT);
	if(sem_id == -1) {
		err_log("main: semget SEMKEY %d fail\n", SEMKEY);
        return 1;
	}

	sem_in = 1;

	if(semctl(sem_id, 0, SETVAL, &sem_in) == -1) {
		err_log("main: semctl SEMKEY %d fail\n", SEMKEY);
        return 1;
	}

    return 0;
}

void P(void)
{
	int    sem_id;
    struct sembuf sem_buff;
	
	sem_id = semget(SEMKEY, 1, 0);
	if(sem_id == -1)
	{
		err_log("P: semget SEMKEY %d fail\n", SEMKEY);
		exit(1);
	}
    /*-----------------------------------------------*/
    /*  semphore p oprate                            */
    /*-----------------------------------------------*/
	sem_buff.sem_num = 0;
	sem_buff.sem_op  = -1;
	sem_buff.sem_flg = SEM_UNDO;
	
	while(semop(sem_id, &sem_buff, 1) == -1)
	{
		if(errno == EINTR || errno == EAGAIN)
		{
			err_log("P: semop SEMKEY %d EINTR or EAGAIN errno=%d\n", SEMKEY, errno);
		}
		else
		{
			err_log("P: semop SEMKEY %d fail errno=%d\n", SEMKEY, errno);
			exit(1);
		}
	}
}

void V(void)
{
	int    sem_id;
    struct sembuf sem_buff;
	
	sem_id = semget(SEMKEY, 1, 0);
	if(sem_id == -1)
	{
		err_log("V: semget SEMKEY %d fail\n", SEMKEY);
		exit(1);
	}
    /*-----------------------------------------------*/
    /*  semphore v oprate                            */
    /*-----------------------------------------------*/
	sem_buff.sem_num = 0;
	sem_buff.sem_op  = 1;
	sem_buff.sem_flg = SEM_UNDO;

	while(semop(sem_id, &sem_buff, 1) == -1)
	{
		if(errno == EINTR || errno == EAGAIN)
		{
			err_log("V: semop SEMKEY %d EINTR or EAGAIN errno=%d\n", SEMKEY, errno);
		}
		else
		{
			err_log("V: semop SEMKEY %d fail errno=%d\n", SEMKEY, errno);
			exit(1);
		}
	}
}

