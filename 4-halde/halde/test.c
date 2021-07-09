#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "halde.h"

int main(int argc, char *argv[]) {
	printList();

	char *m1, *m2, *m3, *m4, *m5, *special_cases;

	// test malloc function
	m1 = malloc(200*1024);
	m2 = malloc(1024*12);
	m3 = malloc(24*12);
	m4 = malloc(1024*450);
	m5 = malloc(1024);
	printList();

	// test free function
	free(m1);
	free(m2);
	free(m3);
	free(m4);
	free(m5);
	printList();

	/*
		Special Case:		try to free a NULL pointer
	*/
	free(NULL);
	printList();

	// test malloc function
	m1 = malloc(200*1024);
	m2 = malloc(1024*12);
	m3 = malloc(24*12);
	m4 = malloc(1024*450);
	printList();

	/*
		Special Case:		ENOMEM - Out of memory
	*/
	special_cases = malloc(1024*1024*2);
	printList();

	// test free function
	free(m1);
	free(m2);
	free(m3);
	free(m4);
	free(special_cases);
	printList();

	/*
		Special Case:		Allocated whole heap 
		===> TODO: Segmentation fault --> GDB
	*/
	special_cases = malloc(1024*1024);
	printList();
	free(special_cases);
	printList();

	// test realloc function
	m1 = malloc(1);
	printList();
	m1 = realloc(m1, 1024);
	printList();
	free(m1);

	// test calloc function
	m1 = calloc(10, 1024);
	printList();
	free(m1);

	exit(EXIT_SUCCESS);
}
