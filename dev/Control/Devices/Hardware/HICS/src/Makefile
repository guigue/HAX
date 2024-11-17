OS   = LINUX
FIRMWARE_VERSION = FIRMWARE_VERSION_120
PROMPT_CALIBRATION = NO_PROMPT_CALIBRATION 

CC      = gcc
CFLAGS  =  -g -Wall -I ../inc -D$(OS) -D$(FIRMWARE_VERSION) -D$(PROMPT_CALIBRATION) 
LDFLAGS = -lrt -lpthread

VPATH   = ../inc
HOMS_SERVER = HOMS_data_transfer_daemon
HICS_CLIENT = HICS_client
HOMS_DATA_SERVER = HOMS_precalibration
HOMS_SERVICES = HOMS_stopacquire
MAIN_PROGS   =  $(HOMS_SERVER) $(HOMS_DATA_SERVER) $(HOMS_SERVICES)

LOG_MSG_OBJS = log_msg.o

all: compile
compile: $(MAIN_PROGS)

log_msg.o: log_msg.c data_transfer.h

HOMS_data_Transfer_daemon.o: HOMS_data_transfer_daemon.c data_transfer.h 
HOMS_data_transfer_daemon:   HOMS_data_transfer_daemon.o $(LOG_MSG_OBJS)
	$(CC) $(CFLAGS) HOMS_data_transfer_daemon.o $(LOG_MSG_OBJS) -o $(HOMS_SERVER) $(LDFLAGS)
	echo

HICS_client.o: HICS_client.c data_transfer.h
HICS_client: HICS_client.o $(LOG_MSG_OBJS)
	$(CC) HICS_client.o $(LOG_MSG_OBJS) -o $(HICS_CLIENT)
	echo

HOMS_precalibration.o: HOMS_precalibration.c data_transfer.h log_msg.c
HOMS_precalibration: HOMS_precalibration.o $(LOG_MSG_OBJS)
	$(CC) $(CFLAGS) HOMS_precalibration.o $(LOG_MSG_OBJS) -o $(HOMS_DATA_SERVER) $(LDFLAGS)
	echo

HOMS_stopacquire.o: HOMS_stopacquire.c data_transfer.h 
HOMS_stopacquire:   HOMS_stopacquire.o 
	$(CC) $(CFLAGS) HOMS_stopacquire.o -o $(HOMS_SERVICES)
	echo

clean:
	rm -f $(MAIN_PROGS) *.err *.o */*.err */*.o 

