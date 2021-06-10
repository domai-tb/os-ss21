/* TODO-List: 

    - background processes
    - PID Liste (.clash/plist)
    - change directory
    - show background processes

*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>	
#include <string.h>	
#include <stdbool.h>


#define MAX_PATH_LENGTH 100
#define BUFFER_SIZE 128
#define TOKEN_BUFFER_SIZE 10


/* Get working directory:

    Parameters:
    -

    Locals:

    - buffer:           buffer for getting the working dir
    - working_dir:      working dir

    Return:             If Fail:    EXIT_FAILURE
                        If Succes:  char* working_dir

*/
char* get_working_dir() 
{
    char buffer[MAX_PATH_LENGTH];  

    if (getcwd(buffer, sizeof(buffer)) == NULL) {
		fprintf(stderr, "Can't read working directory.\n");
		return EXIT_FAILURE;
	}

    char* working_dir;
    working_dir = (char*) malloc(strlen(buffer) * sizeof(char));
    if(working_dir == NULL) {
        fprintf(stderr, "Memory allocation goes wrong.\n");
        return EXIT_FAILURE;
    }

    strcpy(working_dir, buffer);
    return working_dir;
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
        return EXIT_FAILURE;
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
        else if(c == EOF) 
            exit(EXIT_SUCCESS);
        // write character in buffer
        else
            command_line[index] = c;
        index++;

        // Realloc, if buffer size exceeded
        if (index >= buffer_size)
        {
            buffer_size += BUFFER_SIZE;
            command_line = (char*) realloc(command_line, buffer_size);
            if (command_line == NULL) {
                fprintf(stderr, "Memory allocation goes wrong.\n");
                return EXIT_FAILURE;
            }
        }
    }
}


/* Read one line of input:

    Parameters:
    -

    Locals:

    - buffer:           buffer for getting the working dir
    - working_dir:      working dir

    Return:             If Fail:    EXIT_FAILURE
                        If Succes:  char* working_dir

*/
char* read_line()
{
    char* line = NULL;
    size_t buffer_size = 0;

    if (getline(&line, &buffer_size, stdin) == -1)
    {
        if (feof(stdin))
            return EXIT_SUCCESS;
        else  {
            fprintf(stderr, "Unable to read line.\n");
            return EXIT_FAILURE;
        }
    }

    return line;
}


/* Execute command:

    TODO: Write documentation

*/
char** get_parameters(char* line)
{
    int buffer_size = TOKEN_BUFFER_SIZE, index = 0;
    char* token;
    char delimit[6] = " \t\r\n\v\f";

    char** parameters = (char**) malloc(buffer_size * sizeof(char*));
    if (parameters == NULL) {
        fprintf(stderr, "Memory allocation goes wrong.\n");
        return EXIT_FAILURE;
    }

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
                return EXIT_FAILURE;
            }
        }

        token = strtok(NULL, delimit);
    }
    parameters[index] = NULL;
    return parameters;
}


/* Execute command:

    TODO: Write documentation

*/
int execute_command(char** parameters)
{
    pid_t pid, wpid;
    int wait_status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(parameters[0], parameters) == -1) {
            fprintf(stderr, "Unable to execute command.\n");
        }
        return EXIT_FAILURE;
    } else if (pid == -1) {
        // Error forking
        fprintf(stderr, "Unable to execute command.\n");
    } else {
        // Parent process
        do {
            pid = waitpid(pid, &wait_status, WUNTRACED);
        } while (!WIFEXITED(wait_status) && !WIFSIGNALED(wait_status));
    }

    return wait_status;
}


/* Execute command:

    TODO: Write documentation

*/
int read_input()
{
    char* command_line;
    char** parameters;
    int status;

    do {
        fprintf(stdout, "%s: ", get_working_dir());

        command_line = read_command();
        parameters = get_parameters(command_line);
        status = execute_command(parameters);

        fprintf(stdout, "Exisstatus [ %s ] = %d\n", command_line, status);

        free(command_line);
        free(parameters);
    // Stop at error
    } while (status == 0);

    return EXIT_SUCCESS;
}

/* Execute command:

    TODO: Write documentation

*/
int main(int argc, char **argv)
{
    // input loop
    if(read_input() == EXIT_FAILURE) {
        fprintf(stderr, "Input reading failed.\n");
    }

    return EXIT_SUCCESS;
}