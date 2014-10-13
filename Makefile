CFLAGS=-std=c99  -Wall -Werror -Wno-unused-function -pedantic
CLIBS=-lm

ifdef DEBUG
	CFLAGS += -DDEBUG=1 -ggdb -O0
else
	CFLAGS += -Ofast
endif

all:
	gcc  $(CFLAGS) -o main main.c $(CLIBS)

