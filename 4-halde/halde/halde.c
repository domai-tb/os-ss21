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

/*
	TODO: Debugging with GDB - Segmentation fault in some usecases
*/
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

	if (size == 0)
		return return_address;

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

	struct mblock* mblock;

	if (ptr != NULL)
	{
		mblock = (struct mblock*) ((size_t) ptr - sizeof(struct mblock));
		if (mblock->next != MAGIC)
			abort();
		mblock->next = head;
		head = mblock;
	}
	
	return; 
}

void *realloc (void *ptr, size_t size) {
	
	errno = 0;
	void* return_address;
	size_t copy_size;

	if (ptr == NULL) 
		return_address = malloc(size);
	else
	{
		// is size = 0 => realloc <=> free
		if (size == 0) {
			free(ptr);
			return_address = NULL;
		}
		else {
			return_address = malloc(size);
			if (return_address != NULL) 
			{
				// copy old memory to new allocated memory
				copy_size = *(size_t*)((size_t)ptr - 8);
				if (size <= copy_size) copy_size = size;
				memcpy(return_address, ptr, copy_size);
				free(ptr);
			}
		}
	}

	return return_address;
}

void *calloc (size_t nmemb, size_t size) {
	
	errno = 0;
	size_t memory_size = nmemb * size;
	void* return_address;

	if (size == 0 || nmemb == memory_size / size)
	{
		return_address = malloc(memory_size);
		// init with 0x0
		if (return_address != NULL) 
			memset(return_address, 0, size);
	}
	else 
	{
		errno = ENOMEM; 		// cannot allocate memory
		return_address = NULL;
	}
	
	return return_address;
}
