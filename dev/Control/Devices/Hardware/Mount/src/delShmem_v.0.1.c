/*
        ================================================================================
                             Universidade Presbiteriana Mackenzie
                 Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        ================================================================================

        Delete Shared Memories - version 0.1 
        --------------------------------------------------------------------------------
        Programa implementado para apagar todos os arquivos das memorias compartilhadas
	da suite de programas HAX:
		cfgStore
		cfgCatcher
		pTrack
		getPos
		scanTar
		skyDip
	
	As memorias sao criadas em: /dev/shm

	As memorias criadas sao:

		BackingFileIN 	     = "HAX-ConfigBuffer"
		SemaphoreNameIN      = "HAX-ConfigSemaphore"
		OpMode_BackingFile   = "HAX-OpModeBuffer"
		Opmode_SemaphoreName = "HAX-OpModeSemaphore"
		GetPos_BackingFile   = "HAX-GetPosRingBuffer"
		GetPos_Semaphore     = "HAX-GetPosSemaphore"

	--------------------------------------------------------------------------------

        Autor: Tiago Giorgetti
        Email:  tiago.giorgetti@craam.mackenzie.br

        --------------------------------------------------------------------------------

        Histórico:
        ________________________________________________________________________________
         Versão |  Data         |       Atualização
        --------------------------------------------------------------------------------
          0.1   |  26-11-2021   | Primeira versão.
        --------------------------------------------------------------------------------
	________________________________________________________________________________

*/


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <stdint.h>
#include <inttypes.h>

#include <semaphore.h>          //For Shared Memory
#include <sys/mman.h>           //For Shared Memory
#include <sys/stat.h>           //For Shared Memory

#include <fcntl.h>              //Open File
#include <unistd.h>             //Close File

#include "cfg_buffer.h"         //Definitions for cfgCatcher_data_type
#include "opmode.h"
#include "getPos.h"
#include "confuse.h"            //Configuration file support (see: ../etc/HAX2.config)

#define AccessPermsIN 0644





        /***
        *     P R E L I M I N A R Y     F U N C T I O N S
        *****************************************************************/

void report_and_exit(const char *);




        /***
        *     M A I N    F U N C T I O N
        *****************************************************************/


int main(int argc , char *argv[])
{

        // ----------------------------------------
        // DECLARACAO VARIAVEIS DA FUNCAO PRINCIPAL
        // ----------------------------------------


        cfgBuffer_data * config_var             	                ;       //Structure for config variables, used for Shared Memory
        size_t ByteSize_cfg = sizeof(cfgBuffer_data)            	;       //Used for Shared Memory and to Write in file
        int fd_shmem_cfg                                            	;       //Used for Shared Memory
        sem_t * semptr_cfg                                          	;       //Used for Shared Memory


	opmode_data_type * opmode_var                           	;       //Structure for Operation Mode data for Shared Memory
	size_t ByteSize_opmode = sizeof(opmode_data_type)       	;       //Used for OpMode data for Shared Memory
	int fd_shmem_opmode                                     	;       //Used for OpMode data for Shared Memory
	sem_t * semptr_opmode                                   	;       //Used for OpMode data for Shared Memory


	
	// ----------------------------------------------------
        // VARIAVEIS DO CARREGAMENTO DO ARQUIVO DE CONFIGURAÇÃO
        // ----------------------------------------------------


	static char *BackingFileIN = "HAX-ConfigBuffer"         	;
        static char *SemaphoreNameIN = "HAX-ConfigSemaphore"    	;

        static char GetPos_BackingFile[csize]				;
        static char GetPos_SemaphoreName[csize]				;
        static long int GetPos_AccessPerms              = 0             ;

        static char OpMode_BackingFile[csize]				;
        static char OpMode_SemaphoreName[csize]				;
        static long int OpMode_AccessPerms              = 0             ;

	static long int RINGSIZE       		                        ;       //From config file - Specific



	// O P E N   C O N F I G   D A T A   -   S H A R E D   M E M O R Y
        // -----------------------------------------------------

        fd_shmem_cfg = shm_open(BackingFileIN, O_RDWR, AccessPermsIN);          // Empty to begin

        if (fd_shmem_cfg < 0)
        {
                report_and_exit("Can't get file descriptor for configuration data.\nTry execute 'cfgStore' first.");
        }

        // Get a pointer to memory
        config_var = mmap(NULL,
                        ByteSize_cfg,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fd_shmem_cfg,
                        0);

        if ( (void *) -1  == config_var)
        {
                report_and_exit("Can't get segment for shared memory for configuration data.");
        }


        // Create a semaphore for mutual exclusion
        semptr_cfg = sem_open(SemaphoreNameIN,          // name
                        O_CREAT,                        // create the semaphore
                        AccessPermsIN,                  // protection perms
                        0);                             // initial value

        if (semptr_cfg == (void*) -1)
        {
                report_and_exit("Can't open semaphore for config data");
        }

        // Use semaphore as a mutex (lock) by waiting for writer to increment it
        if (!sem_wait(semptr_cfg))              //Wait until semaphore != 0
        {
                strncpy(GetPos_BackingFile,config_var->GetPos_BackingFile,csize);
                strncpy(GetPos_SemaphoreName,config_var->GetPos_SemaphoreName,csize);
		strncpy(OpMode_BackingFile,config_var->OpMode_BackingFile,csize);
                strncpy(OpMode_SemaphoreName,config_var->OpMode_SemaphoreName,csize);
		GetPos_AccessPerms	= config_var->GetPos_AccessPerms;
                OpMode_AccessPerms      = config_var->OpMode_AccessPerms;
		RINGSIZE                = config_var->RINGSIZE;

                sem_post(semptr_cfg);
        }





        // O P E N    O P E R A T I O N   M O D E   -   S H A R E D   M E M O R Y
        // ----------------------------------------------------------------------

        fd_shmem_opmode = shm_open(OpMode_BackingFile,         // name from smem.h
                O_RDWR, 	                               // read
                OpMode_AccessPerms);                           // access permissions (0644)

        if (fd_shmem_opmode < 0)
        {
                report_and_exit("Can't get file descriptor for operation mode data.");
        }

        ftruncate(fd_shmem_opmode, ByteSize_opmode);            //get the bytes

        // Get a pointer to memory
        opmode_var = mmap(NULL,
                        ByteSize_opmode,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fd_shmem_opmode,
                        0);

        if ( (void *) -1  == opmode_var)
        {
                report_and_exit("Can't get segment for shared memory for operation mode data.");
        }


        // Create a semaphore for mutual exclusion
        semptr_opmode = sem_open(OpMode_SemaphoreName,          // name
                        O_CREAT,                                // create the semaphore
                        OpMode_AccessPerms,                     // protection perms
                        0);                                     // initial value

        if (semptr_opmode == (void*) -1)
        {
                report_and_exit("Can't open semaphore for operation mode data");
        }


        if (sem_post(semptr_opmode) < 0)
        {
                report_and_exit("Can't increment opmode semaphore to permit read.");
        }



	// GET POSITION RING BUFFER VARIABLES DEFINITION
	// ---------------------------------------------

	pos_data_type * rbpos_base_ptr		 			;	//Ring Buffer Pointer
	size_t RB_ByteSize                                      	;
	int fd_shmem                                            	;       //Used for Shared Memory
	sem_t * semptr                                         		;       //Used for Shared Memory
        RB_ByteSize = sizeof(pos_data_type) * RINGSIZE          	;       //Used for Shared Memory and to Write in file


        // O P E N    R I N G    B U F F E R    S H A R E D    M E M O R Y
        // ---------------------------------------------------------------

        fd_shmem = shm_open(GetPos_BackingFile,         // name from smem.h
                      O_RDWR,                		// read
                      GetPos_AccessPerms);              // access permissions (0644)

        if (fd_shmem < 0)
        {
                report_and_exit("Can't open shared mem segment for getPos data.");
        }

        ftruncate(fd_shmem, RB_ByteSize);               // get the bytes

        rbpos_base_ptr = mmap(NULL,                     // let system pick where to put segment
                        RB_ByteSize,                    // how many bytes
                        PROT_READ | PROT_WRITE,         // access protections
                        MAP_SHARED,                     // mapping visible to other processes
                        fd_shmem,                       // file descriptor
                        0);                             // offset: start at 1st byte

        // Initializing the pointers
        if ( (void *) -1  == rbpos_base_ptr)
        {
                report_and_exit("Can't get segment for getPos shared memory.");
        }


        /**  Semaphore code to lock the shared mem  **/
        semptr = sem_open(GetPos_SemaphoreName,         // name
                        O_CREAT,                        // create the semaphore
                        GetPos_AccessPerms,             // protection perms
                        0);                             // initial value

        if (semptr == (void*) -1)
        {
                report_and_exit("sem_open");
        }




        //Cleanup Config Data Shared Memory Stuff
        munmap(config_var, ByteSize_cfg);
        close(fd_shmem_cfg);
        sem_close(semptr_cfg);

	//Cleanup Op_Mode Shared Memory Stuff
        munmap(opmode_var, ByteSize_opmode);
        close(fd_shmem_opmode);
        sem_close(semptr_opmode);

        //Cleanup GetPos Shared Memory Stuff
        munmap(rbpos_base_ptr, RB_ByteSize);
        close(fd_shmem);
        sem_close(semptr);



	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
	//								 //
	//	D E L E T E   A L L  S H A R E D  M E M O R I E S	 //
	//								 //
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
	


	shm_unlink(BackingFileIN);
	shm_unlink(SemaphoreNameIN);

	shm_unlink(OpMode_BackingFile);
	shm_unlink(OpMode_SemaphoreName);
	 
	shm_unlink(GetPos_BackingFile);
	shm_unlink(GetPos_SemaphoreName);

	printf("All Shared Memories Deleted!\n\n");


	return 0;

}










// Report and Exit

void report_and_exit(const char* msg)
{
        perror(msg);
        exit(-1);
}




