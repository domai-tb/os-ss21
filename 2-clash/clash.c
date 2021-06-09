#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>	
#include <string.h>	
#include <stdbool.h>


#define MAX_PATH_LENGTH 100
#define MAX_COMMAND_LINE_LENGTH 100
#define MAX_WORD_LENGTH 10


/* Execute user typed command:

	Parameters:
	- command:				array of parameters (<name> <param 1> ... <param n>)
	- parameter_number:		array index 

	Locals:
	- program_name:			name of program to execute
	- pid:					PID from fork
	- parameters:			parsed array of command for execvp syntax 
*/

//  TODO: change directory not working -> fix it

int execute_command(char*** command, size_t* parameter_number)
{
	char* program_name = (*command)[0];
	char* parameters[*parameter_number];
	pid_t pid = fork();
	int wait_status;
	
	if(pid == -1) {
		fprintf(stderr, "Can't fork.\n");
		return EXIT_FAILURE;
	}

	for(int i = 0; i < *parameter_number; i++)
		parameters[i] = (*command)[i];
	parameters[*parameter_number] = NULL;

	if(pid == 0) {
		// child process
		if(execvp(program_name, parameters) == -1) {
			fprintf(stderr, "Could not execute program. Is the program name correct?\nSyntax: <program name> <parameter 1> ... <parameter n> \n");
			return EXIT_FAILURE;
		}
	} else {
		// parent process
		wait(&wait_status);
		if(WIFEXITED(wait_status)) {
			return WEXITSTATUS(wait_status);
		}
	}
}


/* Read command from input line:

	Parameters:
	- parameter_number:		array index
	- command_line:			user input
	- command:				array of parameters (<name> <param 1> ... <param n>)

	Locals:
	- number:				internal array index 
	- token:				strtok return pointer to first word
	- delimit:				delimiters for user input -> strtok function
	- _command_line:		internal command_line
*/
int read_command(size_t* parameter_number, char*** command, char* command_line)
{
	char* token = 0;
	size_t number = 0;
	char delimit[6] = " \t\r\n\v\f";

	if (fgets(command_line, MAX_COMMAND_LINE_LENGTH, stdin) == NULL) {
		fprintf(stderr, "Input reading failed.\n");
		return EXIT_FAILURE;
	} 

	// prepare command_line for strtok
	int length = strlen(command_line);
	if(command_line[length-1] == '\n') { 
		command_line[length-1] = '\0';
		length--;
	}

	char _command_line[length];

	// reading words from command_line 
	strcpy(_command_line, command_line);
	token = strtok(_command_line, delimit);
	while(token != NULL) 
	{
		*command = (char**) realloc(*command, (number + 1) * sizeof(*(*command)));
		if(*command == NULL) {
			fprintf(stderr, "Memory allocation goes wrong.\n");
			return EXIT_FAILURE;
		}	

		(*command)[number] = (char*) calloc(MAX_WORD_LENGTH, sizeof(char));
		if((*command)[number] == NULL) {
			fprintf(stderr, "Memory allocation goes wrong.\n");
			return EXIT_FAILURE;
		}	

		(*command)[number] = token;
		token = strtok(NULL, delimit);
		number += 1;
	}

	*parameter_number = number;

	return EXIT_SUCCESS;
}


/* How to use: 
	- compile:		make -rR
	- run:			<path>/test
	- type:			<name> <param 1> ... <param n>
*/
int main(int argc, char **argv) 
{
	char working_dir[MAX_PATH_LENGTH];
	char command_line[MAX_COMMAND_LINE_LENGTH];
	size_t parameter_number = 0;
	char** command = NULL;
	int exis_status;

	if (argc < 1) {
		fprintf(stderr, "Program start failed.\n");
		return EXIT_FAILURE;
	}

	// get working directory
	if (getcwd(working_dir, sizeof(working_dir)) == NULL) {
		fprintf(stderr, "Can't read working directory.\n");
		return EXIT_FAILURE;
	}

	// input loop
	while(true)
	{
		// read command line 
		fprintf(stdout, "\n%s: ", working_dir);
		if(read_command(&parameter_number, &command, command_line) == EXIT_FAILURE) {
			fprintf(stderr, "Can't read input.\n");
			return EXIT_FAILURE;	
		}

		//execute command
		exis_status = execute_command(&command, &parameter_number);
		if(exis_status != EXIT_SUCCESS) {
			fprintf(stderr, "Can't execute command.\n");
			// no reason to stop the program
		}
		fprintf(stdout, "Exisstatus [ %s ] = %d", command_line, exis_status);

		// free allocated memory 
		// TODO: Fixing bug about free array elements
		// for(int i = 0; i < parameter_number; i++) {
		// 	fprintf(stdout, "%s\n", command[i]);			// no problem
		// 	free(command[i]);								// invalid pointer ... ?
		// }
		//free(command);
	}

	if(fflush(stdout) == EOF) {
		fprintf(stderr, "Output flushing goes wrong.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}