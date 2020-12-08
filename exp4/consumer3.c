#include "ipc.h"

int main(int argc, char *argv[]) {
 	int rate;
 	//可在在命令⾏第⼀参数指定⼀个进程睡眠秒数，以调解进程执⾏速度
 	if (argv[1] != NULL) rate = atoi(argv[1]);
 	else rate = 3; //不指定为 3 秒
 	 
	//共享内存 使⽤的变量
 	buff_key = 101; //缓冲区任给的键值
 	buff_num = 1; //缓冲区任给的⻓度
 	cget_key = 103; //消费者取产品指针的键值
 	cget_num = 1; //指针数
 	shm_flg = IPC_CREAT | 0644; //共享内存读写权限
 	
	//获取缓冲区使⽤的共享内存，buff_ptr 指向缓冲区⾸地址
 	buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
 	
	//获取消费者取产品指针，cget_ptr 指向索引地址
 	cget_ptr = (int *)set_shm(cget_key, cget_num, shm_flg);
 	
	//信号量使⽤的变量
 	prod_key = 201; //⽣产者同步信号灯键值
 	pmtx_key = 202; //⽣产者互斥信号灯键值
 	cons3_key = 303; //消费者同步信号灯键值
 	sem_flg = IPC_CREAT | 0644; //信号灯操作权限
 	
	//⽣产者同步信号灯初值设为缓冲区最⼤可⽤量
 	sem_val = buff_num;
 	//获取⽣产者同步信号灯，引⽤标识存 prod_sem
 	prod_sem = set_sem(prod_key, sem_val, sem_flg);
 	
	//消费者初始⽆产品可取，同步信号灯初值设为 0
 	sem_val = 0;
 	//获取消费者同步信号灯，引⽤标识存 cons3_sem
 	cons3_sem = set_sem(cons3_key, sem_val, sem_flg);
 	
	//循环执⾏模拟消费者不断取产品
 	while (1) {
 		//如果⽆产品，消费者阻塞
 		down(cons3_sem);
 		
		//⽤读⼀字符的形式模拟消费者取产品，报告本进程号和获取的字符及读取的位置
 		sleep(rate);
		printf("%d consumer(have tabacco):now get glue and paper\n", getpid());

 		//唤醒阻塞的⽣产者
 		up(prod_sem);
 	}
 	return EXIT_SUCCESS;
}