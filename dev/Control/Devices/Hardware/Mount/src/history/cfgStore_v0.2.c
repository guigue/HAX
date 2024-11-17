/*
	================================================================================
	                     Universidade Presbiteriana Mackenzie
	         Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
	================================================================================

	Configuration Store - version 0.1
	--------------------------------------------------------------------------------
	Programa implementado para capturar as configuracoes de um arquivo texto e 
	disponibiliza-las em uma memoria compartilhada para que os demais programas
	possam utilizar as informacoes.
	--------------------------------------------------------------------------------

	Autor: Tiago Giorgetti
	Email: tiago.giorgetti@craam.mackenzie.br

	--------------------------------------------------------------------------------

	Histórico:
	________________________________________________________________________________
	 Versão	|  Data		|	Atualização
	--------------------------------------------------------------------------------
	  0.1	|  16-02-2021	| Primeira versão.
	--------------------------------------------------------------------------------
		|		| 
	________|_______________|_______________________________________________________

*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <stdint.h>
#include <inttypes.h>

#include <semaphore.h>		//For Shared Memory
#include <sys/mman.h>   	//For Shared Memory
#include <sys/stat.h>   	//For Shared Memory

#include <fcntl.h>		//Open File
#include <unistd.h>		//Close File

#include "cfg_buffer.h"		//Definitions for cfgCatcher_data_type
#include "confuse.h"   		//Configuration file support (see: ../etc/HAX2.config)

#define AccessPermsIN 0644



        /***
        *     P R E L I M I N A R Y    F U N C T I O N S
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


	cfgBuffer_data_confusetype * config_var		; 
//	cfgBuffer_data_shmtype * config_var			;
//	size_t csize				= 50		;	//Char size for Shared Memory
	
	size_t ByteSize = sizeof(config_var)	 		;	//Used for Shared Memory and to Write in file
	int fd_shmem						;	//Used for Shared Memory
	sem_t * semptr						;	//Used for Shared Memory

	static char *BackingFileIN = "HAXConfigBuffer"		;
	static char *SemaphoreNameIN = "HAXConfigSemaphore"	;
//	static long int AccessPermsIN = 0644  			;

//	int fd_data, fd_data_w                                	;	//Used to Open and Write a Binary Data File
//	char *filename = "config_data.bin"			;	//Used to Open/Create a Binary Data File
	
								
	// ----------------------------------------------------
	// VARIAVEIS DO CARREGAMENTO DO ARQUIVO DE CONFIGURAÇÃO
        // ----------------------------------------------------

	static char *IP_SERVER 				= NULL		;
//	static char IP_SERVER_FIX[csize]		= NULL		;
        static long int TCP_PORT 			= 0		;
        static long int RCV_BUFFER_SIZE 		= 0		;
        static long int TX_DELAY 			= 0		;

        static char *DIRECTORY_LOG 			= NULL		;
//        static char DIRECTORY_LOG_FIX[csize]		= NULL		;

	static char *GetPos_BackingFile			= NULL		;
//	static char GetPos_BackingFile_FIX[csize]	= NULL		;
        static char *GetPos_SemaphoreName		= NULL		;
//	static char GetPos_SemaphoreName_FIX[csize]	= NULL		;
        static long int GetPos_AccessPerms		= 0		;

        static long int RINGSIZE			= 0		;
        static char *DATAFILENAME			= NULL		;
//	static char DATAFILENAME_FIX[csize]		= NULL		;

	static char *OpMode_BackingFile			= NULL		;
//	static char OpMode_BackingFile_FIX[csize]	= NULL		;
        static char *OpMode_SemaphoreName		= NULL		;
//	static char OpMode_SemaphoreName_FIX[csize]	= NULL		;
        static long int OpMode_AccessPerms		= 0		;





	//    S H A R E D     M E M O R Y 
	// -----------------------------------

	//If has created, clean up:
//	close(fd_shmem);
//	sem_close(semptr);
//	shm_unlink(BackingFileIN);


	fd_shmem = shm_open(BackingFileIN,      	// name from smem.h 
		      O_RDWR | O_CREAT, 		// read/write, create if needed 
		      AccessPermsIN);   	  	// access permissions (0644) 

	if (fd_shmem < 0)
	{
		report_and_exit("Can't open shared memory segment.\n");
	}

	ftruncate(fd_shmem, ByteSize); 			// get the bytes 

	config_var = mmap(NULL, 			// let system pick where to put segment 
			ByteSize,   			// how many bytes 
			PROT_READ | PROT_WRITE,		// access protections 
			MAP_SHARED, 			// mapping visible to other processes 
			fd_shmem,      			// file descriptor 
			0);         			// offset: start at 1st byte 

	if ( (void *) -1  == config_var)
	{
		report_and_exit("Can't get segment for shared memory.\n");
	}

	// Semaphore code to lock the shared mem 
	semptr = sem_open(SemaphoreNameIN, 		// name 
			O_CREAT,       			// create the semaphore 
			AccessPermsIN, 			// protection perms 
			0);            			// initial value 

	if (semptr == (void*) -1)
	{
		report_and_exit("Can't open semaphore for some reason.\n");
	}

	// File Open - only if it is necessary to create a binary file.
//	if ( (fd_data = open(filename, O_RDWR | O_CREAT | O_APPEND, AccessPermsIN)) == -1)
//	{
//		fprintf(stderr, "Cannot open getposition data file. Try again later.\n");
//		exit(1);
//	}



	//------------------------------------------------------------------
	//------------- Reading Text File Configuration --------------------
	//--------------------- via confuse.h ------------------------------

       	cfg_t *cfg;
        cfg_opt_t opts[] =
        {
                CFG_SIMPLE_STR ("IP_SERVER", &IP_SERVER),
                CFG_SIMPLE_INT ("TCP_PORT", &TCP_PORT),
                CFG_SIMPLE_INT ("RCV_BUFFER_SIZE", &RCV_BUFFER_SIZE),
                CFG_SIMPLE_INT ("TX_DELAY", &TX_DELAY),

                CFG_SIMPLE_STR ("DIRECTORY_LOG", &DIRECTORY_LOG),
                
		CFG_SIMPLE_STR ("GetPos_BackingFile", &GetPos_BackingFile),
                CFG_SIMPLE_STR ("GetPos_SemaphoreName", &GetPos_SemaphoreName),
                CFG_SIMPLE_INT ("GetPos_AccessPerms", &GetPos_AccessPerms),

                CFG_SIMPLE_INT ("RINGSIZE", &RINGSIZE),
                CFG_SIMPLE_STR ("DATAFILENAME", &DATAFILENAME),

		CFG_SIMPLE_STR ("OpMode_BackingFile", &OpMode_BackingFile),
                CFG_SIMPLE_STR ("OpMode_SemaphoreName", &OpMode_SemaphoreName),
                CFG_SIMPLE_INT ("OpMode_AccessPerms", &OpMode_AccessPerms),

        
		CFG_END()
        };
	
        cfg = cfg_init(opts, 0);
        if( cfg_parse(cfg, "/opt/HAX/Control/Devices/Hardware/Mount/etc/HAX2.config") == CFG_FILE_ERROR)
        {
                printf("\nCan´t open configuration file.\n\n");
                return 1;
        }
        cfg_free(cfg);

	
	printf("\n==============================================\n");
	printf("=======1==1====1====1===========================\n");
	
	printf("Below, Value of local variables after text file reading!\n"); 	

	printf("IP_SERVER            = %s\n",IP_SERVER)		;
	printf("TCP_PORT             = %ld\n",TCP_PORT)		;
	printf("RCV_BUFFER_SIZE      = %ld\n",RCV_BUFFER_SIZE)		;
	printf("TX_DELAY	     = %ld\n",TX_DELAY)		;
	printf("DIRECTORY_LOG        = %s\n",DIRECTORY_LOG)		;
	printf("GetPos_BackingFile   = %s\n",GetPos_BackingFile) 	;
	printf("GetPos_SemaphoreName = %s\n",GetPos_SemaphoreName) 	;
	printf("GetPos_AccessPerms   = %ld\n",GetPos_AccessPerms) 	;
	printf("RINGSIZE             = %ld\n",RINGSIZE) 		;
	printf("DATAFILENAME         = %s\n",DATAFILENAME) 		;
	printf("OpMode_BackingFile   = %s\n",OpMode_BackingFile) 	;
	printf("OpMode_SemaphoreName = %s\n",OpMode_SemaphoreName) 	;
	printf("OpMode_AccessPerms   = %ld\n\n",OpMode_AccessPerms) 	;

	printf("Below, the address of local variables after text file reading!\n\n"); 	
	
	printf("IP_SERVER            = %s\n",&IP_SERVER)		;
	printf("TCP_PORT             = %ld\n",&TCP_PORT)		;
	printf("RCV_BUFFER_SIZE      = %ld\n",&RCV_BUFFER_SIZE)		;
	printf("TX_DELAY	     = %ld\n",&TX_DELAY)		;
	printf("DIRECTORY_LOG        = %s\n",&DIRECTORY_LOG)		;
	printf("GetPos_BackingFile   = %s\n",&GetPos_BackingFile) 	;
	printf("GetPos_SemaphoreName = %s\n",&GetPos_SemaphoreName) 	;
	printf("GetPos_AccessPerms   = %ld\n",&GetPos_AccessPerms) 	;
	printf("RINGSIZE             = %ld\n",&RINGSIZE) 		;
	printf("DATAFILENAME         = %s\n",&DATAFILENAME) 		;
	printf("OpMode_BackingFile   = %s\n",&OpMode_BackingFile) 	;
	printf("OpMode_SemaphoreName = %s\n",&OpMode_SemaphoreName) 	;
	printf("OpMode_AccessPerms   = %ld\n",&OpMode_AccessPerms) 	;






	//------------- Storage Code ---------------------------------------
	//------------------------------------------------------------------

	




	//------------- Storage Code ---------------------------------------
	//------------------------------------------------------------------

	config_var->IP_SERVER 			= IP_SERVER		;
	config_var->TCP_PORT 			= TCP_PORT		;
	config_var->RCV_BUFFER_SIZE 		= RCV_BUFFER_SIZE	;
	config_var->TX_DELAY			= TX_DELAY		;
	config_var->DIRECTORY_LOG 		= DIRECTORY_LOG		;
	config_var->GetPos_BackingFile 		= GetPos_BackingFile	;
	config_var->GetPos_SemaphoreName 	= GetPos_SemaphoreName	;

	config_var->GetPos_AccessPerms 		= GetPos_AccessPerms	;
	config_var->RINGSIZE 			= RINGSIZE		;
	config_var->DATAFILENAME 		= DATAFILENAME		;
	config_var->OpMode_BackingFile 		= OpMode_BackingFile	;
	config_var->OpMode_SemaphoreName 	= OpMode_SemaphoreName	;
	config_var->OpMode_AccessPerms 		= OpMode_AccessPerms	;




/*
	snprintf(config_confuse_var->IP_SERVER,csize,"%s%",IP_SERVER)				;
	snprintf(config_confuse_var->TCP_PORT,csize,"%ld%",TCP_PORT)				;
	snprintf(config_confuse_var->RCV_BUFFER_SIZE,csize,"%ld%",RCV_BUFFER_SIZE)		;
	snprintf(config_confuse_var->TX_DELAY,csize,"%ld",TX_DELAY)				;
	snprintf(config_confuse_var->DIRECTORY_LOG,csize,"%s%",DIRECTORY_LOG)			;
	snprintf(config_confuse_var->GetPos_BackingFile,csize,"%s%",GetPos_BackingFile)		;
	snprintf(config_confuse_var->GetPos_SemaphoreName,csize,"%s%",GetPos_SemaphoreName)	;
	snprintf(config_confuse_var->GetPos_AccessPerms,csize,"%ld%",GetPos_AccessPerms)	;
	snprintf(config_confuse_var->RINGSIZE,csize,"%ld%",RINGSIZE)				;
	snprintf(config_confuse_var->DATAFILENAME,csize,"%s%",DATAFILENAME)			;
	snprintf(config_confuse_var->OpMode_BackingFile,csize,"%s%",OpMode_BackingFile)		;
	snprintf(config_confuse_var->OpMode_SemaphoreName,csize,"%s%",OpMode_SemaphoreName)	;
	snprintf(config_confuse_var->OpMode_AccessPerms,csize,"%ld%",OpMode_AccessPerms)	;


	snprintf(config_var->IP_SERVER,csize,"%s%",config_confuse_var->IP_SERVER)			;
	snprintf(config_var->TCP_PORT,csize,"%ld%",config_confuse_var->TCP_PORT)			;
	snprintf(config_var->RCV_BUFFER_SIZE,csize,"%ld%",config_confuse_var->RCV_BUFFER_SIZE)		;
	snprintf(config_var->TX_DELAY,csize,"%ld",config_confuse_var->TX_DELAY)				;
	snprintf(config_var->DIRECTORY_LOG,csize,"%s%",config_confuse_var->DIRECTORY_LOG)		;
	snprintf(config_var->GetPos_BackingFile,csize,"%s%",config_confuse_var->GetPos_BackingFile)	;
	snprintf(config_var->GetPos_SemaphoreName,csize,"%s%",config_confuse_var->GetPos_SemaphoreName)	;
	snprintf(config_var->GetPos_AccessPerms,csize,"%ld%",config_confuse_var->GetPos_AccessPerms)	;
	snprintf(config_var->RINGSIZE,csize,"%ld%",config_confuse_var->RINGSIZE)			;
	snprintf(config_var->DATAFILENAME,csize,"%s%",config_confuse_var->DATAFILENAME)			;
	snprintf(config_var->OpMode_BackingFile,csize,"%s%",config_confuse_var->OpMode_BackingFile)	;
	snprintf(config_var->OpMode_SemaphoreName,csize,"%s%",config_confuse_var->OpMode_SemaphoreName)	;
	snprintf(config_var->OpMode_AccessPerms,csize,"%ld%",config_confuse_var->OpMode_AccessPerms)	;

*/

	//------------------------------------------------------------------
	//------ Increment the semaphore so that memreader can read  -------

	if (sem_post(semptr) < 0)
	{
		report_and_exit("Can't increment semaphore to permit read.");
	}


	//Write the File - Only necessaty if want to save a binary file
//	if ( (fd_data_w = write(fd_data , config_var , ByteSize)) < ByteSize)
//	{
//		perror("Problems writing the file");
//		exit(1);
//	}
	

	// Only for testing

	printf("\n==============================================\n");
	printf("========2=====2======2=======2==================\n");

	printf("Configuration was recorded in Shared Memory!\n\n"); 	

	printf("IP_SERVER            = %s \n",config_var->IP_SERVER)		;
	printf("TCP_PORT             = %ld\n",config_var->TCP_PORT)		;
	printf("RCV_BUFFER_SIZE      = %ld\n",config_var->RCV_BUFFER_SIZE)	;
	printf("TX_DELAY	     = %ld\n",config_var->TX_DELAY)		;
	printf("DIRECTORY_LOG        = %s \n",config_var->DIRECTORY_LOG)	;
	printf("GetPos_BackingFile   = %s \n",config_var->GetPos_BackingFile) 	;
	printf("GetPos_SemaphoreName = %s \n",config_var->GetPos_SemaphoreName) ;
	printf("GetPos_AccessPerms   = %ld\n",config_var->GetPos_AccessPerms) 	;
	printf("RINGSIZE             = %ld\n",config_var->RINGSIZE) 		;
	printf("DATAFILENAME         = %s \n",config_var->DATAFILENAME) 	;
	printf("OpMode_BackingFile   = %s \n",config_var->OpMode_BackingFile) 	;
	printf("OpMode_SemaphoreName = %s \n",config_var->OpMode_SemaphoreName) ;
	printf("OpMode_AccessPerms   = %ld\n",config_var->OpMode_AccessPerms) ;

	printf("\nBelow, the address of memories!\n\n"); 	

	printf("IP_SERVER            = %s\n",&config_var->IP_SERVER)		;
	printf("TCP_PORT             = %ld\n",&config_var->TCP_PORT)		;
	printf("RCV_BUFFER_SIZE      = %ld\n",&config_var->RCV_BUFFER_SIZE)	;
	printf("TX_DELAY	     = %ld\n",&config_var->TX_DELAY)		;
	printf("DIRECTORY_LOG        = %s\n",&config_var->DIRECTORY_LOG)	;
	printf("GetPos_BackingFile   = %s\n",&config_var->GetPos_BackingFile) 	;
	printf("GetPos_SemaphoreName = %s\n",&config_var->GetPos_SemaphoreName) ;
	printf("GetPos_AccessPerms   = %ld\n",&config_var->GetPos_AccessPerms) 	;
	printf("RINGSIZE             = %ld\n",&config_var->RINGSIZE) 		;
	printf("DATAFILENAME         = %s\n",&config_var->DATAFILENAME) 	;
	printf("OpMode_BackingFile   = %s\n",&config_var->OpMode_BackingFile) 	;
	printf("OpMode_SemaphoreName = %s\n",&config_var->OpMode_SemaphoreName) ;
	printf("OpMode_AccessPerms   = %ld\n",&config_var->OpMode_AccessPerms) ;



	//------------------------------------------------------------------
	//----------- Clean up ---------------------------------------------
	

//	close(fd_data);				//If binary file was created
//	munmap(config_var, ByteSize); 		//unmap the storage
	return 0;
}


       
	/***
        *     O T H E R S    F U N C T I O N S
        *****************************************************************/


/* Report and Exit */

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);
}



