OPTIONS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

.PHONY: all
all: clash

clash: clash-2.c
	gcc $(OPTIONS) clash-2.c .clash/plist.c -o clash

.PHONY: clean
clean:
	rm clash