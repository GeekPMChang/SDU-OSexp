#include "ipc.h";
int main(int argc, char *argv[])
{
    int rate; 
	//可在在命令⾏第⼀参数指定⼀个进程睡眠秒数，以调解进程执⾏速度
 	if (argv[1] != NULL) rate = atoi(argv[1]);
 	else rate = 3;

    Msg_buf msg_arg;
    struct msqid_ds SofaMsg_buf;//用于返回当前的沙发消息队列情况
    struct msqid_ds WaitMsg_buf;//用于返回当前的等待区消息队列情况
    
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
    
    int CustomerNum = 1;
    while (1)
	{
        //IPC_STAT 为⾮破坏性读，从队列中读出⼀个 msgid_ds 结构填充缓冲 buf
        msgctl(SofaQueue_id, IPC_STAT, &SofaMsg_buf);

        if (SofaMsg_buf.msg_qnum < 4)//沙发没满，可能需要更新
        {
            Request_flag = IPC_NOWAIT; 
            //非阻塞方式接收消息
            if (msgrcv(WaitQueue_id, &msg_arg,sizeof(msg_arg), WAIT, Request_flag)>= 0)//沙发已经满：从等待区最前面拿一个人放入沙发
            {
                msg_arg.mtype = SOFA;
                printf("Customer %d enter Sofa from Wait Area.\n", msg_arg.mid);
                msgsnd(SofaQueue_id, &msg_arg, sizeof(msg_arg), IPC_NOWAIT);
            }
            else//沙发没满，顾客直接进入沙发
            {
                msg_arg.mtype = SOFA;
                msg_arg.mid = CustomerNum++;
                printf("Customer %d enter Sofa directly.\n", msg_arg.mid);
                msgsnd(SofaQueue_id, &msg_arg, sizeof(msg_arg),IPC_NOWAIT);
                sleep(rate);
            }
        }
        else
        {
            //IPC_STAT 为⾮破坏性读，从队列中读出⼀个 msgid_ds 结构填充缓冲 buf
            msgctl(WaitQueue_id, IPC_STAT, &WaitMsg_buf);

            if (WaitMsg_buf.msg_qnum < 13)//等待区没满，顾客直接进入等待区
            {
                printf("Customer %d enter the Wait Area.\n",CustomerNum);
                msg_arg.mtype = WAIT;
                msg_arg.mid = CustomerNum++;
                msgsnd(WaitQueue_id, &msg_arg, sizeof(msg_arg), IPC_NOWAIT);
                sleep(rate);
            }
            else//等待区满了,阻塞
            {
                printf("HairShop is full,Customer %d need go away...\n", CustomerNum);
                down(CustomerSem_sem);//使用信号量实现阻塞
                sleep(rate);
            }
        }
    }
    return EXIT_SUCCESS;
}
