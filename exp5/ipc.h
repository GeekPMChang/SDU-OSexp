#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>
#define SOFA 1
#define WAIT 2
#define PAID 3

typedef union semuns
{
    int val;
} Sem_uns;

typedef struct msgbuf
{
    long mtype;
    int mid;
} Msg_buf;

key_t buff_key;
int buff_num;
char *buff_ptr;
int shm_flg;

int Request_flag;
key_t Request_key;
int quest_id;

int respond_flg;
key_t respond_key;
int respond_id;

int get_ipc_id (char *proc_file, key_t key);
char *set_shm(key_t shm_key, int shm_num, int shm_flag);
int set_msq(key_t msq_key, int msq_flag);
int set_sem(key_t sem_key, int sem_val, int sem_flag);
int down (int sem_id);
int up (int sem_id);
int sem_flg;

key_t AccountSem_key;
int AccountSem_val;
int AccountSem_sem;

key_t CustomerSem_key;
int CustomerSem_val;
int CustomerSem_sem;

int Queue_flag;

key_t SofaQueue_key;
int SofaQueue_id;

key_t WaitQueue_key;
int WaitQueue_id;

key_t AccountQueue_key;
int AccountQueue_id;