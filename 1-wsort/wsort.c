#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// +3 in reason of string syntax
#define MAX_LINE_LENGTH 103


// compare strings alpha-numerical 
int compare(const void* a, const void* b) 
{
    return strcmp(*(const char**)a, *(const char**)b);
}


int main(int argc, char **argv)
{
		char** lines = 0;
		int line_count = 0, line_length;
		
		if (argc < 1)
		{
			fprintf(stderr, "Program start failed.\n");
			return EXIT_FAILURE;
		}
		
		fprintf(stdout, "Wait for input... \n");
		
		if(fflush(stdout) == EOF)
		{
			fprintf(stderr, "Output clearing goes wrong.\n");
			return EXIT_FAILURE;
		}
		
		// Read Input Loop
		while(!feof(stdin))
		{
				line_length = 0;
				
				// allocate memory of char pointer
				lines = (char**) realloc(lines, (line_count + 1) * sizeof(*lines));
				if(lines == NULL) 
				{
					fprintf(stderr, "Memory allocation goes wrong.\n");
					return EXIT_FAILURE;
				}	
				
				// allocate memory of line
				lines[line_count] = (char*) calloc(MAX_LINE_LENGTH, sizeof(char));
				if(lines[line_count] == NULL) 
				{
					fprintf(stderr, "Memory allocation goes wrong.\n");
					return EXIT_FAILURE;
				}
				
				// read input from stin in lines[line_count]
				if(fgets(lines[line_count], MAX_LINE_LENGTH, stdin) == NULL && !feof(stdin))
				{
					fprintf(stderr, "Input reading failed.\n");
					free(lines[line_count]);
					return EXIT_FAILURE;
				}
				
				// check if line is to long or empty
				line_length = strlen(lines[line_count]);		
				if(line_length >= MAX_LINE_LENGTH-1)
				{
					fprintf(stderr, "Line size is limited.\n");
					free(lines[line_count]);
					continue;
				}
				if(line_length == 1)
				{
					fprintf(stderr, "Ignoring empty line.\n");
					free(lines[line_count]);
					continue;
				}
				
				line_count++;
		}
		
		// sort input
		qsort(lines, line_count, sizeof(*lines), compare);

		// print sorted input and free memory
		fprintf(stdout, "\n\nSorted input...\n\n");
		for(int i = 0; i < line_count; i++)
		{
			if(fprintf(stdout, "%s", lines[i]) < 0)
			{
				fprintf(stderr, "Internal Error in 'printf' function occured.\n");
				return EXIT_FAILURE;
			}
			free(lines[i]);
		}
		free(lines);
				
		return EXIT_SUCCESS;
}
