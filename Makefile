all:
	gcc -std=c99 -DDEBUG=1 -Wall -Werror -Wno-unused-function -pedantic \
-ggdb -O0 -o main main.c -lm

