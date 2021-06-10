/* 

	DAS WILL EINFACH NICHT FUNKTIONIEREN....

*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>	
#include <string.h>	
#include <stdbool.h>


#define MAX_PATH_LENGTH 100
#define MAX_COMMAND_LINE_LENGTH 100
#define MAX_WORD_LENGTH 12


/* Execute user typed command:

	Parameters:

	- command:				array of parameters (<name> <param 1> ... <param n>)
	- parameter_number:		array index 

	Locals:
	
	- program_name:			name of program to execute
	- pid:					PID from fork
	- parameters:			parsed array of command for execvp syntax 
	- wait_status:			status variable of child process
*/
int execute_command(char*** command, size_t* parameter_number)
{
	/*  TODO's: 

		- change directory not working -> fix it
		- commandline + pid liste 

	*/
	char* program_name = (*command)[0];
	char* parameters[(*parameter_number)+1];
	int wait_status;

	// // parse command to execvp syntax <=> add NULL at last element
	// for(int i = 0; i < *parameter_number; i++)
	// 	parameters[i] = (*command)[i];
	// parameters[*parameter_number] = NULL;

	// if(strcmp(parameters[(*parameter_number)-1], "&") == 0) {
	// 	// cut "&" from command
	// 	parameters[(*parameter_number)-1] = " ";

	// 	pid_t pid = fork();
	// 	if(pid == -1) {
	// 		fprintf(stderr, "Can't fork.\n");
	// 		return EXIT_FAILURE;
	// 	}
	// 	if(pid == 0) {
	// 		// child process
	// 		if(execvp(program_name, parameters) == -1) {
	// 			fprintf(stderr, "Could not execute program. Is the program name correct?\nSyntax: <program name> <parameter 1> ... <parameter n> \n");
	// 			return EXIT_FAILURE;
	// 		}
	// 	} else {
	// 		// parent process
	// 		return EXIT_SUCCESS;
	// 	}
	// } else {
	// 	pid_t pid = fork();
	// 	if(pid == -1) {
	// 		fprintf(stderr, "Can't fork.\n");
	// 		return EXIT_FAILURE;
	// 	}
	// 	if(pid == 0) {
	// 		// child process
	// 		if(execvp(program_name, parameters) == -1) {
	// 			fprintf(stderr, "Could not execute program. Is the program name correct?\nSyntax: <program name> <parameter 1> ... <parameter n> \n");
	// 			return EXIT_FAILURE;
	// 		}
	// 	} else {
	// 		// parent process
	// 		wait(&wait_status);
	// 		if(WIFEXITED(wait_status)) {
	// 			return WEXITSTATUS(wait_status);
	// 		}
	// 	}
	// }

	// return EXIT_FAILURE;

	for(int i = 0; i < *parameter_number; i++)
		parameters[i] = (*command)[i];
	parameters[*parameter_number] = NULL;

	pid_t pid = fork();
	if(pid == -1) {
		fprintf(stderr, "Can't fork.\n");
		return EXIT_FAILURE;
	}

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

	return EXIT_FAILURE;
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
		if(strlen(token)+2 >= MAX_WORD_LENGTH) {
			fprintf(stderr, "Check your command length!");
			return EXIT_FAILURE;
		}

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

	// input loop
	while(true)
	{
		// get working directory
		if (getcwd(working_dir, sizeof(working_dir)) == NULL) {
			fprintf(stderr, "Can't read working directory.\n");
			return EXIT_FAILURE;
		}

		// read command line 
		fprintf(stdout, "\n%s: ", working_dir);
		if(read_command(&parameter_number, &command, command_line) == EXIT_FAILURE) {
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

		if(fflush(stdout) == EOF) {
			fprintf(stderr, "Output flushing goes wrong.\n");
			return EXIT_FAILURE;
		}
	}

	// exit dealloc memory
	return EXIT_SUCCESS;
}