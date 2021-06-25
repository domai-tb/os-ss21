#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "run.h"
#include "queue.h"

#define MAX_LINE 4096

static void die(const char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}

/** 
 * Execute command and command to queue.
 * 
 * The purpose of this function is to parse the
 * "run_cmd" to void*, because "pthread_create"
 * can handly void* only.
 * 
 */
static void* routine(const char* cmd)
{
    char* cmd_out = NULL;

    // execute command
    if (run_cmd(cmd, &cmd_out) < 0)
        die("run_cmd");

    // add thread to queue
    /* keine Ahnung was die flags sind... */
    queue_put(cmd, cmd_out, 0);
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
	
    // init queue
    if (queue_init() != 0)
    {
        fprintf(stderr, "queue_init\n");
        exit(EXIT_FAILURE);
    }

    // reading commands from mach-file
	while (fgets(file_line, MAX_LINE, file) != NULL) {
		printf("%s\n", file_line);

        // MAX_LINE exceeded
        size_t length = strlen(file_line);	
		if(length == MAX_LINE && file_line[MAX_LINE-1] != '\n')
        {
            fprintf(stderr, "Input too long.\n");
            int c;
            while ((c = fgetc(stdin)) != EOF && c != '\n');
            continue;
		} 

        // create thread
        /* Es müssse max_thread oder weniger  threads erstellt werden
            ==> for loop? 
            ==> file_line in array speichern und so viele thread erstellen,
                wie array einträge hat?
        */
        pthread_t thread_1;
        if (pthread_create(&thread_1, NULL, &routine, file_line) != 0)
            die("pthread_create");

        // wait for thread
        /* Nachdem alle commands einer Gruppe ausgeführt wurden, soll auf
           alle Thread gewartet werden. Sollte mehr commands als "max_threads"
           in einer Gruppe sein, müssen manche Thread wieder verwendet werden.
        */
        if (pthread_join(thread_1, NULL) != 0)
            die("pthread_join");
	}
		
    printf("Test %d\n", max_threads);
    return EXIT_SUCCESS;
}
