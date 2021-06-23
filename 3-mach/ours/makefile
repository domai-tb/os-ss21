CFLAGS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

all: mach

mach: mach.o queue.o run.o sem.o
	gcc -o mach mach.o queue.o run.o sem.o

mach.o: mach.c
	gcc $(CFLAGS) -c mach.c

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

clean:
	rm -f mach mach.o queue.o

.PHONY: all clean
