#include <stdio.h>
#include <string.h>
#include "G24.h"
#include "ledmatrix.h"
#include "nrf24l01.h"
#include "ledmatrix.h"
#include "usbd_agent.h"


#define MAX_BUFFER_NUM 64
#define RX_BUFFER_LEN NRF24L01_PAYLOAD_LENGTH



typedef struct
{
	uint8_t 	serial;
	uint8_t 	crc;
	g24_data	data;
}g24_buffer;

typedef struct
{
	uint8_t 	buffer[RX_BUFFER_LEN];
	uint8_t 	len;
	uint8_t 	readlen;
}rxbuffer;

typedef struct 
{
    rxbuffer* data[MAX_BUFFER_NUM];
    volatile int front;
    volatile int rear;
} Queue;


static rxbuffer sg_g24databuffer[MAX_BUFFER_NUM] = {0};
static Queue sg_rxqueue_free = {0};
static Queue sg_rxqueue_used = {0};

volatile uint8_t sgv_TimerOutfg = FALSE;
volatile uint8_t sgv_RxIRQfg = FALSE;

#ifdef CMD_ENABLE

volatile uint8_t sgv_CMDfg = FALSE;

#define CMD_LEN 7
static uint8_t sg_cmdbuffer[CMD_LEN+1] = {0}; 
#endif
static uint8_t RxAddrss[6][5] = 
{
	{0x78, 0x78, 0x78, 0x78, 0x78},
	{0xB3, 0xB4, 0xB5, 0xB6, 0xCD},
	{0xF3,0,0,0,0},
	{0xF2,0,0,0,0},
	{0xF4,0,0,0,0},
	{0xF1,0,0,0,0},
};

static NRF24L01_RxAddTypeDef RxAddrssType[6] = 
{
	NRF24L01_RX_ADDRESS_P0,
	NRF24L01_RX_ADDRESS_P1,
	NRF24L01_RX_ADDRESS_P2,
	NRF24L01_RX_ADDRESS_P3,
	NRF24L01_RX_ADDRESS_P4,
	NRF24L01_RX_ADDRESS_P5,
};

#define ERROR_VERSION 		0
#define ERROR_TYPE 			1
#define ERROR_KB_LEN 		2
#define ERROR_MOUSE_LEN 	3
#define ERROR_VPC_LEN 		4
#define ERROR_KB_SERIAL 		5
#define ERROR_MOUSE_SERIAL 		6
#define ERROR_VPC_SERIAL 		7
#define ERROR_KB_CRC 		8
#define ERROR_MOUSE_CRC		9
#define ERROR_VPC_CRC 		10

#define ERROR_NUM 			11

static uint16_t sg_errstatistics[ERROR_NUM] = {0};
#define ERROR_STAT(ERROR) sg_errstatistics[(ERROR)]++


#define G24_CHANNEL 	37


static void initializeQueue(Queue *q) 
{
    q->front = -1;
    q->rear = -1;
	memset(q->data,0,sizeof(rxbuffer*)*MAX_BUFFER_NUM);
}

static int isFull(Queue *q) 
{
    return (q->rear == MAX_BUFFER_NUM - 1);
}

static int isEmpty(Queue *q) 
{
    return (q->front == -1 || q->front > q->rear);
}

static void enqueue(Queue *q, rxbuffer* item) 
{
    if (isFull(q)) 
	{
        return;
    }

    if (isEmpty(q)) 
	{
        q->front = 0;
    }

    q->rear++;
    q->data[q->rear] = item;
}

static rxbuffer* dequeue(Queue *q) 
{
    if (isEmpty(q)) 
	{
        return NULL;
    }
    rxbuffer* item = q->data[q->front];

    if (q->front == q->rear) 
	{
        q->front = -1;
        q->rear = -1;
    } 
	else 
	{
        q->front++;
    }

    return item;
}

#define G24_GetFreeBuffer() dequeue(&sg_rxqueue_free)
#define G24_PutFreeBuffer(BUFFER) enqueue(&sg_rxqueue_free,(BUFFER))

#define G24_GetUsedBuffer() dequeue(&sg_rxqueue_used)
#define G24_PutUsedBuffer(BUFFER) enqueue(&sg_rxqueue_used,(BUFFER))

//NRF24L01_TxTransmit(TxBuffer, 2000);


// CRC8 polynomial in reversed bit order
// Example: x^8 + x^2 + x + 1 (0x07)
#define CRC8_POLY 0x07

// Calculate CRC8 of the input data
uint8_t crc8(uint8_t *data, uint8_t length) 
{
    uint8_t crc = 0; // Initial value
    for (size_t i = 0; i < length; ++i) 
	{
        crc ^= data[i]; // XOR current byte with the crc
        for (int j = 0; j < 8; ++j) 
		{
            if (crc & 0x80) 
			{ // If the highest bit is set
                crc = (crc << 1) ^ CRC8_POLY; // Left shift and XOR with polynomial
            } else 
			{
                crc <<= 1; // Left shift only
            }
        }
    }
    return crc; // Return the final CRC8 value
}

void do_idle(void);
#define GET_USED_RXBUFFER(RET) do\
{\
	do_idle();\
	(RET)=G24_GetUsedBuffer();\
}while(NULL == (RET))


static rxbuffer* G24_RxData()
{
	rxbuffer* ret;

	GET_USED_RXBUFFER(ret);
	ret->len = RX_BUFFER_LEN;
	ret->readlen = 0;
	return ret;
}

static uint8_t* G24_RecvData(rxbuffer* rxbuf,uint16_t len)
{
	uint8_t* ret;
	
	if((rxbuf->len - rxbuf->readlen) < len)
		return NULL;

	ret = &rxbuf->buffer[rxbuf->readlen];
	rxbuf->readlen += len;

	return ret;
}

static uint8_t G24_RecvByte(rxbuffer* rxbuf)
{
	uint8_t ret;
	
	if(rxbuf->len <= rxbuf->readlen)
		return 0;

	ret = rxbuf->buffer[rxbuf->readlen];
	rxbuf->readlen++;
	return ret;
}


#ifdef CMD_ENABLE

#define CONFIG_ADDR  			(60*1024 + FLASH_BASE)


#define CONFIG_ADDR_NUM_COUNT 	0


#define CONFIG_NUM (CONFIG_ADDR_NUM_COUNT+1)
void CMD_CB(void);

static uint16_t sg_flashbuffer[CONFIG_NUM];

static void flash_erase()
{
	FLASH_EraseInitTypeDef flash_eraseop;
	uint32_t erase_addr;
	 
	HAL_FLASH_Unlock(); 
    flash_eraseop.TypeErase = FLASH_TYPEERASE_PAGES;        /* ??????2ив3y */
    flash_eraseop.Banks = FLASH_BANK_1;
    flash_eraseop.NbPages = 1;
    flash_eraseop.PageAddress = CONFIG_ADDR;  /* и░a2ив3yж╠?ижии?? */
    HAL_FLASHEx_Erase( &flash_eraseop, &erase_addr);
	HAL_FLASH_Lock(); 
}


static uint16_t flash_read_halfword(uint32_t faddr)
{
    return *(volatile uint16_t *)faddr;
}

static void flash_write_halfword(uint16_t config,uint16_t value)
{
	for(uint8_t i=0;i<CONFIG_NUM;i++)
	{
		sg_flashbuffer[i] = flash_read_halfword(CONFIG_ADDR + 2*i);
	}

	sg_flashbuffer[config] = value;
	flash_erase();

	HAL_FLASH_Unlock(); 
	for(uint8_t i=0;i<CONFIG_NUM;i++)
	{		
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, CONFIG_ADDR + 2*i, sg_flashbuffer[i]);	
	}
	HAL_FLASH_Lock();
}

static void G24_AddrInit()
{
	uint16_t num = flash_read_halfword(CONFIG_ADDR);
	if(1 < num && 7 > num)
	{
		for(uint8_t i=1;i<num;i++)
		{
			NRF24L01_SetRxAddress(RxAddrssType[i], RxAddrss[i], 2000);
		}

	}
}


#endif


void G24_Init()
{

	initializeQueue(&sg_rxqueue_free);
	initializeQueue(&sg_rxqueue_used);

	for(uint8_t i=0;i<MAX_BUFFER_NUM;i++)
	{
		enqueue(&sg_rxqueue_free,&sg_g24databuffer[i]);
	}

	NRF24L01_RxInit(G24_CHANNEL, NRF24L01_DATA_RATE_1MBPS, 2000);
	NRF24L01_SetRxAddress(RxAddrssType[0], RxAddrss[0], 2000);
	

	VPC_ReceiveClear();

#ifdef CMD_ENABLE

	G24_AddrInit();
	VPC_Receive(sg_cmdbuffer,CMD_LEN,CMD_CB);
#endif
}


g24_data* G24_Get()
{
	static g24_buffer data = {0};	
	static rxbuffer* rxbuf = NULL;
	uint8_t* head;
	uint8_t type;

	static uint8_t seriel_kb = 0;
	static uint8_t seriel_ms = 0;
	static uint8_t seriel_vp = 0;

GET_HEAD:

	if(NULL != rxbuf)
	{
		G24_PutFreeBuffer(rxbuf);	
		rxbuf = NULL;
	}
	
	rxbuf = G24_RxData();

	head = G24_RecvData(rxbuf,G24_HEAD_LEN-1);
	if(strncmp((char*)head,VERSION,G24_HEAD_LEN-1))
	{
		LED_ShowSignE();
		ERROR_STAT(ERROR_VERSION);
		goto GET_HEAD;
	}
	
	data.data.type = G24_RecvByte(rxbuf);
	data.serial = G24_RecvByte(rxbuf);
	data.data.len = G24_RecvByte(rxbuf);
	type = data.data.type;

	switch(type)
	{
		case G24_TYPE_KEYBOARD:
			if((USBD_KEYBOARD_REPORT_SIZE+1) != data.data.len)
			{
				LED_ShowSignE();
				ERROR_STAT(ERROR_KB_LEN);
				goto GET_HEAD;
			}

			if((data.serial-seriel_kb) != 1)
			{
				ERROR_STAT(ERROR_KB_SERIAL);
			}
			seriel_kb = data.serial;
			break;
		case G24_TYPE_MOUSE:
			if(sizeof(hid_mouse) != data.data.len)
			{
				ERROR_STAT(ERROR_MOUSE_LEN);
				LED_ShowSignE();
				goto GET_HEAD;
			}
			if((data.serial-seriel_ms) != 1)
			{
				ERROR_STAT(ERROR_MOUSE_SERIAL);
			}
			seriel_ms = data.serial;
			break;
		case G24_TYPE_VPC:
			if(VPC_BUFFER_LEN <= data.data.len)
			{
				ERROR_STAT(ERROR_VPC_LEN);
				LED_ShowSignE();
				goto GET_HEAD;
			}
			if((data.serial-seriel_vp) != 1)
			{
				ERROR_STAT(ERROR_VPC_SERIAL);
			}
			seriel_vp = data.serial;
			break;
		default :
			ERROR_STAT(ERROR_TYPE);
			LED_ShowSignE();
			goto GET_HEAD;
	}

	data.data.data = G24_RecvData(rxbuf,data.data.len);
	data.crc = G24_RecvByte(rxbuf);
	if(data.crc != crc8(data.data.data,data.data.len))
	{
		switch(type)
		{
			case G24_TYPE_KEYBOARD:
				ERROR_STAT(ERROR_KB_CRC);
				break;
			case G24_TYPE_MOUSE:
				ERROR_STAT(ERROR_MOUSE_CRC);
				break;
			case G24_TYPE_VPC:
				ERROR_STAT(ERROR_VPC_CRC);
				break;
		}
		
		LED_ShowSignE();
		goto GET_HEAD;
	}

	switch(type)
	{
		case G24_TYPE_KEYBOARD:
			data.data.data++;
			data.data.len--;
			LED_ShowSignK();
			break;
		case G24_TYPE_MOUSE:
			LED_ShowSignM();
			break;
		case G24_TYPE_VPC:
			LED_ShowSignV();
			break;
	}


	TRACELOG("Get SUCCESS\r\n");
	return &data.data;
}





static void Rx_Really()
{
	rxbuffer* buffer;
	
	if(sgv_RxIRQfg)
	{
		sgv_RxIRQfg = FALSE;
		buffer = G24_GetFreeBuffer();
		if(buffer)
		{
			NRF24L01_RxReceive(buffer->buffer, &buffer->len, 2000);
			G24_PutUsedBuffer(buffer);
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{	
	if(GPIO_Pin == NRF24L01_IRQ_GPIO_PIN)
	{
		sgv_RxIRQfg = TRUE;
	}
}

static void Timer_Really()
{
	if(sgv_TimerOutfg)
	{
		sgv_TimerOutfg = FALSE;
		LED_Show();
	}
}

void TimerOut_Callback()
{
	sgv_TimerOutfg = TRUE;
}


#ifdef CMD_ENABLE
static void CMD_CB()
{
	sgv_CMDfg = TRUE;
}

#define CMD_GET "[%s]\r\n\tchan:\t%d\r\n\tbps:\t1Mbps\r\n"
#define CMD_GET_ADDR_1 "\t\taddr(0x%02X):\t0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n"
#define CMD_GET_ADDR_2 "\t\taddr(0x%02X):\t0x%02X\r\n"


static void CMD_Get()
{
	uint8_t buffer[128];
	uint16_t num ;

	snprintf((char*)buffer,128,CMD_GET,sg_cmdbuffer,G24_CHANNEL);
	VPC_Transmit(buffer, strlen((char*)buffer));
	
	num = flash_read_halfword(CONFIG_ADDR);

	for(uint8_t i=0;i<num;i++)
	{
		if(i <= 1)
			snprintf((char*)buffer,128,CMD_GET_ADDR_1,RxAddrssType[i], RxAddrss[i][0], RxAddrss[i][1], RxAddrss[i][2], RxAddrss[i][3], RxAddrss[i][4]);
		else
			snprintf((char*)buffer,128,CMD_GET_ADDR_2,RxAddrssType[i], RxAddrss[i][0]);
		VPC_Transmit(buffer, strlen((char*)buffer));
	}

}

static void CMD_Aad()
{
	uint16_t num ;

	num = flash_read_halfword(CONFIG_ADDR);

	if(num>=6)
	{
		VPC_Transmit("[CMD:AAD]\r\n\tAddr reach the max!\r\n", strlen((char*)"[CMD:AAD]\r\n\tAddr reach the max!\r\n"));
		return;
	}

	num++;
	flash_write_halfword(CONFIG_ADDR_NUM_COUNT,num);

	CMD_Get();

	VPC_Transmit("\tAdd addr succecc.restart,please!\r\n", strlen((char*)"\tAdd addr succecc.restart,please!\r\n"));
}

static void CMD_Dad()
{
	uint16_t num ;

	num = flash_read_halfword(CONFIG_ADDR);

	if(num<=1)
	{
		VPC_Transmit("[CMD:DAD]\r\n\tAddr reach the min!\r\n", strlen((char*)"[CMD:DAD]\r\n\tAddr reach the min!\r\n"));
		return;
	}

	num--;
	flash_write_halfword(CONFIG_ADDR_NUM_COUNT,num);
	CMD_Get();
	VPC_Transmit("\tDelete addr succecc.restart,please!\r\n", strlen((char*)"\tDelete addr succecc.restart,please!\r\n"));
}

static void CMD_Sta()
{
	uint8_t buffer[254];
	VPC_Transmit("[CMD:STA]\r\n", strlen((char*)"[CMD:STA]\r\n"));

	VPC_Transmit("\t[Type err]\r\n", strlen((char*)"\t[Type err]\r\n"));
	snprintf((char*)buffer,254,"\t\ttype error:\t%d\r\n",sg_errstatistics[ERROR_TYPE]);
	VPC_Transmit(buffer, strlen((char*)buffer));

	VPC_Transmit("\t[Length err]\r\n", strlen((char*)"\t[Length err]\r\n"));
	snprintf((char*)buffer,254,"\t\tkeyboard:\t%d\r\n\t\tmouse:\t%d\r\n\t\tvpc:\t%d\r\n"
		,sg_errstatistics[ERROR_KB_LEN],sg_errstatistics[ERROR_MOUSE_LEN],sg_errstatistics[ERROR_VPC_LEN]);
	VPC_Transmit(buffer, strlen((char*)buffer));

	VPC_Transmit("\t[Serial err]\r\n", strlen((char*)"\t[Serial err]\r\n"));
	snprintf((char*)buffer,254,"\t\tkeyboard:\t%d\r\n\t\tmouse:\t%d\r\n\t\tvpc:\t%d\r\n"
		,sg_errstatistics[ERROR_KB_SERIAL],sg_errstatistics[ERROR_MOUSE_SERIAL],sg_errstatistics[ERROR_VPC_SERIAL]);
	VPC_Transmit(buffer, strlen((char*)buffer));
	
	VPC_Transmit("\t[CRC err]\r\n", strlen((char*)"\t[CRC err]\r\n"));
	snprintf((char*)buffer,254,"\t\tkeyboard:\t%d\r\n\t\tmouse:\t%d\r\n\t\tvpc:\t%d\r\n"
		,sg_errstatistics[ERROR_KB_CRC],sg_errstatistics[ERROR_MOUSE_CRC],sg_errstatistics[ERROR_VPC_CRC]);
	VPC_Transmit(buffer, strlen((char*)buffer));
}



static void CMD_Do(void)
{
	sg_cmdbuffer[CMD_LEN] = 0;
	if(0 == strncmp((char*)sg_cmdbuffer,"CMD:GET",CMD_LEN))
	{
		CMD_Get();
	}
	else if(0 == strncmp((char*)sg_cmdbuffer,"CMD:AAD",CMD_LEN))
	{
		CMD_Aad();
	}
	else if(0 == strncmp((char*)sg_cmdbuffer,"CMD:DAD",CMD_LEN))
	{
		CMD_Dad();
	}
	else if(0 == strncmp((char*)sg_cmdbuffer,"CMD:STA",CMD_LEN))
	{
		CMD_Sta();
	}
}


static void CMD_Really()
{
	if(sgv_CMDfg)
	{
		sgv_CMDfg = FALSE;
		CMD_Do();
		VPC_ReceiveClear();
		VPC_Receive(sg_cmdbuffer,CMD_LEN,CMD_CB);
	}
}
#endif
static void do_idle()
{
	Rx_Really();
	Timer_Really();
	//CMD_Really();
}


