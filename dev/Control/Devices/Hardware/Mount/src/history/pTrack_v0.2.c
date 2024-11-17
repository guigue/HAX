/*
        ================================================================================
        	           Universidade Presbiteriana Mackenzie
        	Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        ================================================================================

        Point and Track v.0.2
        --------------------------------------------------------------------------------
        Este programa visa realizar o apontamento para o objeto (slew) e posteriormente
	realizar o acompanhamento (track) utilizando as definicoes de track (velocidade)
	de acordo com o tracking rate fornecido pelo TheSkyX do objeto.

        Used classes:
	--------------------------------------------------------------------------------
        sky6RASCOMTele
	sky6ObjectInformation

        Autor: Tiago Giorgetti
        Email: tiago.giorgetti@craam.mackenzie.br

        Histórico:
        ________________________________________________________________________________
         Versão |  Data         |       Atualização
        --------------------------------------------------------------------------------
          0.1   |  05-03-2020   | Primeira versão, realiza a função e apresenta na tela
                |               | informação do servidor com a conclusão da instrução.
        --------------------------------------------------------------------------------
	  0.2	|  05-03-2020   | Implementação de argumentos na execução com opcoes
                |               | de objetos para o apontamento e posterior tracking.
	--------------------------------------------------------------------------------
                |               | Implementação de shared memory para gravação de log
                |               | com informações de status.
	--------------------------------------------------------------------------------

        Usage:

        # pTrack <object>

        Objetos disponíveis: Sun, Moon, Jupiter.

	--------------------------------------------------------------------------------

*/


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <stdint.h>
#include <inttypes.h>

//#include <semaphore.h>  //For Shared Memory
//#include <sys/mman.h>   //For Shared Memory
//#include <sys/stat.h>   //For Shared Memory

#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <fcntl.h>      //open(sock) or file
#include <unistd.h>     //close(sock) or write to a file descriptor

#include <time.h>       //usleep() to timing socket message transfer

#include "pTrack.h"


        /***
        *     M A I N    F U N C T I O N
        *****************************************************************/

int main(int argc , char *argv[])
{

        if (argc != 2)
        {
                printf("\n\tUsage: pTrack <object> \n");
                printf("\t\tObject: Sun, Moon or Jupiter\n\n");
                exit(0);
        }


	int i					;	//Generic counter
	int sock				;   	//Socket variable
	struct sockaddr_in server		;   	//Socket variable
	char server_reply[RCV_BUFFER_SIZE] 	;	//Socket variable
	int command_nlines = 28			;   	// Getter commands number of lines from set_data[].
                                            		// If you add a command remember to modify, add, etc a new
							// command_nlines

//	char * p;
//	const char sep[2] =";";
	//  double jd , sid , alt, az , ra , dec , tra , tdec;

//	unsigned long long rb_ctr = 1                         ;       //Ring Buffer Counter


/*	pos_data_type * rbpos_base_ptr, * rbpos_ptr ;
	size_t ByteSize = sizeof(pos_data_type) * RINGSIZE    ;       //Used for Shared Memory and to Write in file
	int fd_shmem                                          ;       //Used for Shared Memory
	sem_t * semptr                                        ;       //Used for Shared Memory

	int fd_data, fd_data_w                                ;       //Used to Open and Write a File
	char *filename = DATAFILENAME                         ;       //Used to Open/Create a File
*/

	char* set_Sun_data[] =
	{
		"/* Java Script */								",
		"/* Socket Start Packet */							",
		"var Out;									",
		"var err;									",
		"var obj='Sun';									",
		"sky6StarChart.LASTCOMERROR = 0;						",
		"sky6StarChart.Find(obj);							",
		"err = sky6StarChart.LASTCOMERROR;						",
		"if (err != 0){									",
		"        Out = Target + ' not found.';						",
		"}else{										",
		"        sky6ObjectInformation.Property(54);					",
		"        var targetRA = sky6ObjectInformation.ObjInfoPropOut;			",
		"        sky6ObjectInformation.Property(55);					",
		"        var targetDec = sky6ObjectInformation.ObjInfoPropOut;			",
		"        sky6ObjectInformation.Property(77);					",
		"        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;		",
		"        sky6ObjectInformation.Property(78);					",
		"        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;		",
		"        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, obj);			",
		"        var slewComplete = sky6Web.IsSlewComplete;				",
		"        while(slewComplete != 1){						",
		"                slewComplete = sky6Web.IsSlewComplete;				",
		"        }									",
		"        sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);	",
		"}										",
		"Out = 'Slew to ' + obj + ' completed. Tracking on RaDec rates.';		",
		"/* Socket End Packet */							"
	};

        char* set_Moon_data[] =
        {
                "/* Java Script */                                                              ",
                "/* Socket Start Packet */                                                      ",
                "var Out;                                                                       ",
                "var err;									",
		"var obj='Moon';                                                                ",
                "sky6StarChart.LASTCOMERROR = 0;                                                ",
                "sky6StarChart.Find(obj);                                                       ",
                "err = sky6StarChart.LASTCOMERROR;                                              ",
                "if (err != 0){                                                                 ",
                "        Out = Target + ' not found.';                                          ",
                "}else{                                                                         ",
                "        sky6ObjectInformation.Property(54);                                    ",
                "        var targetRA = sky6ObjectInformation.ObjInfoPropOut;                   ",
                "        sky6ObjectInformation.Property(55);                                    ",
                "        var targetDec = sky6ObjectInformation.ObjInfoPropOut;                  ",
                "        sky6ObjectInformation.Property(77);                                    ",
                "        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;            ",
                "        sky6ObjectInformation.Property(78);                                    ",
                "        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;           ",
                "        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, obj); 		        ",
                "        var slewComplete = sky6Web.IsSlewComplete;                             ",
                "        while(slewComplete != 1){                                              ",
                "                slewComplete = sky6Web.IsSlewComplete;                         ",
                "        }                                                                      ",
                "        sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);   ",
                "}                                                                              ",
                "Out = 'Slew to ' + obj + ' completed. Tracking on RaDec rates.';               ",
                "/* Socket End Packet */                                                        "
        };

        char* set_Jupiter_data[] =
        {
                "/* Java Script */                                                              ",
                "/* Socket Start Packet */                                                      ",
                "var Out;                                                                       ",
                "var err;                                                                       ",
                "var obj='Jupiter';                                                             ",
                "sky6StarChart.LASTCOMERROR = 0;                                                ",
                "sky6StarChart.Find(obj);                                                       ",
                "err = sky6StarChart.LASTCOMERROR;                                              ",
                "if (err != 0){                                                                 ",
                "        Out = Target + ' not found.';                                          ",
                "}else{                                                                         ",
                "        sky6ObjectInformation.Property(54);                                    ",
                "        var targetRA = sky6ObjectInformation.ObjInfoPropOut;                   ",
                "        sky6ObjectInformation.Property(55);                                    ",
                "        var targetDec = sky6ObjectInformation.ObjInfoPropOut;                  ",
                "        sky6ObjectInformation.Property(77);                                    ",
                "        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;            ",
                "        sky6ObjectInformation.Property(78);                                    ",
                "        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;           ",
                "        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, obj);                  ",
                "        var slewComplete = sky6Web.IsSlewComplete;                             ",
                "        while(slewComplete != 1){                                              ",
                "                slewComplete = sky6Web.IsSlewComplete;                         ",
                "        }                                                                      ",
                "        sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);   ",
                "}                                                                              ",
                "Out = 'Slew to ' + obj + ' completed. Tracking on RaDec rates.';               ",
                "/* Socket End Packet */                                                        "
        };




	/***** S H A R E D  M E M O R Y ********************************************************/
/*
	fd_shmem = shm_open(BackingFile,	// name from smem.h
        	O_RDWR | O_CREAT,         	// read/write, create if needed
     		AccessPerms);             	// access permissions (0644)

  	if (fd_shmem < 0) report_and_exit("Can't open shared mem segment...");

  	ftruncate(fd_shmem, ByteSize);          // get the bytes

  	rbpos_base_ptr = mmap(NULL,             // let system pick where to put segment
        	ByteSize,                       // how many bytes
                PROT_READ | PROT_WRITE,         // access protections
                MAP_SHARED,                     // mapping visible to other processes
                fd_shmem,                       // file descriptor
                0);                             // offset: start at 1st byte

	if ( (void *) -1  == rbpos_base_ptr)
    	report_and_exit("Can't get segment for shared memory...");
  	else
    	rbpos_ptr = rbpos_base_ptr;

  	//  Semaphore code to lock the shared mem
  	semptr = sem_open(SemaphoreName,      	// name
        	O_CREAT,                        // create the semaphore
                AccessPerms,                    // protection perms
                0);                             // initial value

  	if (semptr == (void*) -1) report_and_exit("sem_open");
*/
	/***** F I L E  O P E N  ***************************************************************/
/*
	if ( (fd_data = open(filename, O_RDWR | O_CREAT | O_APPEND, AccessPerms)) == -1)
	{
		fprintf(stderr, "Cannot open getposition data file. Try again later.\n");
		exit(1);
	}

*/

	//CREATING THE SOCKET
      	sock = socket(AF_INET , SOCK_STREAM , 0);
      	if (sock == -1)
        {
        	printf("Could not create socket.\n");
        }

      	server.sin_addr.s_addr = inet_addr(IP_SERVER);
      	server.sin_family = AF_INET;
      	server.sin_port = htons(TCP_PORT);

      	//CONNECT TO REMOTE SERVER
      	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
        {
        	printf("Could not connect to remote socket server.\n");
          	return 1;
        }


	//SENDING DATA TO SERVER BASED ON ARGUMENTS
	if (strcmp(argv[1],"Sun") == 0)
	{
		/************************************************************/
      		//SEND DATA
      		for(i = 0; i<command_nlines ; i++)
        	{
			if( send(sock , set_Sun_data[i] , strlen(set_Sun_data[i]) , 0) < 0)
            		{
              			printf("Could not send data to remote socket server.\n");
              			return -1;
            		}
        		usleep(TX_DELAY);
        	}
	}else if (strcmp(argv[1],"Moon") == 0)
	{
                /************************************************************/
                //SEND DATA
                for(i = 0; i<command_nlines ; i++)
                        {
                                if( send(sock , set_Moon_data[i] , strlen(set_Moon_data[i]) , 0) < 0)
                                {
                                        printf("Could not send data to remote socket server.\n");
                                        return -1;
                                }
                        usleep(TX_DELAY);
                }
	}else if (strcmp(argv[1],"Jupiter") == 0)
	{
                /************************************************************/
                //SEND DATA
                for(i = 0; i<command_nlines ; i++)
                        {
                                if( send(sock , set_Jupiter_data[i] , strlen(set_Jupiter_data[i]) , 0) < 0)
                                {
                                        printf("Could not send data to remote socket server.\n");
                                        return -1;
                                }
                        usleep(TX_DELAY);
                }
	}else{
                printf("\n\tUsage: pTrack <object> \n");
                printf("\t\tObject: Sun, Moon or Jupiter\n\n");
		exit(0);
	}


      	//RECEIVE A REPLY FROM THE SERVER
      	if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        {
        	printf("Could not receive data from socket server.\n");
          	return -1;
        }

      	//=================================================================
      	close(sock);
      	//=================================================================


  	printf("\nServer Reply: %s\n\n", server_reply);

  	return 0;
}

