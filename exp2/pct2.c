#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int f(int a){
	if(a == 1)return 1;
	else return f(a-1) * a;
}

int g(int b){
  	if(b == 1 || b == 2) return 1;
 	else return g(b-1) + g(b-2);
}

int main(int argc, char *argv[]) {
	int x, y;
	printf("Please enter x, y\n");
	scanf("%d, %d",&x, &y); 
	int pid1, pid2; //两个子进程f(x), f(y)的进程号
	int pipe1[2]; //存放第一个无名管道标号
	int pipe2[2]; //存放第二个无名管道标号
	int pipe3[2]; //存放第三个无名管道标号
	int pipe4[2]; //存放第四个无名管道标号	
	
	//使用pipe()系统调用建立四个无名管道。建立不成功程序退出,执行终止
	if(pipe(pipe1) < 0) {
		perror("pipe not create");
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe2) < 0) {
		perror("pipe not create");
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe3) < 0) {
		perror("pipe not create");
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe4) < 0) {
		perror("pipe not create");
		exit(EXIT_FAILURE);
	}
	
	//使用fork()系统调用建立子进程,建立不成功程序退出,执行终止
	if((pid1 = fork()) <0) {
		perror("process not create");
		exit(EXIT_FAILURE);
	}
	//子进程号等于 0 表示子进程在执行,
	else if(pid1 == 0) {
		int x1;
		//计算f(x)的子进程1负责从管道1的0端读 管道2的1端写，所以关掉管道1的1端 管道2的0端
		close(pipe1[1]);
		close(pipe2[0]);
		read(pipe1[0], &x1, sizeof(int));
		printf("received x from Parent(Child1 pid:%d): %d\n",getpid(), x1);
		x1 = f(x1);
		write(pipe2[1],&x1,sizeof(int));
		
		//读写完成后,关闭管道
		close(pipe1[0]);
		close(pipe2[1]);
		
		//子进程执行结束
		exit(0);
	}
	//子进程号大于0表示父进程在执行,
	else {
		if((pid2 = fork()) <0) {
			perror("process not create");
			exit(EXIT_FAILURE);
		}
		if (pid2 == 0) {
			int y1;
			//计算f(y)的子进程2负责从管道3的0端读 管道4的1端写，所以关掉管道3的1端 管道4的0端
			close(pipe3[1]);
			close(pipe4[0]);
			
			read(pipe3[0], &y1, sizeof(int));
			printf("received y from Parent(Child2 pid:%d): %d\n", getpid(), y1);
			y1 = g(y1);
			write(pipe4[1], &y1, sizeof(int));
			
			//读写完成后,关闭管道
			close(pipe3[0]);
			close(pipe4[1]);	
			//子进程执行结束
			exit(0);
		}
		else 
		{
			//父进程负责从管道2的0端读 管道1的1端写 管道3的0端读 管道4的1端写
			//所以关掉管道1的0端 管道2的1端 管道3的0端 管道4的1端
			close(pipe1[0]);	
			close(pipe2[1]);
			close(pipe3[0]);
			close(pipe4[1]);
		
			int z;
			write(pipe1[1], &x, sizeof(int));
			write(pipe3[1], &y, sizeof(int));
			read(pipe2[0], &x, sizeof(int));
			read(pipe4[0], &y, sizeof(int));
		
			z = x + y;
			printf("received f(x) of Child1(Parent pid:%d): %d\n", getpid(), x);
			printf("received f(y) of Child2(Parent pid:%d): %d\n", getpid(), y);
			printf("f(x,y) of Parent(Parent pid: %d): %d\n", getpid(), z);
		
			//读写完成后,关闭管道
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe3[1]);
			close(pipe4[0]);
		}
		
	}
	
	//父进程执行结束
	return 0;
}
