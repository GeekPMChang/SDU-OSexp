#include "pctl.h"

int main(int argc, char *argv[]) {
    int i; 
    int pid1, pid2;
    int status1, status2;
    signal(SIGINT,(sighandler_t)sigcat);
    char *arg1[] = {"/bin/ls","-a", NULL};
    char *arg2[] = {"/bin/ps","-a", NULL};

    for (i = 0; i < 1; i++) 
    {
        pid1 = fork();
        if (pid1 < 0)
        {
            printf("Create Process child1 fail!\n"); 
            exit(EXIT_FAILURE);
        }
        if (pid1 == 0)//child1
        {      
            pause();
            printf("I'm the child1 process %d \n", getpid());
			status1 = execve(arg1[0], &arg1[0], NULL);
        }
        else//father
        {
            pid2 = fork();
            if (pid2 < 0)
            {
                printf("Create Process child2 fail!\n"); 
                exit(EXIT_FAILURE);
            }
            if (pid2 == 0)//child2
            {
                printf("I'm the child2 process %d \n", getpid());
			    status2 = execve(arg2[0], &arg2[0], NULL);
            }
            else//father
			{
				waitpid(pid2, &status2, 0);
				if (kill(pid1, SIGINT) >= 0)
					waitpid(pid1, &status1, 0);
			}
        }
        sleep(3);
    }
    return EXIT_SUCCESS; 
}