.PHONY: all

CC=gcc
CFLAGS=-std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

all: hello

hello: hello.c
	$(CC) $(CFLAGS) -o $@ $^
