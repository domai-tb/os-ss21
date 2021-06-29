/*
        TODO:

        - limit thread to max_thread
        - execute group by group 
        - outout in seperate thread

*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "run.h"
#include "queue.h"

#define MAX_LINE 4096

static void die(const char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}

// More than one argument at work_routine
struct thread_arguments
{
    char* command;
    char* command_output;
};

/** 
 * Execute command and add it to queue.
 * 
 * The purpose of this function is to parse the
 * "run_cmd" to void*, because "pthread_create"
 * can handly void* only.
 * 
 */
static void* work_routine(void* arguments)
{
    // parsing struct 
    struct thread_arguments *args = (struct thread_arguments*) arguments;

    //printf("{Work-Routine-Beginn} Command: [%s] Out: [%s]\n", (*args).command, (*args).command_output);

    // execute command
    if (run_cmd((*args).command, &((*args).command_output)) < 0)
        die("run_cmd");

    // add command and output to queue
    if (queue_put((*args).command, (*args).command_output, 0) != 0)
        die("queue_put");

    //printf("{Work-Routine-End} Command: [%s] Out: [%s]\n", (*args).command, (*args).command_output);

    return NULL;
}

static void create_worker_thread_and_execute_command(char* command)
{
    pthread_t worker_thread;

    // create work thread and execute command
    if (pthread_create(&worker_thread, NULL, &work_routine, command) != 0)
        die("pthread_create");
    pthread_detach(worker_thread);
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

    char* path_to_file;
    char file_line[MAX_LINE + 1];
    FILE* file;

    char c; // placeholder character
    int max_threads;
    
    // handle wrong usage
    if(argc != 3)
    {
		fprintf(stderr, "usage: ./mach <anzahl threads> <mach-datei>\n");
		return EXIT_FAILURE;
	}
	//"parse" args
	max_threads = parse_positive_int_or_die(argv[1]);
	path_to_file = argv[2];
	
	file = fopen(path_to_file, "r");
    // can't open file
	if(!file)
		die("File");
	
    // init queue
    if (queue_init() != 0)
    {
        fprintf(stderr, "queue_init\n");
        exit(EXIT_FAILURE);
    }

    // reading commands from mach-file
	while (fgets(file_line, MAX_LINE, file) != NULL) 
    {
        struct thread_arguments args;

        // MAX_LINE exceeded
        size_t length = strlen(file_line);	
		if(length == MAX_LINE && file_line[MAX_LINE-1] != '\n')
        {
            fprintf(stderr, "Input too long.\n");
            while ((c = fgetc(stdin)) != EOF && c != '\n');
            continue;
		} 

        // execute command in a seperate thread
        args.command = file_line;
        args.command_output = NULL;
        create_worker_thread_and_execute_command((void*) &args);

        printf("Command: [%s] Output: [%s]\n", args.command, args.command_output);
        if(EOF == fflush(stdout))
            die("fflush");
    }

    printf("Maximale Anzahl an Threads: %d\n", max_threads);
    if(EOF == fflush(stdout))
        die("fflush");

    // wait for all thread
    pthread_exit(EXIT_SUCCESS);
}
