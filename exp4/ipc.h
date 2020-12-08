#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define BUFSZ 256

//建⽴或获取 ipc 的⼀组函数的原型说明
int get_ipc_id(char *proc_file,key_t key);
char *set_shm(key_t shm_key,int shm_num,int shm_flag);
int set_msq(key_t msq_key,int msq_flag);
int set_sem(key_t sem_key,int sem_val,int sem_flag);
int down(int sem_id);
int up(int sem_id);

/*信号灯控制⽤的共同体*/
typedef union semuns {
 int val;
} Sem_uns;

/* 消 息 结 构 体 */
typedef struct msgbuf {
 long mtype;
 char mtext[1];
} Msg_buf;

//⽣产消费者共享缓冲区即其有关的变量
key_t buff_key;
int buff_num;
char *buff_ptr;

//⽣产者放产品位置的共享指针
key_t pput_key;
int pput_num;
int *pput_ptr;

//消费者取产品位置的共享指针
key_t cget_key;
int cget_num;
int *cget_ptr;

//⽣产者有关的信号量
key_t prod_key;
key_t pmtx_key;
int prod_sem;
int pmtx_sem;
int p_type1, p_type2, p_type3;
int all_product;

//消费者有关的信号量
key_t cons1_key;
key_t cmtx1_key;
key_t cons2_key;
key_t cmtx2_key;
key_t cons3_key;
key_t cmtx3_key;
int cons1_sem;
int cmtx1_sem;
int cons2_sem;
int cmtx2_sem;
int cons3_sem;
int cmtx3_sem;
int sem_val;
int sem_flg;
int shm_flg;