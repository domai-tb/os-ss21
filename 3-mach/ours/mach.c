#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "run.h"

#define MAX_LINE 4096
static void die(const char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}


static int parse_positive_int_or_die(char *str) {
    errno = 0;
    char *endptr;
    long x = strtol(str, &endptr, 10);
    if (errno != 0) {
        die("invalid number");
    }
    // Non empty string was fully parsed
    if (str == endptr || *endptr != '\0') {
        fprintf(stderr, "invalid number\n");
        exit(EXIT_FAILURE);
    }
    if (x <= 0) {
        fprintf(stderr, "number not positive\n");
        exit(EXIT_FAILURE);
    }
    if (x > INT_MAX) {
        fprintf(stderr, "number too large\n");
        exit(EXIT_FAILURE);
    }
    return (int)x;
}

int main(int argc, char **argv) {
    if(argc != 3)
    {
		fprintf(stderr, "usage: ./mach <anzahl threads> <mach-datei>\n");
		return EXIT_FAILURE;
	}
	
	int max_threads = parse_positive_int_or_die(argv[1]);
	
	char* path_to_file = argv[2];
	char file_line[MAX_LINE + 1];
	FILE* file = fopen(path_to_file, "r");
	if(!file)
		die("File");
	
	while (fgets(file_line, MAX_LINE, file) != NULL) {
		printf(file_line);
		char* cmd_out = NULL;
		run_cmd(file_line, &cmd_out);
		printf("%s\n", cmd_out);
	}
		
    printf("Test %d\n", max_threads);
    return EXIT_SUCCESS;
}
