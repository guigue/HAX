#Compiler Flags
CC=gcc
LDFLAGS= -lm -lfftw3
CFLAGS= -g -Wall -I../inc

#Project files

EXE = readbin2

#Base dependencies
SRCS = ../FFT/src/windft/windowed_dft.c readbin2.c 
OBJS = ../FFT/src/windft/windowed_dft.o readbin2.o

debug : CFLAGS += -g

$(EXE) : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LDFLAGS)

debug : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LDFLAGS)

clean : $(OBJS)
	rm -f $(OBJS)
