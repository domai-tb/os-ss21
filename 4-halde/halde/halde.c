#include "halde.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

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

static bool is_heap_initialized = false;

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
	void* return_address = NULL;
	struct mblock* current_mblock;
	struct mblock* new_mblock;

	// init heap 
	if (!is_heap_initialized) {
		head = (struct mblock*) memory;
		head->next = NULL;
		head->size = SIZE;
		is_heap_initialized = true;
	}

	if (size == 0) { return return_address; }

	// search for next free memory block and allocate memory
	current_mblock = (struct mblock*) &head;
	while (true)
	{
		if (current_mblock->next != MAGIC && current_mblock->size >= size) 
		{
			// create new memory block and initialize
			new_mblock = (struct mblock*) ((size_t)current_mblock + size);
			new_mblock->next = NULL;
			new_mblock->size = current_mblock->size - size - 2*sizeof(struct mblock);
			
			// move head
			head = new_mblock;

			// update current block and save memory address
			current_mblock->size = size;
			current_mblock->next = MAGIC;
			return_address = (void*) current_mblock->memory;
			current_mblock->next = MAGIC;
			break;
		}
		if (current_mblock->next == NULL) {
			errno = ENOMEM;			// cannot allocate memory
			break;
		}
		current_mblock = current_mblock->next;
	}
	
	return return_address;
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
