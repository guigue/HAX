#Compiler Flags
CC=gcc
CFLAGS=-lm -lfftw3

#Project files

EXE = amplitude_spectrum

#Base dependencies
SRCS = ../windft/windowed_dft.c amplitude.c 
OBJS = ../windft/windowed_dft.o amplitude.o

debug : CFLAGS += -g

$(EXE) : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

debug : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

clean : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) 
	rm -f $(OBJS)
