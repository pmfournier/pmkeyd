CFLAGS=-Wall -Werror

all: inputview pmkeyd

inputview: inputview.c
pmkeyd: pmkeyd.c

clean:
	rm -f inputview pmkeyd
