#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define EXIT_FAILED  1
#define EXIT_SUCCESS 0

#define MAXBUF 256
#define PASS 	 \
	puts("PASS");\
	return 0
#define FAIL \
	puts("FAIL");\
	exit(EXIT_FAILED)

char *shname;

static char *
randstr(char *buf, int n) 
{
	for(int i = 0; i < n; ++i)
		buf[i] = "abcdefghijklmnopqrstuvwxyz"[rand() % 26];
	buf[n] = '\0';
	return buf;
}

void
writefile(char *name, char *data)
{
	unlink(name);
	int fd = open(name, O_CREAT|O_WRONLY|O_TRUNC, 0644);
	if(fd < 0) {
		fprintf(stderr, "testsh: could not write %s\n", name);
		exit(EXIT_FAILED);
	}
	if(write(fd, data, strlen(data)) != strlen(data)) {
		fprintf(stderr, "testsh: write failed\n");
		close(fd);
		exit(EXIT_FAILED);
	}
	close(fd);
}

void
readfile(char *name, char *data, int limit)
{
	data[0] = 0;
	int fd = open(name, O_RDONLY);
	if(fd < 0) {
		fprintf(stderr, "testsh: could not open %s\n", name);
		exit(EXIT_FAILED);
	}
	int n = read(fd, data, limit - 1);
	if(n < 0) {
		fprintf(stderr, "testsh: read %s failed\n", name);
		close(fd);
		exit(EXIT_FAILED);
	}
	close(fd);
	data[n] = 0;
}

int
test(char *cmd, char *expect, int tight, void (*do_to_child)(int))
{
	char infile[16], outfile[16];

	randstr(infile, 12);
	randstr(outfile, 12);

	writefile(infile, cmd);
	unlink(outfile);

	int pid = fork();
	if(pid < 0) {
		fprintf(stderr, "testsh: fork() failed\n");
		exit(EXIT_FAILED);
	}

	if(pid == 0) {
		close(0);
		if(open(infile, O_RDONLY) != 0){
			fprintf(stderr, "testsh: open != 0\n");
			exit(EXIT_FAILED);
		}
		close(1);
		if(open(outfile, O_CREAT|O_WRONLY|O_TRUNC, 0644) != 1) {
			fprintf(stderr, "testsh: open != 1\n");
			exit(EXIT_FAILED);
		}
		char *argv[2] = {shname, 0};
		execvp(shname, argv);
		fprintf(stderr, "testsh: exec %s faile\n", shname);
		exit(EXIT_FAILED);
	}

	if(do_to_child) {
		sleep(1);
		do_to_child(pid);
	}

	if(wait(0) != pid) {
		fprintf(stderr, "testsh: unexpected wait() return\n");
		exit(EXIT_FAILED);
	}

	char buf[MAXBUF];
	readfile(outfile, buf, MAXBUF);
	unlink(infile); 
	unlink(outfile);

	if(strstr(buf, expect)) {
		if(tight && strlen(buf) > strlen(expect) + 10) {
			fprintf(stderr, "testsh: saw expected output, but too much else as well\n");
			fprintf(stderr, "you: \n%s\n", buf);
			fprintf(stderr, "expected: \n%s\n", expect);
			return 1;
		}
		return 0;
	}
	fprintf(stderr, "you: \n%s\n", buf);
	fprintf(stderr, "expected: \n%s\n", expect);
	return 1;
}

// test a command with arguments.
int
t0()
{
	printf("simple echo: ");
	if(!test("echo hello goodbye\n", "hello goodbye", 1, 0)) {
		PASS;
	} else {
		FAIL;
	}
}

// test a command with arguments.
int
t1()
{
	printf("simple grep: ");
	if(!test("grep land testinput.txt\n", "to seek another land.", 1, 0)) {
		PASS;
	} else {
		FAIL;
	}
}

// test a command, then a newline, then another command
int
t2()
{
	printf("two commands: ");
	if(!test("echo hello\necho bye\n", "hello\nbye", 1, 0)) {
		PASS;
	} else {
		FAIL;
	}
}

// test output redirection: echo xxx > file
int
t3()
{
	printf("output redirection: ");

	char filename[16], data[16], cmd[MAXBUF];
	randstr(filename, 12);
	randstr(data, 12);

	sprintf(cmd, "echo %s > %s\n", data, filename);

	if(!test(cmd, "", 1, 0)) {
		char buf[MAXBUF];
		readfile(filename, buf, MAXBUF);
		unlink(filename);
		if(!strstr(buf, data)) {
			FAIL;
		} else {
			PASS;
		}
	}
	unlink(filename);
	FAIL;
}

// test input redirection: cat < file
int
t4()
{
	printf("input redirection: ");

	char filename[16], data[16], cmd[MAXBUF];
	randstr(filename, 12);
	randstr(data, 12);
	writefile(filename, data);

	sprintf(cmd, "cat < %s\n", filename);

	if(!test(cmd, data, 1, 0)) {
		unlink(filename);
		PASS;
	} else {
		unlink(filename);
		FAIL;
	}
}

// test a command with both input and output redirection.
int
t5()
{
	printf("both redirection: ");
	unlink("testsh.txt");
	if(!test("grep flower < testinput.txt > testsh.txt\n", "", 1, 0)) {
		char buf[MAXBUF];
		readfile("testsh.txt", buf, MAXBUF);
		unlink("testsh.txt");
		if(!strstr(buf, "If each day a flower climbs")) {
			FAIL;
		} else {
			PASS;
		}
	}
	unlink("testsh.txt");
	FAIL;
}

// test a command with pipe: cat file | cat.
int
t6()
{
	printf("simple pipe: ");

	char filename[16], data[16], cmd[MAXBUF];
	randstr(filename, 12);
	randstr(data, 12);
	writefile(filename, data);

	sprintf(cmd, "cat %s | cat\n", filename);

	if(!test(cmd, data, 1, 0)) {
		unlink(filename);
		PASS;
	} else {
		unlink(filename);
		FAIL;
	}
}

// test a pipeline taht has both redirection and a pipe.
int
t7()
{
	printf("pipe and redirections: ");

 	if(!test("grep me < testinput.txt | wc > testsh.txt\n", "", 1, 0)) {
 		char buf[MAXBUF];
 		readfile("testsh.txt", buf, MAXBUF);
 		unlink("testsh.txt");
 		if(!strstr(buf, "     12      70     365")) {
 			FAIL;
 		} else {
 			PASS;
 		}
 	} else {
 		unlink("testsh.txt");
 		FAIL;
 	}
}

// ask the shell to execute many commands, 
// to check if it leaks file descriptors.
int
t8() {
	printf("lots of commands: ");

	char term[32], cmd[MAXBUF * 4];
	randstr(term, 24);

	cmd[0] = 0;
	for(int i = 0; i < 17; ++i) {
		strcpy(cmd + strlen(cmd), "echo x < testinput.txt > tso\n");
    	strcpy(cmd + strlen(cmd), "echo x | echo\n");
	}
	strcpy(cmd + strlen(cmd), "echo ");
	strcpy(cmd + strlen(cmd), term);
	strcpy(cmd + strlen(cmd), " > tso\n");
	strcpy(cmd + strlen(cmd), "cat < tso\n");

	if(!test(cmd, term, 0, 0)) {
		unlink("tso");
		PASS;
	} else {
		unlink("tso");
		FAIL;
	}
}

int
t9()
{
	printf("two commands with multiple pipes: ");

	char filename[32], cmd[MAXBUF];
	randstr(filename, 20);

	sprintf(cmd, "echo hello > %s\nfind . -maxdepth 1 -name %s | xargs grep hello | wc\n", filename, filename);

	if (!test(cmd, "      1       1       6", 1, 0)) {
		unlink(filename);
		PASS;
	} else {
		unlink(filename);
		FAIL;
	}
}

// test a command that exec a endless prog,
// then send SIGINT
static void do_to_child(int pid) { kill(pid, SIGINT); }
int
t10()
{
	printf("signal SIGINT: ");

	char progc[20], cplcmd[40];
	randstr(progc, 10);
	sprintf(cplcmd, "gcc prog.c -o %s", progc);

	char term[32], cmd[MAXBUF];
	randstr(term, 24);
    sprintf(cmd, "./%s\necho %s\n", progc, term);

	writefile("prog.c", "int main(){while(1);}");
	system(cplcmd);
	unlink("prog.c");

	if(!test(cmd, term, 1, do_to_child)) {
		unlink(progc);
		PASS;
	} else {
		unlink(progc);
		FAIL;
	}
}

int 
main(int argc, char *argv[]) 
{
	if(argc < 2) {
		fprintf(stderr, "Usage: testsh msh\n");
		exit(0);
	}
	srand((unsigned) time(0));
	shname = argv[1];

	t0();
	t1();
	t2();
	t3();
	t4();
	t5();
	t6();
	t7();
	t8();
	t9();
	t10();

	printf("All tests passed :)\n");
	exit(EXIT_SUCCESS);
}
