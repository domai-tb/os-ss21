/* TODO-List: 

    - show background processes:

    "jobs" zeigt aktuell nur den ersten Background Prozess an, nicht aber alle.
    Wenn man den Befehl mehrfach hintereinander ausführt, arbeitet er die 
    List ab.

    --> While Loop in plist.c 
    Rouven: ich habe in der jobs fkt. einen Teil auskommentiert, der mir sinnlos erscheint, jetzt geht jobs

	in der Aufgabe wird für delimit nur ' ' und '\t' angegeben. Ist also mehr als
	eigentlich muss. ~Zeile 252
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>	
#include <string.h>	
#include <stdbool.h>

#include ".clash/plist.h"


#define MAX_PATH_LENGTH 100
#define MAX_LINE_LENGTH 1337
#define BUFFER_SIZE 128
#define TOKEN_BUFFER_SIZE 10


/* Get working directory:

    Parameters:
    -

    Locals:

    - buffer:           buffer for getting the working dir
    - working_dir:      working dir

    Return:

    - If Fail:    EXIT_FAILURE
    - If Succes:  char* working_dir

*/
char* get_working_dir() 
{
    char buffer[MAX_PATH_LENGTH];  

    if (getcwd(buffer, sizeof(buffer)) == NULL) {
		fprintf(stderr, "Can't read working directory.\n");
		exit(EXIT_FAILURE);
	}

    char* working_dir = NULL;
    working_dir = (char*) malloc(strlen(buffer) * sizeof(char));
    if(working_dir == NULL) {
        fprintf(stderr, "Memory allocation goes wrong.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(working_dir, buffer);
    return working_dir;
}

/* Change Working directory:

    Parameters:
    
    - directory:        targeted directory

    Locals:
    -

    Return:

    - If Fail:          EXIT_FAILURE
    - If Success:       EXIT_SUCCESS

*/
int change_directory(char* directory)
{
    if(directory == NULL) {
        fprintf(stderr, "Syntax: cd <path>\n");
        return EXIT_FAILURE;
    } else {
        if(chdir(directory) != EXIT_SUCCESS) {
            fprintf(stderr, "Unable to change directory.");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}


/* Show all inputed commands:

    Parameters:
    
    - 

    Locals:
    -

    Return:

    - If Fail:          exit(EXIT_FAILURE)
    - If Success:       EXIT_SUCCESS

*/
int jobs(pid_t pid, const char* cmdline)
{
    int status;
    waitpid(pid, &status, WUNTRACED);
	printf("[%d]: '%s', Exisstatus: %d\n", pid, cmdline, status);

    /*size_t buffer_size = strlen(cmdline);
    char* command_line_buffer = (char*) malloc(buffer_size * sizeof(char));
    if(command_line_buffer == NULL) {
        fprintf(stderr, "Memory allocation goes wrong.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(command_line_buffer, cmdline);

    removeElement(pid, command_line_buffer, buffer_size);

    free(command_line_buffer);*/
    return EXIT_SUCCESS;
}

int show_background_processes()
{
    walkList(jobs);
    return EXIT_SUCCESS;
}


/* Try to find a '&' at the end of user input:

    Parameters:

    - line:             input line (from user)  

    Locals:

    -

    Return:

    - If Fail:
    - If Success:

*/
bool is_background(char* line)
{
    size_t length = strlen(line);

    if(line[length-1] == '&')
        return true;
    else 
        return false;  
}

/* Reading the command from user input:

    Parameters:
    -

    Locals:

    - command_line:     buffer for getting the user input
    - index:            array index of buffer
    - c:                character of user input
    - buffer_size:      max. size of buffer

    Return:    

    - If Fail:    EXIT_FAILURE
    - If Succes:  char* command_line
    - If EOF:     exit program

*/
char* read_command()
{
    int index = 0, c, buffer_size = BUFFER_SIZE;

    char* command_line = (char*) malloc(sizeof(char) * buffer_size);
    if(command_line == NULL) {
        fprintf(stderr, "Memory allocation goes wrong.\n");
        exit(EXIT_FAILURE);
    }

    while(true)
    {
        // Read a character
        c = getchar();

        // Replace '\n' with a null character and return.
        if(c == '\n') {
            command_line[index] = '\0';
            return command_line;
        } 
        // Exit at EOF
        else if(c == EOF) {
            fprintf(stdout, "exit\n");
            exit(EXIT_SUCCESS);
        }
        // write character in buffer
        else
            command_line[index] = c;
        index++;

        // Realloc, if buffer size exceeded
        if (index >= buffer_size && index <= MAX_LINE_LENGTH)
        {
            buffer_size += BUFFER_SIZE;
            command_line = (char*) realloc(command_line, buffer_size);
            if (command_line == NULL) {
                fprintf(stderr, "Memory allocation goes wrong.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


/* Parse parameters from input line:

    Parameters:

    - line:             input line (from user)

    Locals:

    - buffer_size:      size of buffer to write parameters
    - token:            single word from input line
    - delimit:          array of delimiters for strtok
    - type:             background process? -> '&' at last position

    Return:             
    
    - If Fail:          exit(EXIT_FAILURE)
    - If Succes:        char** parameters

*/
char** get_parameters(char* line, bool type)
{
    int buffer_size = TOKEN_BUFFER_SIZE, index = 0;
    char* token;
    char delimit[6] = " \t\r\n\v\f";

    char** parameters = (char**) malloc(buffer_size * sizeof(char*));
    if (parameters == NULL) {
        fprintf(stderr, "Memory allocation goes wrong.\n");
        exit(EXIT_FAILURE);
    }

    // parse (user) input into parameter array
    token = strtok(line, delimit);
    while (token != NULL) {
        parameters[index] = token;
        index++;

        if (index >= buffer_size) 
        {
            buffer_size += TOKEN_BUFFER_SIZE;
            parameters = realloc(parameters, buffer_size * sizeof(char*));
            if (parameters == NULL) {
                fprintf(stderr, "Memory allocation goes wrong.\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, delimit);
    }

    // if background process, overwrite '&'
    if(type)
        parameters[index-1] = NULL;
    else
        parameters[index] = NULL;

    return parameters;
}


/* Execute inputed command:

    Parameters:

    - parameters:       parsed user input
    - type:             background process? 
    - _pid:             PID of forked process

    Locals:

    - wait_status:      status of child process

    Return:             
    
    - If Fail:          EXIT_FAILURE
    - If Succes:        int wait_status
*/
int execute_command(char** parameters, bool type, pid_t* _pid)
{
    int wait_status;

    if(strcmp(parameters[0], "cd") == 0) 
        return change_directory(parameters[1]);
    else if(strcmp(parameters[0], "jobs") == 0)
        return show_background_processes();

    *_pid = fork();
    if (*_pid == -1) {
        // Error forking
        fprintf(stderr, "Unable to fork.\n");
        exit(EXIT_FAILURE);
    } 
    else if (*_pid == 0) {
        // Child process:   Execute command
        if (execvp(parameters[0], parameters) == -1)
            fprintf(stderr, "Unable to execute command.\n");
        return EXIT_FAILURE;
    } else {
        // Parent process:  Wait for child
        if(type) {
            // internal statuscode for not waiting
            return -42;
        } else {
            do {
                *_pid = waitpid(*_pid, &wait_status, WUNTRACED);
            } while (!WIFEXITED(wait_status) && !WIFSIGNALED(wait_status));
        }
    }

    return wait_status;
}


/* How to use: 

	- compile:		    make -rR
	- run:			    <path>/clash
	- type:			    <name> <param 1> ... <param n>

    Locals:

    - commmand_line:    user input
    - _line:            copy of user input 
    - working_dir:      working directory
    - parameters:       array of single words in user input
    - status:           status of processes; abort programm at error 
    - job_type:         background process?   
    - pid:              PID of Backgroundprocess
*/
int main(int argc, char **argv)
{
    char* command_line;
    char* _line;
    char* working_dir;
    char** parameters;
    int status;
    bool job_type = false;
    pid_t pid;

    // input loop
    do {
        // print working directory
        working_dir = get_working_dir();
        fprintf(stdout, "%s: ", working_dir);

        // read user input
        command_line = read_command();

        // skip if no command typed
        if((strcmp(command_line, "") && strcmp(command_line, " ")) == 0) 
            continue;

        // copy user input (because strtok)
        _line = (char*) malloc(strlen(command_line) * sizeof(char));
        if (_line == NULL) {
            fprintf(stderr, "Memory allocation goes wrong.\n");
            return EXIT_FAILURE;
        }
        strcpy(_line, command_line);

        // serach for '&' at the end of user input
        job_type = is_background(_line);

        // get parameters from user input
        parameters = get_parameters(_line, job_type);

        /* execute command and return status code.
           status code -42 target the background process. */
        status = execute_command(parameters, job_type, &pid);
        if(status == -42)
            // write pid and command_line in list
            insertElement(pid, command_line);
        else 
            fprintf(stdout, "Exisstatus [ %s ] = %d\n", command_line, status);

        // free allocated memory
        free(working_dir);
        free(command_line);
        free(_line);
        free(parameters);
    // Stop at error
    } while (true);

    return EXIT_SUCCESS;
}
