#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "data_transfer.h"

#define DEBUG

void log_msg (unsigned short int, char *, unsigned short int) ;

int main()
{
  
  /* shared memories */
  unsigned char * shmem_ptr ;
  int  shmem_fd, shmem_info_fd  ;
  ring_buffer_shmem_info  * shmem_info_ptr ;

  /* Semaphores */
  sem_t * sem_ptr  ;

  /* Data buffers */
  HICS_DATA_STR * hics_data_ptr = malloc(sizeof(HICS_DATA_STR)*TEN_MIN_MS) ; 
  receive_data_str * data_buffer_ptr = malloc(sizeof(receive_data_str))    ;

  char msg[120]                      ;

  FILE * testfile                    ;

    /* 
     * register behind syslog()
     */
  openlog("precalibration", LOG_NDELAY, LOG_ACQ)  ;

    /* 
     *  Create the shared memories
     */

  if ( 0> (shmem_fd = shm_open(ACQ_SHMEM_BCKFILE, O_RDWR , ACQ_SHMEM_PERM )))
    {
      log_msg(LOG_ACQ,"Can't open shared memory segment",EMERGENCY);
      exit(1)             ;
    }
  
  if ( (void *) -1  == (shmem_ptr = mmap(NULL, ACQ_SHMEM_SIZE , PROT_READ | PROT_WRITE, MAP_SHARED, shmem_fd,0)))
    {
      log_msg(LOG_ACQ,"Can't get segment...",EMERGENCY);
      exit(1);
    } else {
    sprintf(msg, "shared memory base address= %p , size=%d ",shmem_ptr, ACQ_SHMEM_SIZE);
    log_msg(LOG_ACQ,msg , NOTICE);
    sprintf(msg,"backing file: /dev/shm/%s", ACQ_SHMEM_BCKFILE);
    log_msg(LOG_ACQ, msg, NOTICE );
  }

  if ( 0> (shmem_info_fd = shm_open(ACQ_SHMEM_INFO_BCKFILE, O_RDWR , ACQ_SHMEM_PERM )))
    {
      log_msg(LOG_ACQ,"Can't open shared memory info segment",EMERGENCY);
      exit(1)             ;
    }

  if ( (void *) -1  == (shmem_info_ptr = mmap(NULL, sizeof(ring_buffer_shmem_info) ,
					      PROT_READ | PROT_WRITE, MAP_SHARED, shmem_info_fd,0)))
    {
      log_msg(LOG_ACQ,"Can't get segment for shared memory info",EMERGENCY);
      exit(1);
    } else {
    sprintf(msg, "shared memory info base address= %p , size=%ld ",shmem_info_ptr, sizeof(ring_buffer_shmem_info));
    log_msg(LOG_ACQ,msg , NOTICE);
    sprintf(msg,"backing file: /dev/shm/%s", ACQ_SHMEM_INFO_BCKFILE);
    log_msg(LOG_ACQ, msg, NOTICE );
  }
      
  /* 
   * semaphore code to lock the shared mem
   */
  if ( (void *) -1 == ( sem_ptr = sem_open(ACQ_SEM_NAME, O_RDWR , ACQ_SHMEM_PERM,0)))
    {
      log_msg(LOG_ACQ,"Can't open semaphore",EMERGENCY);
      exit(1);
    }
  
  testfile = fopen("/data/homs/testfile","w");


  for (;;){
    
    if ( ! sem_wait(sem_ptr) )
      { //
	memcpy(data_buffer_ptr, shmem_ptr , shmem_info_ptr->nrec * DATA_QUARK_SIZE);
	fprintf(testfile,"Nrec = %d\n",shmem_info_ptr->fake);
	fflush(testfile);
      }
    
  }

  fclose(testfile);
  
}
  
