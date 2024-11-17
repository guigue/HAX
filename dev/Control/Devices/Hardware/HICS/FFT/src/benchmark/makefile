#Compiler Flags
CC=gcc
CFLAGS=-lm -lfftw3

#Project files

EXE = benchmark

#Base dependencies
SRCS = ../windft/windowed_dft.c benchmark.c
OBJS = ../windft/windowed_dft.o benchmark.o

debug : CFLAGS += -g

all : $(EXE)

$(EXE) : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

debug : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) -g $(CFLAGS)

clean : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)
	rm -f $(OBJS)
