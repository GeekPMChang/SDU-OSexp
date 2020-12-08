#include "ipc.h"
int main (int argc, char *argv[])
{
    int rate; 
	//可在在命令⾏第⼀参数指定⼀个进程睡眠秒数，以调解进程执⾏速度
 	if (argv[1] != NULL) rate = atoi(argv[1]);
 	else rate = 3;

    Msg_buf msg_arg;
    struct msqid_ds SofaMsg_buf;//用于返回当前的沙发消息队列情况
    
    //顾客同步信号量
    sem_flg = IPC_CREAT | 0644;
    CustomerSem_key = 101;
    CustomerSem_val = 0;
    CustomerSem_sem = set_sem(CustomerSem_key, CustomerSem_val, sem_flg);
    
    //账本互斥信号量
    sem_flg = IPC_CREAT | 0644;
    AccountSem_key = 201;
    AccountSem_val = 1;
    AccountSem_sem = set_sem(AccountSem_key, AccountSem_val, sem_flg);

    //沙发消息队列
    Queue_flag = IPC_CREAT | 0644;
    SofaQueue_key = 301;
    SofaQueue_id = set_msq(SofaQueue_key, Queue_flag);
    
    //等候区消息队列
    Queue_flag = IPC_CREAT | 0644;
    WaitQueue_key = 401;
    WaitQueue_id = set_msq(WaitQueue_key, Queue_flag);

    //账本消息队列
    Queue_flag = IPC_CREAT | 0644;
    AccountQueue_key = 501;
    AccountQueue_id = set_msq(AccountQueue_key, Queue_flag);
    
    //三个理发师进程：本进程，两个子进程
    pid_t pid1,pid2;
    pid1 = fork();

    if (pid1 == 0)//理发师1
    {
        while (1)
        {
            //IPC_STAT 为⾮破坏性读，从队列中读出⼀个 msgid_ds 结构填充缓冲 buf
                msgctl(SofaQueue_id, IPC_STAT, &SofaMsg_buf);

                if (SofaMsg_buf.msg_qnum == 0)//保持睡觉
                    printf("Barber %d is Sleeping.\n", getpid());
                
                if(msgrcv(AccountQueue_id,&msg_arg,sizeof(msg_arg),PAID,IPC_NOWAIT)>0)
                {
                    down(AccountSem_sem);
                    printf("barber %d get money from customer %d.\n", getpid(), msg_arg.mid);
                    sleep(1);
                    up(AccountSem_sem);
                }
                
                msgrcv(SofaQueue_id, &msg_arg, sizeof(msg_arg), SOFA, 0);//从沙发拉一个剪发
                
                up(CustomerSem_sem);//唤醒顾客进程(让下一顾客坐入沙发)

                //剪头发的过程
                printf("Barber %d cut the hair for customer %d.\n", getpid(), msg_arg.mid);

                sleep(rate);

                msg_arg.mtype=PAID;
                msgsnd(AccountQueue_id,&msg_arg,sizeof(msg_arg),IPC_NOWAIT)
        }
    }
    else
    {
        pid2 = fork();
        if (pid2 == 0)//理发师2
        {
            while (1)
            {
                //IPC_STAT 为⾮破坏性读，从队列中读出⼀个 msgid_ds 结构填充缓冲 buf
                msgctl(SofaQueue_id, IPC_STAT, &SofaMsg_buf);

                if (SofaMsg_buf.msg_qnum == 0)//保持睡觉
                    printf("Barber %d is Sleeping.\n", getpid());
                
                if(msgrcv(AccountQueue_id,&msg_arg,sizeof(msg_arg),PAID,IPC_NOWAIT)>0)
                {
                    down(AccountSem_sem);
                    printf("barber %d get money from customer %d.\n", getpid(), msg_arg.mid);
                    sleep(1);
                    up(AccountSem_sem);
                }
                
                msgrcv(SofaQueue_id, &msg_arg, sizeof(msg_arg), SOFA, 0);//从沙发拉一个剪发
                
                up(CustomerSem_sem);//唤醒顾客进程(让下一顾客坐入沙发)

                //剪头发的过程
                printf("Barber %d cut the hair for customer %d.\n", getpid(), msg_arg.mid);

                sleep(rate);

                msg_arg.mtype=PAID;
                msgsnd(AccountQueue_id,&msg_arg,sizeof(msg_arg),IPC_NOWAIT)
            }
        }
        else//理发师3
        {
            while (1)
            {
                //IPC_STAT 为⾮破坏性读，从队列中读出⼀个 msgid_ds 结构填充缓冲 buf
                msgctl(SofaQueue_id, IPC_STAT, &SofaMsg_buf);

                if (SofaMsg_buf.msg_qnum == 0)//保持睡觉
                    printf("Barber %d is Sleeping.\n", getpid());
                
                if(msgrcv(AccountQueue_id,&msg_arg,sizeof(msg_arg),PAID,IPC_NOWAIT)>0)
                {
                    down(AccountSem_sem);
                    printf("barber %d get money from customer %d.\n", getpid(), msg_arg.mid);
                    sleep(1);
                    up(AccountSem_sem);
                }
                
                msgrcv(SofaQueue_id, &msg_arg, sizeof(msg_arg), SOFA, 0);//从沙发拉一个剪发
                
                up(CustomerSem_sem);//唤醒顾客进程(让下一顾客坐入沙发)

                //剪头发的过程
                printf("Barber %d cut the hair for customer %d.\n", getpid(), msg_arg.mid);

                sleep(rate);

                msg_arg.mtype=PAID;
                msgsnd(AccountQueue_id,&msg_arg,sizeof(msg_arg),IPC_NOWAIT)
            }
        }
    }
    return EXIT_SUCCESS;
}
