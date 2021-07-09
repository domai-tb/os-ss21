#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "halde.h"

int main(int argc, char *argv[]) {
	printList();

	char *m1 = malloc(200*1024);
	printList();

	char *m2 = calloc(200, 1024);
	printList();

	free(m2);
	printList();

	m1 = realloc(m1, 300*1024);
	printList();

	free(m1);
	printList();

	exit(EXIT_SUCCESS);
}
