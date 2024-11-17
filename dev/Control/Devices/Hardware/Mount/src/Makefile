# Makefile for the Mount getter
#
# Guigue - 2019-11-19 @Sampa
#
# Last by Tiago - 2021-02-16
#
########################################################################

CFLAGS =  -g  -Wall -I../inc
CLIBS = -lrt -lpthread -lconfuse -lm
CC = gcc

GETPOS_EXE = getPos
GETPOS = getPos.c

PTRACK_EXE = pTrack
PTRACK = pTrack.c

SKYDIP_EXE = skyDip
SKYDIP = skyDip.c

SCANTAR_EXE = scanTar
SCANTAR = scanTar.c

CFGSTORE_EXE = cfgStore
CFGSTORE = cfgStore.c

CFGCATCHER_EXE = cfgCatcher
CFGCATCHER = cfgCatcher.c

DELSHMEM_EXE = delShmem
DELSHMEM = delShmem.c

TEST_EXE = test_getopt
TEST = test_getopt.c

MAIN_PROGS = $(GETPOS_EXE) $(PTRACK_EXE) $(SKYDIP_EXE) $(SCANTAR_EXE) $(CFGSTORE_EXE) $(CFGCATCHER_EXE) $(DELSHMEM_EXE) $(TEST_EXE)

#MAIN_PROGS = $(GETPOS_EXE) $(PTRACK_EXE) $(CFGSTORE_EXE) $(CFGCATCHER_EXE)

#########################################################################
all: compile

getPos: $(GETPOS)
	$(CC) $(CFLAGS) $(GETPOS) -o $(GETPOS_EXE) $(CLIBS)

pTrack: $(PTRACK)
	$(CC) $(CFLAGS) $(PTRACK) -o $(PTRACK_EXE) $(CLIBS)

skyDip: $(SKYDIP)
	$(CC) $(CFLAGS) $(SKYDIP) -o $(SKYDIP_EXE) $(CLIBS)

scanTar: $(SCANTAR)
	$(CC) $(CFLAGS) $(SCANTAR) -o $(SCANTAR_EXE) $(CLIBS)

cfgStore: $(CFGSTORE)
	$(CC) $(CFLAGS) $(CFGSTORE) -o $(CFGSTORE_EXE) $(CLIBS)

cfgCatcher: $(CFGCATCHER)
	$(CC) $(CFLAGS) $(CFGCATCHER) -o $(CFGCATCHER_EXE) $(CLIBS)

delShmem: $(DELSHMEM)
	$(CC) $(CFLAGS) $(DELSHMEM) -o $(DELSHMEM_EXE) $(CLIBS)

test: $(TEST)
	$(CC) $(CFLAGS) $(TEST) -o $(TEST_EXE) $(CLIBS)

compile: $(MAIN_PROGS)

clean:
	rm -f $(MAIN_PROGS) *.o

