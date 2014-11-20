
# run as:
# mpiexec -np 4 ./main --mpi ./tests/5-graph.txt 2

OBJECTS=utils.o \
	graph.o \
	problem.o \
	mpi_utils.o

BINARY=./main

CC=gcc
MPICC=mpicc
LD=$(CC)
MPILD=$(MPICC)
MKDIR_P=mkdir -p
MAKE=make

CFLAGS=-std=c99 -Wall -Werror -Wno-unused-function -pedantic
CLIBS=-lm

SRC=./src
BUILD=./build

ifdef DEBUG
	CFLAGS += -DDEBUG=1 -ggdb -O0
else
	CFLAGS += -Ofast
endif

#-------------------------------------------------------------------------------

all: build_dir $(OBJECTS) $(BINARY)

#-------------------------------------------------------------------------------

.PHONY: build_dir
build_dir:
	$(MKDIR_P) $(BUILD)

#-------------------------------------------------------------------------------

$(BINARY): $(OBJECTS) ./main.o
	$(MPILD) $(CFLAGS) $(addprefix $(BUILD)/, main.o) \
	$(addprefix $(BUILD)/, $(OBJECTS)) -o $(BINARY) $(CLIBS)

main.o: $(SRC)/main.c $(SRC)/utils.h $(SRC)/graph.h
	$(MPICC) $(CFLAGS) -c -o $(BUILD)/$@ $< $(CLIBS)

utils.o: $(SRC)/utils.c $(SRC)/utils.h
	$(CC) $(CFLAGS) -c -o $(BUILD)/$@ $< $(CLIBS)

graph.o: $(SRC)/graph.c $(SRC)/graph.h $(SRC)/utils.h
	$(CC) $(CFLAGS) -c -o $(BUILD)/$@ $< $(CLIBS)

problem.o: $(SRC)/problem.c $(SRC)/graph.h $(SRC)/utils.h  $(SRC)/problem.h
	$(MPICC) $(CFLAGS) -c -o $(BUILD)/$@ $< $(CLIBS)

mpi_utils.o: $(SRC)/mpi_utils.c $(SRC)/graph.h $(SRC)/utils.h $(SRC)/problem.h \
	$(SRC)/mpi_utils.h
	$(MPICC) $(CFLAGS) -c -o $(BUILD)/$@ $< $(CLIBS)
