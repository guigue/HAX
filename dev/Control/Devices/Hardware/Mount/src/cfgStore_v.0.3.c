/*
	================================================================================
	                     Universidade Presbiteriana Mackenzie
	         Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
	================================================================================

	Configuration Store - version 0.3
	--------------------------------------------------------------------------------
	Programa implementado para capturar as configuracoes de um arquivo texto e 
	disponibiliza-las em uma memoria compartilhada para que os demais programas
	possam utilizar as informacoes.
	--------------------------------------------------------------------------------

	Autores: Tiago Giorgetti
		 Guillermo Gimenez de Castro
	Emails:  tiago.giorgetti@craam.mackenzie.br
	         guigue@craam.mackenzie.br

	--------------------------------------------------------------------------------

	Histórico:
	________________________________________________________________________________
	 Versão	|  Data		|	Atualização
	--------------------------------------------------------------------------------
	  0.1	|  16-02-2021	| Primeira versão.
	--------------------------------------------------------------------------------
	  0.2	|  25-03-2021	| Correção de falha na armazenagem da shared Memory
	 	| 		| Guigue: Realizou diversas modificacoes para corrigir 
		|		| o problema. 
		|		|	* criando estrutura com tamanho fixo em
		|		| 	cfg_buffer.h
		|		|	* linha 80, pegando tamanho da estrutura
	       	|		|	* linhas 189-195, utilizacao da funcao strncpy()
	--------------------------------------------------------------------------------
	  0.3	|  05-04-2021	| Pequenas modificacoes para organizar nomenclatura de
  		| 		| variaveis e demais notas de esclarecimento no codigo.
        --------------------------------------------------------------------------------
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


	cfgBuffer_data * config_var				;	//Structure for config variables, used for Shared Memory 
	
	size_t ByteSize = sizeof(cfgBuffer_data)		;	//Used for Shared Memory and to Write in file
	int fd_shmem						;	//Used for Shared Memory
	sem_t * semptr						;	//Used for Shared Memory

	static char *BackingFileIN = "HAX-ConfigBuffer"		;
	static char *SemaphoreNameIN = "HAX-ConfigSemaphore"	;

								
	// ----------------------------------------------------
	// VARIAVEIS DO CARREGAMENTO DO ARQUIVO DE CONFIGURAÇÃO
        // ----------------------------------------------------

	static char *IP_SERVER	                        = NULL          ;
        static long int TCP_PORT 			= 0		;
        static long int RCV_BUFFER_SIZE 		= 0		;
        static long int TX_DELAY 			= 0		;

        static char *DIRECTORY_LOG 			= NULL		;

	static char *GetPos_BackingFile			= NULL		;
        static char *GetPos_SemaphoreName		= NULL		;
        static long int GetPos_AccessPerms		= 0		;

        static long int RINGSIZE			= 0		;
        static char *DATAFILENAME			= NULL		;

	static char *OpMode_BackingFile			= NULL		;
        static char *OpMode_SemaphoreName		= NULL		;
        static long int OpMode_AccessPerms		= 0		;


	//    S H A R E D     M E M O R Y 
	// -----------------------------------

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
        if( cfg_parse(cfg, "../etc/HAX.config") == CFG_FILE_ERROR)
        {
                printf("\nCan´t open configuration file.\n\n");
                return 1;
        }
        cfg_free(cfg);

	//------------- Storage Code ---------------------------------------
	//------------------------------------------------------------------
	// strncpy() for strings variables (csize from cfg_buffer.h)

	strncpy(config_var->IP_SERVER,IP_SERVER,csize)		           	;
	strncpy(config_var->DIRECTORY_LOG,DIRECTORY_LOG,csize)		   	;		
	strncpy(config_var->GetPos_BackingFile,GetPos_BackingFile,csize)	;
	strncpy(config_var->GetPos_SemaphoreName,GetPos_SemaphoreName,csize) 	;
	strncpy(config_var->DATAFILENAME,DATAFILENAME,csize)		   	;
	strncpy(config_var->OpMode_BackingFile,OpMode_BackingFile,csize)     	;
	strncpy(config_var->OpMode_SemaphoreName,OpMode_SemaphoreName,csize) 	;

	config_var->GetPos_AccessPerms          = GetPos_AccessPerms	   	;
	config_var->TCP_PORT 			= TCP_PORT		   	;
	config_var->RCV_BUFFER_SIZE 		= RCV_BUFFER_SIZE	   	;
	config_var->TX_DELAY			= TX_DELAY		   	;
	config_var->RINGSIZE 			= RINGSIZE		   	;
	config_var->OpMode_AccessPerms          = OpMode_AccessPerms       	;

	//------------------------------------------------------------------
	//------ Increment the semaphore so that memreader can read  -------

	if (sem_post(semptr) < 0)
	{
		report_and_exit("Can't increment semaphore to permit read.");
	}

	//------------------------------------------------------------------
	//----------- Clean up ---------------------------------------------
	

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



