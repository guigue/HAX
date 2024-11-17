#include <stdio.h>
#include "confuse.h"



int main(void)
{

        // CARREGA O ARQUIVO DE CONFIGURAÇÃO

        char *IP_SERVER = NULL;
	char *DIRECTORY_LOG = NULL;
        long int TCP_PORT = 0;
        long int RCV_BUFFER_SIZE = 0;
        long int TX_DELAY = 0;


        cfg_t *cfg;
        cfg_opt_t opts[] =
        {
                CFG_SIMPLE_STR ("IP_SERVER", &IP_SERVER),
                CFG_SIMPLE_INT ("TCP_PORT", &TCP_PORT),
                CFG_SIMPLE_INT ("RCV_BUFFER_SIZE", &RCV_BUFFER_SIZE),
                CFG_SIMPLE_INT ("TX_DELAY", &TX_DELAY),
		CFG_SIMPLE_STR ("DIRECTORY_LOG", &DIRECTORY_LOG),
                CFG_END()
        };
        cfg = cfg_init(opts, 0);
	if( cfg_parse(cfg, "/opt/HAX/Control/Devices/Hardware/Mount/etc/HAX.config") == CFG_FILE_ERROR)
	{
		printf("\nCan´t open config file. \n\n");
		return 1;
	}

	cfg_free(cfg);


	printf ("\nIP_SERVER: %s \n", IP_SERVER);
	printf ("TCP_PORT: %ld \n", TCP_PORT);
	printf ("RCV_BUFFER_SIZE: %ld \n", RCV_BUFFER_SIZE);
	printf ("TX_DELAY: %ld \n", TX_DELAY);
	printf ("DIRECTORY_LOG: %s \n\n", DIRECTORY_LOG);



	return 0;
}

