#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main(int argc, char *argv[]) {
	int pid;

	printf("Starting program; process has pid %d\n", getpid());

	FILE *file;
	file = fopen("fork-output.txt", "w");
	fprintf(file, "BEFORE FORK\n");
	fflush(file);

	int p[2] = {4,5};
	pipe(p);


	if ((pid = fork()) < 0) {
		fprintf(stderr, "Could not fork()");
		exit(1);
	}

	/* BEGIN SECTION A */
	fprintf(file, "SECTION A\n");
	fflush(file);
	printf("Section A;  pid %d\n", getpid());
//	sleep(30);

	/* END SECTION A */
	if (pid == 0) {
		/* BEGIN SECTION B */
	        fprintf(file, "SECTION B\n");
	        fflush(file);

		close(p[0]);
		FILE *tmp = fdopen(p[1], "w");
		fputs("hello from Section B\n", tmp);

		printf("Section B\n");
//		sleep(30);
//		sleep(30);
//		printf("Section B done sleeping\n");

        	char *newenviron[] = { NULL };


        	if (argc <= 1) {
                	printf("No program to exec.  Exiting...\n");
        	        exit(0);
	        }

//        	printf("Program \"%s\" has pid %d. Sleeping.\n", argv[0], getpid());
//        	sleep(30);

        	printf("Running exec of \"%s\"\n", argv[1]);

		dup2(1, fileno(file));

	       	execve(argv[1], &argv[1], newenviron);
	        printf("End of program \"%s\".\n", argv[0]);

		exit(0);

		/* END SECTION B */
	} else {
		/* BEGIN SECTION C */
	        fprintf(file, "SECTION C\n");
	        fflush(file);

		printf("Section C\n");
                int status;
                wait(&status);

		close(p[1]);
		FILE *tmp = fdopen(p[0], "r");
		char str[50];
		fgets (str, 50, tmp);
		puts(str);
//		sleep(30);
//		printf("Section C done sleeping\n");

		exit(0);

		/* END SECTION C */
	}
	/* BEGIN SECTION D */

        fprintf(file, "SECTION D\n");
        fflush(file);
	printf("Section D\n");
	sleep(30);

	/* END SECTION D */
}

