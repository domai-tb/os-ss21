#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>	
#include <string.h>	
#include <stdbool.h>


#define MAX_PATH_LENGTH 100
#define MAX_COMMAND_LINE_LENGTH 100
# define MAX_WORD_LENGTH 10


/* Execute user typed command:
	- command:				array of parameters (<name> <param 1> ... <param n>)
	- parameter_number:		array index 
	- program_name:		name of program to execute
*/
int execute_command(char*** command, size_t* parameter_number)
{
	char* program_name = (*command)[0];

	// TODO: Execute command
	fprintf(stdout, "%s\n", program_name);

	return EXIT_SUCCESS;
}


/* Read command from input line:
	- parameter_number:		array index
	- number:				internal array index 
	- command_line:			user input
	- token:				strtok return pointer to first word
	- delimit:				delimiters for user input -> strtok function
	- command:				array of parameters (<name> <param 1> ... <param n>)
*/
int read_command(size_t* parameter_number, char*** command, char* command_line)
{
	char* token = 0;
	size_t number = 0;
	char delimit[6] = " \t\r\n\v\f";

	if (fflush(stdout) == EOF) {
		fprintf(stderr, "Output clearing goes wrong.\n");
		return EXIT_FAILURE;
	}

	if (fgets(command_line, MAX_COMMAND_LINE_LENGTH, stdin) == NULL) {
		fprintf(stderr, "Input reading failed.\n");
		return EXIT_FAILURE;
	} 
	else {
		// reading words from command_line 
		token = strtok(command_line, delimit);
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
		fprintf(stdout, "%s: ", working_dir);
		read_command(&parameter_number, &command, command_line);

		//execute command
		if(fork() == 0)
			execute_command(&command, &parameter_number);

		// free allocated memory 
		// TODO: Fixing bug about free array elements
		// for(int i = 0; i < parameter_number; i++) {
		// 	fprintf(stdout, "%s\n", command[i]);			// no problem
		// 	free(command[i]);								// invalid pointer ... ?
		// }
		//free(command);
	}

	return EXIT_SUCCESS;
}