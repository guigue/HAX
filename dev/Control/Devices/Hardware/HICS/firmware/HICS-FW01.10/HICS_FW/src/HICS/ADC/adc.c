
/*
 * adc.c
 *
 * Created: 12/07/2019 15:15:02
 *  Author: User
 */ 

/*
	ADC Canal 3 - Celula de Golay
	ADC Canal 0 - Monitoramento Fonte de Alimentacao

*/

// HICS SPI TIPS&TRICKS
//http://asf.atmel.com/docs/latest/sam3x/html/sam_spi_quickstart.html
//https://www.avrfreaks.net/forum/spi-master-driver
//http://asf.atmel.com/docs/latest/sam3x/html/spi_master_xmega.html
//	if (spi_write_packet(SPI, data_buffer, 1) != STATUS_OK)
//	{	printf ("\nSPI_write_error!\n");
//	}
//	if (spi_read_packet(SPI, data_buffer, 1) != STATUS_OK)
//	{	printf ("\nSPI_read_error!\n");
//	}
// HICS_end


#include <asf.h>
#include <string.h>
#include <inttypes.h>	// Used for PRIu64

#include "conf_board.h"
#include "sockets.h"
#include "h_sntp.h"
#include "husec.h"
#include "htask.h"
#include "h_wdt.h"
#include "adc.h"


volatile uint32_t dwDRDYpulse = 0;	// Contador de amostras (pulso no sinal DRDY)

// Channel circular buffer data 
__attribute__((__section__(".data_sdram2")))  CHBUF_DATA chcbuf_data[ADC_BUFFER_LINES]; 

ADC_CIRC_BUFF adc_cbuff;

volatile uint16_t wADCcbuff_sizefulln=0;				// Debug contador buffer full
volatile uint32_t dwADCcbuff_sizefme=ADC_BUFFER_LINES;	// Debug buffer free size minimum ever
volatile uint32_t dwADCcbuff_sizeact=0;					// Debug buffer size atual

uint8_t	byADC_Task_step=0;

struct spi_device spi_device_conf = {
	//.id = SPI_NPCS1_PB14_GPIO
	//.id = 00 // SPI_NPCS0_GPIO (PA11) (Conector EXT1)
	.id = 03		// Conector EXT3
};

typedef union uHexToInt
{	uint8_t buff[4];
	int32_t lSample;
} uHexToInt;

uHexToInt uxiGolay;
uHexToInt uxiPS;

volatile uint8_t byADCtx_sprintf_buff[256];

volatile uint8_t byADCtx_sprintf_n;

volatile uint8_t  byADCtx_mtu_buff[1400];
volatile uint16_t wADCtx_mtu_n;

volatile uint32_t dwLostSamples_cx=0;
volatile uint32_t dwLostSamples_old=0;
volatile uint32_t dwLostSamples_lost_o=0;
volatile uint32_t dwLostSamples_lost_n=0;


void BinToAsciiHex(U8 byValue, U8 *pbyCharLSB, U8 *pbyCharMSB)
{
	// Extrai o nibble mais significativo
	*pbyCharMSB = byValue & 0xF0;
	*pbyCharMSB = *pbyCharMSB >> 4;

	// Extrai o nibble menos significativo
	*pbyCharLSB = byValue & 0x0F;

	if(*pbyCharMSB < 0x0A)  *pbyCharMSB = *pbyCharMSB + 0x30;
	else                    *pbyCharMSB = *pbyCharMSB + 0x37;

	if(*pbyCharLSB < 0x0A)  *pbyCharLSB = *pbyCharLSB + 0x30;
	else                    *pbyCharLSB = *pbyCharLSB + 0x37;
}


void ADC_copy_to_mtu_buff(U8 *pby, U8 bySize)
{
	U8 by;

	for(by=0; by<bySize; by++)
	{
		byADCtx_mtu_buff[wADCtx_mtu_n++] = *pby++;
	}
}



// https://embedjournal.com/implementing-circular-buffer-embedded-c/
void ADC_CBUF_Init(void)
{
	adc_cbuff.pchbuf = 	&chcbuf_data[0];
	adc_cbuff.head = 0;
	adc_cbuff.tail = 0;
	adc_cbuff.maxlen = ADC_BUFFER_LINES;
}


int ADC_CBUF_push(CHBUF_DATA *data)
{
	uint32_t next;
	uint32_t size;

	next = adc_cbuff.head + 1;  // next is where head will point to after this write.

	if (next >= adc_cbuff.maxlen)
		next = 0;

	if (next == adc_cbuff.tail)  // if the head + 1 == tail, circular buffer is full
		return -1;

	// Load data
	adc_cbuff.pchbuf[adc_cbuff.head].ulSample	     = data->ulSample;		
	adc_cbuff.pchbuf[adc_cbuff.head].ulTimestamp_sec = data->ulTimestamp_sec;
	adc_cbuff.pchbuf[adc_cbuff.head].usTimestamp_ms  = data->usTimestamp_ms;
	adc_cbuff.pchbuf[adc_cbuff.head].u64husec		 = data->u64husec;

	adc_cbuff.pchbuf[adc_cbuff.head].rawGolay[0]	 = data->rawGolay[0];
	adc_cbuff.pchbuf[adc_cbuff.head].rawGolay[1]	 = data->rawGolay[1];
	adc_cbuff.pchbuf[adc_cbuff.head].rawGolay[2]	 = data->rawGolay[2];
	adc_cbuff.pchbuf[adc_cbuff.head].rawGolay[3]	 = data->rawGolay[3];

	adc_cbuff.pchbuf[adc_cbuff.head].rawPS[0]		= data->rawPS[0];
	adc_cbuff.pchbuf[adc_cbuff.head].rawPS[1]		= data->rawPS[1];
	adc_cbuff.pchbuf[adc_cbuff.head].rawPS[2]		= data->rawPS[2];
	adc_cbuff.pchbuf[adc_cbuff.head].rawPS[3]		= data->rawPS[3];

	adc_cbuff.head = next;             // head to next data offset.
	
	// Debug: registra FREE minimum ever
	if( adc_cbuff.head >= adc_cbuff.tail )
	{	size = (adc_cbuff.head - adc_cbuff.tail);
	}
	else
	{	size = (adc_cbuff.maxlen + adc_cbuff.head - adc_cbuff.tail);
	}
	dwADCcbuff_sizeact = size;
	if (dwADCcbuff_sizefme > (adc_cbuff.maxlen-size) )
	{	dwADCcbuff_sizefme = adc_cbuff.maxlen-size;
	}
	
	return 0;  // return success to indicate successful push.
}


int ADC_CBUF_pop(CHBUF_DATA *data)
{
	int next;

	if (adc_cbuff.head == adc_cbuff.tail)  // if the head == tail, we don't have any data
		return -1;

	next = adc_cbuff.tail + 1;  // next is where tail will point to after this read.

	if(next >= adc_cbuff.maxlen)
		next = 0;

	// Read data
	data->ulSample			= adc_cbuff.pchbuf[adc_cbuff.tail].ulSample;	     
	data->ulTimestamp_sec	= adc_cbuff.pchbuf[adc_cbuff.tail].ulTimestamp_sec;
	data->usTimestamp_ms	= adc_cbuff.pchbuf[adc_cbuff.tail].usTimestamp_ms;
	data->u64husec			= adc_cbuff.pchbuf[adc_cbuff.tail].u64husec;
	
	data->rawGolay[0]		= adc_cbuff.pchbuf[adc_cbuff.tail].rawGolay[0];
	data->rawGolay[1]		= adc_cbuff.pchbuf[adc_cbuff.tail].rawGolay[1];
	data->rawGolay[2]		= adc_cbuff.pchbuf[adc_cbuff.tail].rawGolay[2];
	data->rawGolay[3]		= adc_cbuff.pchbuf[adc_cbuff.tail].rawGolay[3];

	data->rawPS[0]			= adc_cbuff.pchbuf[adc_cbuff.tail].rawPS[0];
	data->rawPS[1]			= adc_cbuff.pchbuf[adc_cbuff.tail].rawPS[1];
	data->rawPS[2]			= adc_cbuff.pchbuf[adc_cbuff.tail].rawPS[2];
	data->rawPS[3]			= adc_cbuff.pchbuf[adc_cbuff.tail].rawPS[3];

	adc_cbuff.tail = next;              // tail to next offset.

	return 0;  // return success to indicate successful push.
}

uint32_t ADC_CBUF_size(void)
{
	uint32_t size;
	
	// HICS: AQUI TEM QUE SER ATOMICO
	if( adc_cbuff.head >= adc_cbuff.tail )
	{	size = (adc_cbuff.head - adc_cbuff.tail);
	}
	else
	{	size = (adc_cbuff.maxlen + adc_cbuff.head - adc_cbuff.tail);
	}
	
	return size;
}

void ADC_Init(void)
{
	// Inicializa buffer circular
	ADC_CBUF_Init();
	
	// Init ADC
	ADC_Reset();				// Reset ADC
	ADC_SPI_init();				// Inicializa SPI para comunicar com ADC
	ADC_ReadAllRegisters();		// Le e print registros de configuracao
	ADC_Config();				// Configura AD
	ADC_ReadAllRegisters();		// Le novamente e print registros para verificar se houve alteracoes
	ADC_EnableReadADConSDO();	// Libera leitura amostras via SPI

	// Configure AD7770 DRDY signal trigger an interrupt on rising edge
	// DRDY vai gerar int a cada 1ms (ver DECIMATION RATE = 2048 (ODR = 1KHz) em AD7770_Config())
	// ANALISAR: Configuar PIN para gerar uma int ou um evento (ver exemplo 32-02-GPIO_ISR_EXAMPLE_SAM4L) ???

	//pmc_enable_periph_clk(ID_PIOA);	// Conector EXT1
	pmc_enable_periph_clk(ID_PIOD);	// Conector EXT3

	ioport_set_pin_dir(AD7770_DRDY_PIN, IOPORT_DIR_INPUT);  // DRDY
	ioport_set_pin_mode(AD7770_DRDY_PIN, IOPORT_MODE_GLITCH_FILTER); // 
	ioport_set_pin_sense_mode(AD7770_DRDY_PIN, IOPORT_SENSE_RISING);

	pio_handler_set(PIOD, ID_PIOD, PIO_PD17, PIO_IT_RISE_EDGE, ADC_ReadADC_isr);	// Conector EXT3
	
	NVIC_SetPriority(PIOD_IRQn,7);	

	pio_enable_interrupt(PIOD, PIO_PD17);		// Conector EXT3

	NVIC_EnableIRQ(PIOD_IRQn);		// Conector EXT3
	
}


void ADC_Reset(void)
{
	const TickType_t x2ms = 2UL / portTICK_PERIOD_MS;	// Delay de 2ms
	
	// Gera um pulso no RESET do AD7770 (pulso minimo de 2×MCLK ns)
	// Atencao: pino reset do AD7770 nao pode ficar aberto
	// Conector EXT1
	//ioport_set_pin_dir(PIO_PD25_IDX, IOPORT_DIR_OUTPUT);
	//ioport_set_pin_level(PIO_PD25_IDX, IOPORT_PIN_LEVEL_HIGH);
	//vTaskDelay( x2ms );	// Delay 2ms
	//ioport_set_pin_level(PIO_PD25_IDX, IOPORT_PIN_LEVEL_LOW); 	
	//vTaskDelay( x2ms );	// Delay 2ms
	//ioport_set_pin_level(PIO_PD25_IDX, IOPORT_PIN_LEVEL_HIGH);

	// Gera um pulso no RESET do AD7770 (pulso minimo de 2×MCLK ns)
	// Atencao: pino reset do AD7770 nao pode ficar aberto
	// Conector EXT3
	ioport_set_pin_dir(PIO_PD26_IDX, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIO_PD26_IDX, IOPORT_PIN_LEVEL_HIGH);
	vTaskDelay( x2ms );	// Delay 2ms
	ioport_set_pin_level(PIO_PD26_IDX, IOPORT_PIN_LEVEL_LOW); 	
	vTaskDelay( x2ms );	// Delay 2ms
	ioport_set_pin_level(PIO_PD26_IDX, IOPORT_PIN_LEVEL_HIGH);
	
	
	
	// tINIT_RESET RESET rising edge to first DRDY 16 kSPS, high resolution mode 225us
	vTaskDelay( x2ms );	// Delay 2ms

	printf ("\nAD7770_tINIT_RESET_delay!\n"); 

}


void ADC_SPI_init(void)
{
	spi_enable_clock(SPI);  
	
	// uC SAM4E is master
	spi_master_init(SPI);
	
	//*   - \code void spi_master_setup_device(SPI_EXAMPLE, &SPI_DEVICE_EXAMPLE,
	//SPI_MODE_0, SPI_EXAMPLE_BAUDRATE, 0); \endcode

	// AD7770:
	// - EVAL-AD7770 configured as SPI CONTROL MODE (FORMAT0 and FORMAT1 pins to logic high => jumpers SL5 and SL6 positon A)
	// - SPI operates in Mode 0 and Mode 3: CPOL = 0, CPHA = 0 (Mode 0) or CPOL = 1, CPHA = 1 (Mode 3)
	// - CLK period up to 30MHz
	//spi_master_setup_device(SPI, &spi_device_conf, SPI_MODE_0, 1000000, 0); // 1MHz (1us per bit)   (256bits = 256us)
	//spi_master_setup_device(SPI, &spi_device_conf, SPI_MODE_0, 10000000, 0); // 10MHz (0.1us per bit) (256bits = 25.6us)
	spi_master_setup_device(SPI, &spi_device_conf, SPI_MODE_0, 20000000, 0); // 20MHz (0.05us per bit) (256bits = 12.8us) => HICS_HFchanges
	
	spi_enable(SPI);
}

void ADC_ReadAllRegisters(void)
{
	uint8_t wr_buff[1];
	uint8_t rd_buff[AD7770_NUMBER_OF_REGISTERS];
	uint8_t by;
	
	// Read all registers
	spi_select_device(SPI, &spi_device_conf);
	for (by=0; by<AD7770_NUMBER_OF_REGISTERS; by++)
	{	wr_buff[0] = 0x80 | by;	// For read operation MSB must be equal to 1
		spi_write_packet(SPI, wr_buff, 1);
		spi_read_packet (SPI, &rd_buff[by], 1);
	}
	spi_deselect_device(SPI, &spi_device_conf);

	// Print all registers
	printf ("\nAD7770 Registers data from 0x00 to %02X:\n",AD7770_NUMBER_OF_REGISTERS-1);
	for (by=0; by<AD7770_NUMBER_OF_REGISTERS; by++)
	{	printf ("%02X ",rd_buff[by]);
		if ((by+1)%16 == 0)
			printf ("\n");
	}
	printf ("\n");


}

void ADC_Config(void)
{
	uint8_t buff[2];

	printf ("\nAD7770 config main registers:\n");

	// POWERMODE = High resolution
	printf ("=> POWERMODE = High resolution (reg 0x11 bit6=1)\n");
	buff[0] = 0x11;	// Operation: write GENERAL_USER_CONFIG_1 register (address: 0x11)
	buff[1] = 0x64;	// bit 6 = 1 (POWERMODE = High resolution)
	spi_select_device(SPI, &spi_device_conf);
	spi_write_packet(SPI, &buff[0], 2);
	spi_deselect_device(SPI, &spi_device_conf);
	
	// Config DECIMATION RATE = 2048 (0x0800) (e consequentemente ODR (output data rate) sera igual a 1KHz)
	printf ("=> DECIMATION RATE = 2048 (ODR = 1KHz) (reg 0x60=0x08 and reg 0x61=00)\n");
	buff[0] = 0x60;	// Operation: write SRC_N_MSB register (address: 0x60)
	buff[1] = 0x08;	// MSB of 0x0800 
	spi_select_device(SPI, &spi_device_conf);
	spi_write_packet(SPI, &buff[0], 2);
	spi_deselect_device(SPI, &spi_device_conf);
	buff[0] = 0x61;	// Operation: write SRC_N_LSB register (address: 0x61)
	buff[1] = 0x00;	// LSB of 0x0800
	spi_select_device(SPI, &spi_device_conf);
	spi_write_packet(SPI, &buff[0], 2);
	spi_deselect_device(SPI, &spi_device_conf);

	// New ODR value is updated by setting the SRC_LOAD_UPDATE bit to 1
	// There are two different ways to change the ODR after a new value is written in the SRC registers: via software or via hardware,	// depending on SRC_LOAD_SOURCE (SRC_UPDATE register, Bit 7).
	// If the SRC_LOAD_SOURCE bit is clear, the new ODR value is updated by setting the SRC_LOAD_UPDATE bit to 1. This bit must be held high	// for at least two MLCK periods; return the bit to 0 before attempting another update.
	printf ("=> New ODR value is updated by setting the SRC_LOAD_UPDATE bit to 1\n");
	buff[0] = 0x64;	// Operation: write SRC_UPDATE register (address: 0x64)
	buff[1] = 0x01;	// SRC_LOAD_UPDATE bit = 1
	spi_select_device(SPI, &spi_device_conf);
	spi_write_packet(SPI, &buff[0], 2);
	spi_deselect_device(SPI, &spi_device_conf);

	
}


void ADC_EnableReadADConSDO(void)
{
	uint8_t buff[2];

	printf ("\nAD7770 ENABLE SPI slave mode to read back ADC on SDO!\n\n");
	
	// Enable to SPI slave mode to read back ADC on SDO
	// 0x013	GENERAL_USER_CONFIG_3	[7:0]	CONVST_ DEGLITCH_DIS	RESERVED	SPI_SLAVE_MODE_EN	RESERVED 	CLK_QUAL_DIS	0x80

	buff[0] = 0x80 | 0x13;	// Operation: read GENERAL_USER_CONFIG_3 register (For read operation MSB must be equal to 1)
	spi_select_device(SPI, &spi_device_conf);
	spi_write_packet(SPI, &buff[0], 1);
	spi_read_packet (SPI, &buff[1], 1);
	spi_deselect_device(SPI, &spi_device_conf);
	
	buff[0] =  0x13;	// Operation: write GENERAL_USER_CONFIG_3 register
	buff[1] |= 0x10;	// Set SPI_SLAVE_MODE_EN bit
	spi_select_device(SPI, &spi_device_conf);
	spi_write_packet(SPI, &buff[0], 2);
	spi_deselect_device(SPI, &spi_device_conf);

}

void ADC_DisableReadADConSDO(void)
{
	uint8_t buff[2];

	printf ("\nAD7770 DISABLE SPI slave mode to read back ADC on SDO!\n");
	
	// Enable to SPI slave mode to read back ADC on SDO
	// 0x013	GENERAL_USER_CONFIG_3	[7:0]	CONVST_ DEGLITCH_DIS	RESERVED	SPI_SLAVE_MODE_EN	RESERVED 	CLK_QUAL_DIS	0x80

	// ATENCAO: NAO DA PARA LER O REGISTRO, ESTA EM MODO "READ BACK ADC"
	//buff[0] = 0x80 | 0x13;	// Operation: read GENERAL_USER_CONFIG_3 register
	//spi_select_device(SPI, &spi_device_conf);
	//spi_write_packet(SPI, &buff[0], 1);
	//spi_read_packet (SPI, &buff[1], 1);
	//spi_deselect_device(SPI, &spi_device_conf);
	
	buff[0] = 0x13;	// Operation: write GENERAL_USER_CONFIG_3 register
	buff[1] = 0x80;	// Reset SPI_SLAVE_MODE_EN bit 
	spi_select_device(SPI, &spi_device_conf);
	spi_write_packet(SPI, &buff[0], 2);
	spi_deselect_device(SPI, &spi_device_conf);

}


void ADC_ReadADC(void)
{
	uint8_t adc_buff[ADC_8CH_BUFFER_SIZEOF]; 
	uint8_t by;

	for (by=0; by<ADC_8CH_BUFFER_SIZEOF; by++)
		adc_buff[by] = 0x55; // "clear" buffer
	
	// Read all 8 channels (each channel => header 8-bit + adc 24-bit)
	spi_select_device(SPI, &spi_device_conf);
	spi_read_packet (SPI, &adc_buff, ADC_8CH_BUFFER_SIZEOF);
	spi_deselect_device(SPI, &spi_device_conf);

	//printf ("\nAD7770 read ADC:\n");

	for (by=0; by<ADC_8CH_BUFFER_SIZEOF; by++)
		printf ("%02X ",adc_buff[by]);

	printf ("\n");

	
}

// Sinal DRDY vai gerar int a cada 1ms (ver DECIMATION RATE = 2048 (ODR = 1KHz) em AD7770_Config())
void ADC_ReadADC_isr(const uint32_t id, const uint32_t index)
{
	CHBUF_DATA  *pdata;
	CHBUF_DATA  chdata;
	
	uint8_t adc_buff[ADC_8CH_BUFFER_SIZEOF];
	uint8_t	by;
	
	UBaseType_t uxSavedInterruptStatus;
	
	//if ((id == ID_PIOA) && (index == PIO_PA25))	// Conector EXT1
	if ((id == ID_PIOD) && (index == PIO_PD17))	// Conector EXT3
	{

		ioport_set_pin_level(PIO_PA24_IDX, IOPORT_PIN_LEVEL_HIGH);	// HICS debug: Utiliza PA24 (conector EXT1-5) como sinal de teste da ISR ADC

		//taskDISABLE_INTERRUPTS();	// Leitura tem que ser atomica
		uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();	
		//taskENTER_CRITICAL();
		//portDISABLE_INTERRUPTS();

		byWDT_adcisr_ok=1;	// Sinaliza ISR ADC ok

		dwDRDYpulse++;
		
		// Incrementa timestamp do HICS (seconds and ms)
		// Utiliza ISR 1ms do sinal DRDY como tick de tempo para as amostras
		if (++hts.usMs > 999)
		{	hts.usMs=0;
			hts.ulSec++;
		}
		
		// Incrementa timestamp do HICS (husecs)
		hts.u64husec+=10;	 // Incrementa o milesegundo do husec (número de centenas de microsegundos (=0,1 milisegundos))
		if (hts.u64husec >= HUSEC_ONEDAY_HUSECS)
		{	hts.u64husec=0;
		}

		// Incrementa tick schedulers		
		if (hsched.ps_rst==0) 
		{	hsched.ps++;
		}
		else 
		{	hsched.ps=0; 
			hsched.ps_rst=0; 
		}

		if (hsched.sntp_rst==0)
		{	hsched.sntp++;
		}
		else
		{	hsched.sntp=0;
			hsched.sntp_rst=0;
		}
		
		
		// "clear" buffer
		for (by=0; by<ADC_8CH_BUFFER_SIZEOF; by++)
			adc_buff[by] = 0x55; 
	
		// Read all 8 channels (each channel => header 8-bit + adc 24-bit)
		spi_select_device(SPI, &spi_device_conf);
		spi_read_packet (SPI, &adc_buff, ADC_8CH_BUFFER_SIZEOF);
		spi_deselect_device(SPI, &spi_device_conf);

		// Copia conteudo raw do canal 0 (POWER SUPPLY) para a struct
		chdata.rawPS[0] = adc_buff[0];
		chdata.rawPS[1] = adc_buff[1];
		chdata.rawPS[2] = adc_buff[2];
		chdata.rawPS[3] = adc_buff[3];
		
		// Copia conteudo raw do canal 3 (CELULA DE GOLAY) para a struct
		chdata.rawGolay[0] = adc_buff[12];
		chdata.rawGolay[1] = adc_buff[13];
		chdata.rawGolay[2] = adc_buff[14];
		chdata.rawGolay[3] = adc_buff[15];

		// Copia numero da amostra e timestamp
		chdata.ulSample			= dwDRDYpulse;
		chdata.ulTimestamp_sec  = hts.ulSec;
		chdata.usTimestamp_ms   = hts.usMs;
		chdata.u64husec			= hts.u64husec;

		// Adiciona dados da amostragem no buffer circular
		if (ADC_CBUF_push(&chdata)==-1)
		{	wADCcbuff_sizefulln++;
		}

		//portENABLE_INTERRUPTS();
		//taskEXIT_CRITICAL();
		taskEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus ); 
		//taskENABLE_INTERRUPTS();
	
		ioport_set_pin_level(PIO_PA24_IDX, IOPORT_PIN_LEVEL_LOW);	// HICS debug

	}
	
}


void ADC_task_steps(void)
{
	CHBUF_DATA  chdata;
	uint32_t	dwCBuffSize;
	uint8_t		*pby, by, byBuff[32];
	int8_t		i;

	switch (byADC_Task_step)
	{
		case ADC_TASK_STEP_WAIT_FOR_NETWORK:
			if( stSocket_ADC.can_create != pdFALSE )
			{	byADC_Task_step = ADC_TASK_STEP_OPEN_SOCKET;
			}
			break;

		case ADC_TASK_STEP_OPEN_SOCKET:
			if (vSocket_ADC_tcp_create() == pdPASS)
			{	byADC_Task_step = ADC_TASK_STEP_POP_DATA;
			}
			break;
		
		
		 case ADC_TASK_STEP_POP_DATA:
			dwCBuffSize=ADC_CBUF_size();
			if (dwCBuffSize >= 20)
			{	
				wADCtx_mtu_n=0;
				do
				{
					if (ADC_CBUF_pop(&chdata) == -1)
						break;	// Buffer esvaziou


					#ifdef HICS_TX_ASCII_HEX
					//**********************
					// Monta buffer ASCII-HEX
					byADCtx_sprintf_n=0;
				
					pby = &chdata.ulSample;	// 4 bytes
					for (by=0, i=0; by < sizeof(chdata.ulSample); by++, i+=2)
					{	BinToAsciiHex(*pby++, &byBuff[i], &byBuff[i+1]);
					}
					while ( (--i) >= 0)
					{	byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[i];	// MSB First
					}
					
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = 0x20; // Space
					
					pby = &chdata.ulTimestamp_sec;
					for (by=0, i=0; by < sizeof(chdata.ulTimestamp_sec); by++, i+=2)
					{	BinToAsciiHex(*pby++, &byBuff[i], &byBuff[i+1]);
					}
					while ( (--i) >= 0)
					{	byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[i];	// MSB First					
					}

					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = 0x20; // Space

					pby = &chdata.usTimestamp_ms;
					for (by=0, i=0; by < sizeof(chdata.usTimestamp_ms); by++, i+=2)
					{	BinToAsciiHex(*pby++, &byBuff[i], &byBuff[i+1]);
					}
					while ( (--i) >= 0)
					{	byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[i];	// MSB First					
					}

					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = 0x20; // Space

					pby = &chdata.u64husec;
					for (by=0, i=0; by < sizeof(chdata.u64husec); by++, i+=2)
					{	BinToAsciiHex(*pby++, &byBuff[i], &byBuff[i+1]);
					}
					while ( (--i) >= 0)
					{	byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[i];	// MSB First					
					}

					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = 0x20; // Space

					pby = &chdata.rawGolay[0];
					for (by=0, i=0; by < sizeof(chdata.rawGolay); by++, i+=2)
					{	BinToAsciiHex(*pby++, &byBuff[i], &byBuff[i+1]);
					}
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[1];	// MSB First
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[0];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[3];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[2];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[5];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[4];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[7];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[6];

					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = 0x20; // Space

					pby = &chdata.rawPS[0];
					for (by=0, i=0; by < sizeof(chdata.rawPS); by++, i+=2)
					{	BinToAsciiHex(*pby++, &byBuff[i], &byBuff[i+1]);
					}
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[1];	// MSB First
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[0];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[3];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[2];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[5];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[4];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[7];
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = byBuff[6];
					
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = 0x0D; // CR
					byADCtx_sprintf_buff[byADCtx_sprintf_n++] = 0x0A; // LF

					for (by=0; by < byADCtx_sprintf_n; by++)
					{   byADCtx_mtu_buff[wADCtx_mtu_n] = byADCtx_sprintf_buff[by];
						wADCtx_mtu_n++;
					}
					
					#else	
					//**********************
					// Monta buffer no formato binario (raw data) LSB first
					ADC_copy_to_mtu_buff( &chdata.ulSample,			sizeof(chdata.ulSample) );
					ADC_copy_to_mtu_buff( &chdata.ulTimestamp_sec,	sizeof(chdata.ulTimestamp_sec) );
					ADC_copy_to_mtu_buff( &chdata.usTimestamp_ms,	sizeof(chdata.usTimestamp_ms) );
					ADC_copy_to_mtu_buff( &chdata.u64husec,			sizeof(chdata.u64husec) );

					byADCtx_mtu_buff[wADCtx_mtu_n++] = chdata.rawGolay[3];	// LSB first
					byADCtx_mtu_buff[wADCtx_mtu_n++] = chdata.rawGolay[2];
					byADCtx_mtu_buff[wADCtx_mtu_n++] = chdata.rawGolay[1];
					byADCtx_mtu_buff[wADCtx_mtu_n++] = chdata.rawGolay[0];
					
					byADCtx_mtu_buff[wADCtx_mtu_n++] = chdata.rawPS[3];		// LSB first
					byADCtx_mtu_buff[wADCtx_mtu_n++] = chdata.rawPS[2];
					byADCtx_mtu_buff[wADCtx_mtu_n++] = chdata.rawPS[1];
					byADCtx_mtu_buff[wADCtx_mtu_n++] = chdata.rawPS[0];
					//**********************
					#endif

					// Checking lost samples
					if ( (++dwLostSamples_old) != chdata.ulSample)
					{	dwLostSamples_lost_n = chdata.ulSample;
						dwLostSamples_lost_o = dwLostSamples_old;
						dwLostSamples_cx++;
						dwLostSamples_old  = chdata.ulSample;
					}

				} while (wADCtx_mtu_n < 1200);	// (MTU=1400)

				if (wADCtx_mtu_n != 0 )
				{	byADC_Task_step = ADC_TASK_STEP_TX_DATA;
				}
			}
			break;
			
		case ADC_TASK_STEP_TX_DATA:
			if (vSocket_ADC_tcp_sendto(byADCtx_mtu_buff,wADCtx_mtu_n) > 0)
			{	byADC_Task_step = ADC_TASK_STEP_POP_DATA;
			}
			else
			{	byADC_Task_step = ADC_TASK_STEP_CLOSE_SOCKET;
			}	
			break;

		case ADC_TASK_STEP_CLOSE_SOCKET:
			vSocket_ADC_tcp_close();
			byADC_Task_step = ADC_TASK_STEP_OPEN_SOCKET;
			break;
	
	}

}



