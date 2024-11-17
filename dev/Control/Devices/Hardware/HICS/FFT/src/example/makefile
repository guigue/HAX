#Compiler Flags
CC=gcc
CFLAGS=-lm -lfftw3

#Project files

EXE = example

#Base dependencies
SRCS = ../windft/windowed_dft.c example.c
OBJS = ../windft/windowed_dft.o example.o

debug : CFLAGS += -g

all : $(EXE)

$(EXE) : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

debug : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) -g $(CFLAGS)

clean : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)
	rm -f $(OBJS)
