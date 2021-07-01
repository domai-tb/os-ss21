#include "halde.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// Magic value for occupied memory chunks.
#define MAGIC ((void*)0xbaadf00d)

/// Size of the heap (in bytes).
#define SIZE (1024*1024*1)

/// Memory-chunk structure.
struct mblock {
	struct mblock *next;	// Zeiger zur Verkettung
	size_t size;			// Größe des Speicherbereichs
	char memory[];			// Anfang des Speicherbereichs
};

/// Heap-memory area.
static char memory[SIZE];

/// Pointer to the first element of the free-memory list.
static struct mblock *head;

/// Helper function to visualise the current state of the free-memory list.
void printList(void) {
	struct mblock *lauf = head;

	// Empty list
	if (head == NULL) {
		char empty[] = "(empty)\n";
		write(STDERR_FILENO, empty, sizeof(empty));
		return;
	}

	// Print each element in the list
	const char fmt_init[] = "(off: %7zu, size:: %7zu)";
	const char fmt_next[] = " --> (off: %7zu, size:: %7zu)";
	const char * fmt = fmt_init;
	char buffer[sizeof(fmt_next) + 2 * 7];

	while (lauf) {
		size_t n = snprintf(buffer, sizeof(buffer), fmt
			, (uintptr_t) lauf - (uintptr_t)memory, lauf->size);
		if (n) {
			write(STDERR_FILENO, buffer, n);
		}

		lauf = lauf->next;
		fmt = fmt_next;
	}
	write(STDERR_FILENO, "\n", 1);
}

void *malloc (size_t size) {

	errno = 0;
	
	// Search for free memory in Heap (static char memory)
		// TODO
		/* if ( out of memory )
		{
			errno = ENOMEM; 		// errno from man page
			return NULL;
		} */
	

	// create new mblock and initialize
		// TODO: create size-Bytes behind last block
	struct mblock new_block;
	new_block.next = NULL;
	new_block.size = sizeof(struct mblock);

	// mark head as used (magic) and update size
		/* TODO: declare block with magic adress:
				- head->memory = MAGIC: wrong
				- head = MAGIC: doesn't make sens
				- &head = MAGIC: wrong
				- ...
		*/
	head->size = size;

	// save memory pointer
	void* allocated_memory = head->memory;

	// move head to new block
	head = &new_block;

	// return pointer to allocated memory
	return allocated_memory;

	return NULL;
}

void free (void *ptr) {
	
	// find mblock of memory: mbp pointer
		// TODO

	// check for validity (is magic?)
		// TODO
	
	// set old mblock->next to head
		// TODO

	// set head to mbp 
		// TODO
}

void *realloc (void *ptr, size_t size) {
	// TODO: implement me!
	return NULL;
}

void *calloc (size_t nmemb, size_t size) {
	// TODO: implement me!
	return NULL;
}
