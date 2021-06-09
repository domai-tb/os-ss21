#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define WORD_LENGTH 100
#define LINE_LENGTH (WORD_LENGTH+2)
#define LINES_AT_ONCE 100


static int compare(const void* x, const void* y)
{
		char* const* a = (char* const*)x;
		char* const* b = (char* const*)y;
		
		return strcmp(*a, *b);
}


int main(int argc, char** argv)
{
	char** words = NULL;
	unsigned int wordcount = 0;
	unsigned int alloccount = 0; 
	
	char buf[LINE_LENGTH];
	
	while(fgets(buf, LINE_LENGTH, stdin))
	{
		size_t length = strlen(buf);
		
		if(length == 101 && buf[WORD_LENGTH] != '\n') {
			fprintf(stderr, "Wort ist zu lang.\n");
			
			int c;
			do {
				c = getchar();
			} while(c != EOF && c != '\n');
			
			continue;
		}
		
		if(length < 1)
			continue;
			
		if(buf[length-1] == '\n') { 
			buf[length-1] = '\0';
			length--;
		}
		
		char* line = strdup(buf);
		if(line == NULL) {
			perror("Strdup");
			exit(EXIT_FAILURE);
		}
		
		if(alloccount <= wordcount) {
				alloccount += LINES_AT_ONCE;
				words = realloc(words, alloccount * sizeof(words));
				if(words == NULL) {
					perror("realloc");
					exit(EXIT_FAILURE);
				}
		}
		
		words[wordcount] = line;
		wordcount++;
	}
	
	if(ferror(stdin)) {
		perror("fget / getchar");
		exit(EXIT_FAILURE);
	}
	
	qsort(words, wordcount, sizeof(*words), compare);
	
	for(int i = 0; i < wordcount; i++) {
		if(EOF == puts(words[i])) {
			perror("puts");
			exit(EXIT_FAILURE);
		}
		
		free(words[i]);
	}
	free(words);
	
	if(EOF == fflush(stdout)) {
		perror("fflush");
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}
