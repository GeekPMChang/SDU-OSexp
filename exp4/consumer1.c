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
 	cons1_key = 301; //消费者同步信号灯键值
 	sem_flg = IPC_CREAT | 0644; //信号灯操作权限
 	
	//⽣产者同步信号灯初值设为缓冲区大小，这里设置为1即可
 	sem_val = buff_num;
 	//prod_sem----⽣产者同步信号灯
 	prod_sem = set_sem(prod_key, sem_val, sem_flg);
 	
	//消费者初始⽆产品可取，同步信号灯初值设为 0
 	sem_val = 0;
 	//cons1_sem----消费者1同步信号灯
 	cons1_sem = set_sem(cons1_key, sem_val, sem_flg);
 	
	//循环执⾏模拟消费者不断取产品
 	while (1) {
 		//如果自己的信号灯为0，则阻塞
		down(cons1_sem);
		
		//模拟消费者取产品，报告本进程的进程号和获取的字符
 		sleep(rate);
		printf("%d consumer(have glue):now get tobacco and paper\n", getpid());
 	
 		//唤醒阻塞的⽣产者
 		up(prod_sem);
 	}
 	return EXIT_SUCCESS;
}