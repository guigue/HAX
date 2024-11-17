/*
        ================================================================================
        	           Universidade Presbiteriana Mackenzie
        	Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        ================================================================================

        Point and Track v.0.1
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

        Usage:

        # ptrack <object>

        Objetos disponíveis: Sun.

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

#include "getPos.h"


        /***
        *     M A I N    F U N C T I O N
        *****************************************************************/

int main(int argc , char *argv[])
{

	int i					;	//Generic counter
	int sock				;   	//Socket variable
	struct sockaddr_in server		;   	//Socket variable
	char server_reply[RCV_BUFFER_SIZE] 	;	//Socket variable
	int command_nlines = 34			;   	// Getter commands number of lines from set_data[].
                                            		// If you add a command remember to modify, add, etc a new
							// command_nlines

	char* set_data[] =
	{
		"/* Java Script */								",
		"/* Socket Start Packet */							",
		"function wait(ms){								",
		"        var start = new Date().getTime();					",
		"        var end = start;							",
		"        while(end < start + ms){						",
		"                end = new Date().getTime();					",
		"        }									",
		"}										",
		"var Out;									",
		"var err;									",
		"sky6StarChart.LASTCOMERROR = 0;						",
		"sky6StarChart.Find('Sun');							",
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
		"        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, 'Sun');		",
		"        var slewComplete = sky6Web.IsSlewComplete;				",
		"        while(slewComplete != 1){						",
		"                slewComplete = sky6Web.IsSlewComplete;				",
		"        }									",
		"        sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);	",
		"}										",
		"Out = 'Slew to Sun completed. Tracking on RaDec rates.';			",
		"/* Socket End Packet */							"
	};


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

      	/************************************************************/
      	//SEND DATA
      	for(i = 0; i<command_nlines ; i++)
        {
        	if( send(sock , set_data[i] , strlen(set_data[i]) , 0) < 0)
            	{
              		printf("Could not send data to remote socket server.\n");
              		return -1;
            	}
        	usleep(TX_DELAY);
        }

      	//Receive a reply from the server
      	if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        {
        	printf("Could not receive data from socket server.\n");
          	return -1;
        }

      	//=================================================================
      	close(sock);
      	//=================================================================


  	printf("Server Reply: %s\n", server_reply);

  	return 0;
}

