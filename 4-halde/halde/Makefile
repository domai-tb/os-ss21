CFLAGS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

all: test test-ref

test: test.o halde.o
	gcc -o test test.o halde.o

test-ref: test.o halde-ref.o
	gcc -o test-ref test.o halde-ref.o

test.o: test.c halde.h
	gcc $(CFLAGS) -c test.c

halde.o: halde.c halde.h
	gcc $(CFLAGS) -c halde.c

clean:
	rm -f test test-ref test.o halde.o

.PHONY: all clean