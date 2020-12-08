#include "ipc.h"

int main(int argc, char *argv[]) {
	int rate; 
	//可在在命令⾏第⼀参数指定⼀个进程睡眠秒数，以调解进程执⾏速度
 	if (argv[1] != NULL) rate = atoi(argv[1]);
 	else rate = 3; //不指定则默认为 3 秒

	//共享内存使⽤的变量
 	buff_key = 101;//缓冲区任给的键值
 	buff_num = 1;//缓冲区任给的⻓度
 	pput_key = 102;//⽣产者放产品指针的键值
 	pput_num = 1; //指针数
 	shm_flg = IPC_CREAT | 0644;//共享内存读写权限
 	
	//获取缓冲区使⽤的共享内存，buff_ptr 指向缓冲区⾸地址
 	buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
 	
	//获取⽣产者放产品位置指针 pput_ptr
 	pput_ptr = (int *)set_shm(pput_key, pput_num, shm_flg);
 	
	//信号量使⽤的变量
 	prod_key = 201;//⽣产者同步信号灯键值
 	pmtx_key = 202;//⽣产者互斥信号灯键值
 	cons1_key = 301;//消费者1同步信号灯键值
	cons2_key = 302;//消费者2同步信号灯键值
	cons3_key = 303;//消费者3同步信号灯键值
 	sem_flg = IPC_CREAT | 0644;
 	
	//⽣产者同步信号灯初值设为缓冲区的容量，这里设置为1即可
	sem_val = buff_num;
 	//prod_sem----获取生产者同步信号灯
 	prod_sem = set_sem(prod_key, sem_val, sem_flg);
 	
	//⽣产者互斥信号灯初值为 1
 	sem_val = 1;
 	//pmtx_sem----获取生产者互斥信号灯
 	pmtx_sem = set_sem(pmtx_key, sem_val, sem_flg);
	
	//由于开始状态缓冲区无存量，三个消费的同步信号灯初值都设为 0
 	sem_val = 0;
 	//获取三个消费者的同步信号cons1,cons2,cons3
 	cons1_sem = set_sem(cons1_key, sem_val, sem_flg);
	cons2_sem = set_sem(cons2_key, sem_val, sem_flg);
	cons3_sem = set_sem(cons3_key, sem_val, sem_flg);
	
	while(1)
	{
		//如果缓冲区满则生产者阻塞----生产者的同步信号prod_sem
		down(prod_sem);
		//如果另一生产者正在放产品，本生产者阻塞---生产者的互斥信号pmtx_sem
		down(pmtx_sem);
		
		buff_ptr[*pput_ptr] = 'A' + rand()%3;

		if(buff_ptr[*pput_ptr] == 'A'){
			printf("The producer(%d) gives tobacco and paper\n",getpid());
			//唤醒阻塞的生产者----生产者的互斥信号pmtx_sem
			up(pmtx_sem);
			//唤醒一号消费者----消费者1的同步信号cons1_sem
			up(cons1_sem);
		}
		if(buff_ptr[*pput_ptr] == 'B'){
			printf("The producer(%d) gives tobacco and glue\n",getpid());
			//唤醒阻塞的生产者----生产者的互斥信号pmtx_sem
			up(pmtx_sem);
			//唤醒二号消费者----消费者2的同步信号cons2_sem
			up(cons2_sem);
		}
		if(buff_ptr[*pput_ptr] == 'C'){
			printf("The producer(%d) gives glue and paper\n",getpid());
			//唤醒阻塞的生产者----生产者的互斥信号pmtx_sem
			up(pmtx_sem);
			//唤醒三号消费者----消费者3的同步信号cons3_sem
			up(cons3_sem);
		}
		sleep(rate);
		
	}
 	return EXIT_SUCCESS;
}