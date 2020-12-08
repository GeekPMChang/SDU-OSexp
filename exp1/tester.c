#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/wait.h>

void redirect(int k, int fd[2]) {
	close(k);
	dup(fd[k]);
	close(fd[0]);
	close(fd[1]);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Useage: tester ./<EXEC>\n");
		exit(-1);
	}
	int fd[2], pid;
	pipe(fd);
	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork failed\n");
		exit(-1);
	} else if(pid == 0) {
		close(2);
		dup(fd[1]);
		close(1);
		close(fd[0]);
		close(fd[1]);
		char *argvs[] = {
			"strace",
			"-f",
			"-F",
			"-e",
			"trace=file,process",
			argv[1],
			NULL
		};
		execvp(argvs[0], argvs);
		fprintf(stderr, "exec failed\n");
		exit(-1);
	} else {
		printf("test lab1: \n");
		redirect(0, fd);
		static char buf[1024];
		regmatch_t pmatch[4];
		const size_t nmatch = 4;
		const char * pattern = "\\[pid\\s*([0-9]*)\\]\\sexecve\\(\"(.*?)\"";
		const char * pattern2 = "clone\\(.*?\\)\\s=\\s([0-9]*)";
		regex_t reg, reg2;
		if (regcomp(&reg, pattern, REG_EXTENDED)) {
			fprintf(stderr, "regex compile failed\n");
			exit(-1);
		}
		if (regcomp(&reg2, pattern2, REG_EXTENDED)) {
			fprintf(stderr, "regex compile failed\n");
			exit(-1);
		}
		char fp[30] = {0}, sp[30] = {0};
		int cnt = 0;
		while(fgets(buf, 1024, stdin)) {
			if (!regexec(&reg2, buf, nmatch, pmatch, 0)) {
				buf[pmatch[1].rm_eo] = 0;
				char *p = buf + pmatch[1].rm_so;
				if (strlen(fp)) {
					strcpy(sp, p);
					printf("\tget second child: %s\n", sp);
				} else {
					strcpy(fp, p);
					printf("\tget first child: %s\n", fp);
				}
			}
			if (strlen(fp) && strlen(sp) && !regexec(&reg, buf, nmatch, pmatch, 0)) {
				buf[pmatch[1].rm_eo] = 0;
				buf[pmatch[2].rm_eo] = 0;
				char *p1 = buf + pmatch[1].rm_so;
				char *p2 = buf + pmatch[2].rm_so;
				if (cnt == 0) {
					if (strstr(p2, "ps") && !strcmp(p1, sp)) {
						printf("\t[pid %s] ps: \033[0;32mok\033[0m\n", sp);
						cnt ++;
					}
				} else if (cnt == 1) {
					if (strstr(p2, "ls") && !strcmp(p1, fp)) {
						printf("\t[pid %s] ls: \033[0;32mok\033[0m\n", fp);
						cnt ++;
					}
				}
			}
		}
		regfree(&reg);
		regfree(&reg2);
		wait(NULL);
		if (cnt == 2) {
			puts("\tlab1: \033[0;32mPASS\033[0m");
		} else {
			puts("\tlab1: \033[0;31mFAIL\033[0m");
		}
	}
	return 0;
}