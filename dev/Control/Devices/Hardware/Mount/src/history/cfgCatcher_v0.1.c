/** Compilation: gcc -o memreader memreader.c -lrt -lpthread **/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "cfg_buffer.h"

#define AccessPermsIN 0644 

void report_and_exit(const char* msg) 
{
	perror(msg);
  	exit(-1);
}

int main() 
{

	printf("Here 1\n");
	cfgBuffer_data_type * config_var				;
	size_t ByteSize = sizeof(config_var)			;
	int fd_shmem							;
	sem_t * semptr							;

	static char *BackingFileIN = "HAXConfigBuffer"			;
	static char *SemaphoreNameIN = "HAXConfigSemaphore"		;
//	unsigned int AccessPermsIN = S_IRWXU|S_IRWXG			;

//	char *IP_Server;


	printf("Here 2\n");
	fd_shmem = shm_open(BackingFileIN, O_RDWR, AccessPermsIN);  // empty to begin 

	if (fd_shmem < 0) 
	{
		report_and_exit("Can't get file descriptor.");
	}

  	printf("Here 3\n");
	// get a pointer to memory 
  	config_var = mmap(NULL,       		// let system pick where to put segment 
        	ByteSize,   			// how many bytes 
		PROT_READ | PROT_WRITE, 	// access protections 
		MAP_SHARED, 			// mapping visible to other processes 
		fd_shmem,         		// file descriptor 
		0);         			// offset: start at 1st byte 

	printf("Here 4\n");
	if ((void *) -1 == config_var) 
	{
		report_and_exit("Can't access segment for shared memory.");
	}	

	// create a semaphore for mutual exclusion 
	semptr = sem_open(SemaphoreNameIN, 	// name 
		O_CREAT,       			// create the semaphore
		AccessPermsIN,   		// protection perms 
		0);            			// initial value 
	
	if (semptr == (void*) -1) 
	{
		report_and_exit("Can't open semaphore.");
	}

	/* use semaphore as a mutex (lock) by waiting for writer to increment it */
	if (!sem_wait(semptr))  /* wait until semaphore != 0 */
	{
	//	strncpy(IP_Server, config_var->IP_SERVER, ByteSize);

		printf("\nConfiguration was recorded in Shared Memory!\n\n");
       // 	printf("IP_SERVER            = %s \n",IP_Server)			;
        	printf("IP_SERVER            = %s \n",&config_var->IP_SERVER)		;
        	printf("TCP_PORT             = %ld\n",config_var->TCP_PORT)             ;
       		printf("RCV_BUFFER_SIZE      = %ld\n",config_var->RCV_BUFFER_SIZE)      ;
       		printf("TX_DELAY	     = %ld\n",config_var->TX_DELAY)		;
       		printf("TX_DELAY	     = %ld\n",&config_var->TX_DELAY)		;
       // 	printf("DIRECTORY_LOG        = %s \n",config_var->DIRECTORY_LOG)        ;
        	printf("DIRECTORY_LOG        = %s \n",&config_var->DIRECTORY_LOG)        ;
        //	printf("GetPos_BackingFile   = %s \n",config_var->GetPos_BackingFile)   ;
        //	printf("GetPos_SemaphoreName = %s \n",config_var->GetPos_SemaphoreName) ;
        	printf("GetPos_AccessPerms   = %ld\n",config_var->GetPos_AccessPerms)   ;
        	printf("RINGSIZE             = %ld\n",config_var->RINGSIZE)             ;
        //	printf("DATAFILENAME         = %s \n",config_var->DATAFILENAME)         ;
        //	printf("OpMode_BackingFile   = %s \n",config_var->OpMode_BackingFile)   ;
        //	printf("OpMode_SemaphoreName = %s \n",config_var->OpMode_SemaphoreName) ;
        	printf("OpMode_AccessPerms   = %ld\n\n",config_var->OpMode_AccessPerms) ;
	
		sem_post(semptr);
		
	}

	/* cleanup */
	munmap(config_var, ByteSize);
	close(fd_shmem);
	sem_close(semptr);
//	unlink(BackingFile);
	return 0;
}
