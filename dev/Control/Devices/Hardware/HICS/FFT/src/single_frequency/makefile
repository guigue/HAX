#Compiler Flags
CC=gcc
CFLAGS=-lm -lfftw3

#Project files

EXE = single_frequency

#Base dependencies
SRCS = ../windft/windowed_dft.c single_frequency_amplitude.c 
OBJS = ../windft/windowed_dft.o single_frequency_amplitude.o

debug : CFLAGS += -g

$(EXE) : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

debug : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

clean : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)
	rm -f $(OBJS)
