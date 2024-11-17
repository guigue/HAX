/*
 * ad7770.h
 *
 * Created: 12/07/2019 15:15:30
 *  Author: User
 */ 


#ifndef ADC_H_
#define ADC_H_

//-----------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------

#define AD7770_NUMBER_OF_REGISTERS 101	// Registers address 0x00 to 0x64

#define ADC_1CH_BUFFER_SIZEOF (1+3)	// header 8-bit + adc 24-bit)
#define ADC_8CH_BUFFER_SIZEOF (8*(ADC_1CH_BUFFER_SIZEOF)) // 8 canais x (header 8-bit + adc 24-bit)

#define ADC_BUFFER_LINES	10000	// SRAM EXTERNA: 10.000 amostras x 1ms = 10 segundos de buffer
									// Capacidade maxima: 512KB / sizeof registro (32bytes) => 16.384
									// Remark: sizeof eh 22 bytes, mas compilador considerando 32 por causa de alinhamento

#define ADC_TASK_STEP_WAIT_FOR_NETWORK	0
#define ADC_TASK_STEP_OPEN_SOCKET		1
#define ADC_TASK_STEP_POP_DATA			2
#define ADC_TASK_STEP_TX_DATA			3
#define ADC_TASK_STEP_CLOSE_SOCKET		4

//-----------------------------------------------------------
// VARS PROTOTYPES
// ----------------------------------------------------------

// ADC CHANNEL BUFFER DATA
typedef struct
{	uint32_t ulSample;

	uint32_t ulTimestamp_sec;	// Timestamp segundos
	uint16_t usTimestamp_ms;	// Timestamp milesegundos

	uint64_t u64husec;			// Timestamp HUSECS

	// Raw do Canal ADC
	uint8_t  rawGolay[ADC_1CH_BUFFER_SIZEOF];	// Celula de Golay
	uint8_t  rawPS[ADC_1CH_BUFFER_SIZEOF];		// Power Supply (monitoramento da Tensao de Alimentacao)
		
} CHBUF_DATA;

extern CHBUF_DATA chcbuf_data[ADC_BUFFER_LINES];		// Channel circular buffer data 

typedef struct {
	CHBUF_DATA *pchbuf;
	uint32_t	head;
	uint32_t	tail;
	uint32_t	maxlen;
} ADC_CIRC_BUFF;
extern ADC_CIRC_BUFF adc_cbuff;

//-----------------------------------------------------------
// FUNCTIONS PROTOTYPES
// ----------------------------------------------------------

void ADC_CBUF_Init(void);
int ADC_CBUF_push(CHBUF_DATA *data);
int ADC_CBUF_pop(CHBUF_DATA *data);
uint32_t ADC_CBUF_size(void);

void ADC_Init(void);

void ADC_Reset(void);
void ADC_SPI_init(void);

void ADC_ReadAllRegisters(void);

void ADC_Config(void);

void ADC_EnableReadADConSDO(void);
void ADC_DisableReadADConSDO(void);

void ADC_ReadADC(void);

void ADC_ReadADC_isr(const uint32_t id, const uint32_t index);

void ADC_task(void);
void ADC_task_steps(void);

#endif /* ADC_H_ */