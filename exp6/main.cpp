#include "dp.h"
#include <iostream>

int main(int argc, char *argv[]) {
    int limit; 
 	if (argv[1] != NULL) limit = atoi(argv[1]);
 	else limit = 5;
    OneWay oneWay(limit); // 最大十辆车
    int pid = fork();
    while (pid != 0) { //创建子进程
        sleep(rand() % 5);
        pid = fork();
    }
    srand(time(NULL));
    int direct = rand() % 2; //决定东西方向
    
    oneWay.Arrive(direct,limit); //进入
    oneWay.Cross(direct); //通过
    oneWay.Quit(direct); //离开
    return EXIT_SUCCESS;
}

