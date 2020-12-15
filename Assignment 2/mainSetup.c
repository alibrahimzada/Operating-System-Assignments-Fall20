#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
 
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
 
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[], int *background) {
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

    /* the signal interrupted the read system call */
    /* if the process is in the read() system call, read returns -1
       However, if this occurs, errno is set to EINTR. We can check this  value
       and disregard the -1 value */

    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	    exit(-1);           /* terminate with error code of -1 */
    }

	printf(">>%s<<",inputBuffer);
    for (i = 0; i < length; i++) { /* examine every character in the inputBuffer */

        switch (inputBuffer[i]) {
    	    case ' ':
	        case '\t' :               /* argument separators */
		        if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		            ct++;
		        }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
        		if (start != -1){
                    args[ct] = &inputBuffer[start];     
        		    ct++;
		        }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
        		break;

	        default :             /* some other character */
        		if (start == -1)
		            start = i;
                if (inputBuffer[i] == '&'){
		            *background  = 1;
                    inputBuffer[i-1] = '\0';
		        }
	    } /* end of switch */
    }    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++)
		printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

int isFileExists(const char *path) {
    // Check for file existence
    if (access(path, F_OK) == -1)
        return 0;

    return 1;
}

int main(void) {
    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */
    while (1) {
        background = 0;
        printf("myshell: ");
        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);
        
        /** the steps are:
        (1) fork a child process using fork()
        (2) the child process will invoke execv()
        (3) if background == 0, the parent will wait,
        otherwise it will invoke the setup() function again. */

        pid_t childpid;

        if ( (childpid = fork()) == -1 ) {   // this condition is to check if fork was successfull
            fprintf(stderr, "failed to fork!");
            continue;
        }
        
        if ( childpid == 0 ) {    // this condition is true when the processor schedules the child process
            char* path = getenv("PATH");   // this returns all of the dirs in PATH env variable
            if (path == NULL) {   // this condition is to check if PATH is empty
                fprintf(stderr, "getenv returned NULL");
                continue;
            }

            char *dir = strtok(path, ":");
            while(dir != NULL) {
                char * absPath = (char *) malloc(1 + strlen(dir) + strlen(args[0]) );
                strcpy(absPath, dir);
                char slash = '/';
                strncat(absPath, &slash, 1);
                strcat(absPath, args[0]);

                if (isFileExists(absPath)) {   // this condition checks if the given program exists in directory
                    execv(absPath, args);   // if so, execute execv() from child process
                    fprintf(stderr, "An error must have happened when running execv()!");
                    break;
                }

		        dir = strtok(NULL, ":");
	        }
        }

        if ( !background && wait(NULL) != childpid ) {   // needs some editing
            fprintf(stderr, "A signal must have interrupted the wait!");
        }
    }
}
