#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	
#include <string.h>	


#define MAX_PATH_LENGTH 100
#define MAX_COMMAND_LINE_LENGTH 100


/* Read command from input line:
	- parameter_number:		array index 
	- command_line:			user input
	- token:				return of strtok -> first word
	- number:				internal array index
	- command:				array of parameters (<name> <param 1> ... <param n>)
*/
int read_command(int* parameter_number)
{
	char* command_line;
	command_line = (char*) calloc(MAX_COMMAND_LINE_LENGTH, sizeof(char));
	char* token = 0;
	//char** command = 0;
	int number = 0;

	if (fflush(stdout) == EOF) {
		fprintf(stderr, "Output clearing goes wrong.\n");
		return EXIT_FAILURE;
	}

	if (fgets(command_line, MAX_COMMAND_LINE_LENGTH, stdin) == NULL) {
		fprintf(stderr, "Input reading failed.\n");
		return EXIT_FAILURE;
	} else {
		// get first word (= program name) from command_line
		token = strtok(command_line, " ");
		// insert word in command array
   		while(token != NULL) {
			fprintf(stdout, "%s\n", token);

			// TODO: write parameters (<name> <param 1> ... <param n>) into array
			//command[number] = token;
       		
			// get next word
			token = strtok(NULL, " ");
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
	int parameter_number = 0;

	if (argc < 1) {
		fprintf(stderr, "Program start failed.\n");
		return EXIT_FAILURE;
	}

	// get working directory
	if (getcwd(working_dir, sizeof(working_dir)) != NULL) {
		fprintf(stdout, "%s: ", working_dir);
		read_command(&parameter_number);
	} else {
		fprintf(stderr, "Can't read working directory.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}