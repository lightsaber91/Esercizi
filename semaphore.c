#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>

union semun {
  int val;
	struct semid_ds *buf;
	unsigned short *array;
};

int bsem_init_busy(int bsemID)
{
	union semun arg;
	arg.val = 0;
	return semctl(bsemID, 0, SETVAL, arg);
}

int bsem_init_free(int bsemID)
{
	union semun arg;
	arg.val = 1;
	return semctl(bsemID, 0, SETVAL, arg);
}

int bsem_get(bsemID)
{
	struct sembuf sops;
	sops.sem_num = 0;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	while(semop(bsemID, &sops, 1) == -1) {
		if(errno != EINTR)
			return -1;
	}
	return 0;
}

int bsem_put(int bsemID)
{
	struct sembuf sops;
	sops.sem_num = 0;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	return semop(bsemID, &sops, 1);
}
