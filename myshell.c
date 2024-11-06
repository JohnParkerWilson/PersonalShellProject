#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define BUFFER_SIZE 50
static char buffer[BUFFER_SIZE];
 
/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 */
void setup(char inputBuffer[], char *args[],int *background, char *ent, int recall)
{
	int length, /* # of characters in the command line */
    	i,  	/* loop index for accessing inputBuffer array */
    	start,  /* index where beginning of next command parameter is */
    	ct; 	/* index of where to place the next parameter into args[] */
   
	ct = 0;
	/* read what the user enters on the command line */
	if (recall != 1) {
   	 length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
   	 strcpy(ent, inputBuffer);
	}
	else {
   	 length = strlen(inputBuffer);
	}
	start = -1;
	if (length == 0)
    	exit(0);        	/* ^d was entered, end of user command stream */
	if (length < 0){
    	perror("error reading the command");
    	exit(-1);       	/* terminate with error code of -1 */
	}
	/* examine every character in the inputBuffer */
	for (i=0;i<length;i++) {
    	switch (inputBuffer[i]){
      	case ' ':
      	case '\t' :           	/* argument separators */
        	if(start != -1){
                	args[ct] = &inputBuffer[start];	/* set up pointer */
            	ct++;
        	}
        	inputBuffer[i] = '\0'; /* add a null char; make a C string */
        	start = -1;
        	break;
      	case '\n':             	/* should be the final char examined */
        	if (start != -1){
                	args[ct] = &inputBuffer[start];    
            	ct++;
        	}
            	inputBuffer[i] = '\0';
            	args[ct] = NULL; /* no more arguments to this command */
        	break;
      	default :         	/* some other character */
        	if (start == -1)
            	start = i;
        	if (inputBuffer[i] == '&'){
            	*background  = 1;
            	start = -1;
            	inputBuffer[i] = '\0';
        	}
      	}
 	}   
 	args[ct] = NULL; /* just in case the input line was > 80 */
}

void yellWord(char *word) { //yelling function
	int x = 0;
	while (word[x]) {
    	putchar(toupper(word[x]));
    	x++;
	}
}



void printHistory(char history[][MAX_LINE], int size) { //print history
    for (int i = 0; i < 10; i++) {
   	 if (i > size)
   		 i = 30;
   	 else
   		 printf("\n[%d]%s",i , history[i]);
    }
    fflush(stdout);
}

int main(void)
{
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	char enteredLine[MAX_LINE];
	int background; /* equals 1 if a command is followed by '&' */
	char *args[(MAX_LINE/2)+1];/* command line (of 80) has max of 40 arguments */
	int numOfPrompts = 0;
	int recallFlag = -1;
	char history[10][MAX_LINE];
	int histPos = 0;
	int size = 0;
	int lastKnown = -1;
	void handle_SIGTSTP() { //signal handle ctrl+z function
  	write(STDOUT_FILENO,buffer,strlen(buffer));
  	printHistory(history, size);
    }
	/* set up the signal handler */
	struct sigaction handler;
	handler.sa_handler = handle_SIGTSTP;
	handler.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &handler, NULL);
	strcpy(buffer,"Caught <ctrl><z>\n");
	/* wait for <control> <Z> */
    
	pid_t pid;
    pid_t childPID;
    pid_t initPID = getpid();
	printf("\nJohn Parker Wilson's Shell. PID is %d \n", initPID);
    
	while (1){ /* Program terminates normally inside setup */
    	background = 0;
    	printf("\nJWshell[%d]:\n", numOfPrompts);
   	 
    	if(recallFlag < 0) { //for any other command
   		 setup(inputBuffer,args,&background,enteredLine, 0); /* get next command */
   		 strcpy(history[histPos], enteredLine);
   	 	histPos++;
   	 	lastKnown++;
   	 	if (histPos >= 10) {
   			 histPos = 0;
   	 	}
   	 	if (lastKnown >= 10) {
   			 lastKnown = 0;
   	 	}
   	 	if (size < 10) {
   			 size++;
   	 	}
  		 }
    	else { //when a command is recalled
   		 setup(history[recallFlag],args,&background,enteredLine, 1);
   		 recallFlag = -1;
    	}
  	 
   	 
    	if(strcmp(args[0], "yell") == 0) { //yell command
        	for (int i = 1; i < (MAX_LINE/2)+1; i++){
            	if (args[i] == NULL) {
                	i = (MAX_LINE/2) + 1;
            	}
            	else {
                	yellWord(args[i]);
                	putchar(' ');
            	}
        	}
    	}
    	else if (strcmp(args[0], "r") == 0) { //recall command
   		 if (atoi(args[1]) < 10 && atoi(args[1]) >= 0)
   			 recallFlag = atoi(args[1]);
   		 else if (atoi(args[1]) >= size)
   			 printf("Error! invalid\n");
   		 else
   			 printf("Error! Must enter number between 0 and 9\n");
    	}
    	else if (strcmp(args[0], "exit") == 0) {//exit command
   		 char command[MAX_LINE];
   		 strcpy(command, "ps -o pid,ppid,pcpu,pmem,etime,user,command -p ");
   		 char numpid[MAX_LINE];
   		 sprintf(numpid, "%d", initPID);
   		 strncat(command, numpid, 10);
   		 system(command);
        	exit(0);
    	}
    	else if (args[0] != NULL){//for anything else or a process
       	 
    	/*fork a child process*/
   	 
    	pid = fork();
   	 childPID = getpid();
    	printf("\nChild Created [PID = %d], [BACKGROUND = %d]", pid, background);
    	if (pid < 0) { //fork failed
   		 fprintf(stderr, "\nFork Failed");
    	}
    	else if (pid == 0) { //child process
   		 execvp(args[0], args);
    	}
    	else { //parent process
        	if (background == 0) {
       		 waitpid(childPID, NULL, 0);
   			 printf("\nChild Complete");
   		 }
    	}
  	 }
    	else {
   	 
    	}
   	 
    	numOfPrompts++;
    	/* the steps are:
    	(0) if built-in command, handle internally
    	(1) if not, fork a child process using fork()
    	(2) the child process will invoke execvp()
    	(3) if background == 0, the parent will wait,
    	otherwise returns to the setup() function. */
	}
}

