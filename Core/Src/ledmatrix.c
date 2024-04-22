
#include <string.h>
#include "gpio.h"
#include "ledmatrix.h"
#include "main.h"

typedef struct
{
	GPIO_TypeDef * port;
	uint16_t pin;
}led_coordinate;


static led_coordinate sg_led_coord_x[8] = 
{
	{LED0_0_GPIO_Port,LED0_0_Pin},
	{LED01_GPIO_Port,LED01_Pin},
	{LED02_GPIO_Port,LED02_Pin},
	{LED03_GPIO_Port,LED03_Pin},
	{LED04_GPIO_Port,LED04_Pin},
	{LED05_GPIO_Port,LED05_Pin},
	{LED06_GPIO_Port,LED06_Pin},
	{LED07_GPIO_Port,LED07_Pin},

	
};

static led_coordinate sg_led_coord_y[8] = 
{

	{LED_00_GPIO_Port,LED_00_Pin},
	{LED10_GPIO_Port,LED10_Pin},
	{LED20_GPIO_Port,LED20_Pin},
	{LED30_GPIO_Port,LED30_Pin},
	{LED40_GPIO_Port,LED40_Pin},
	{LED50_GPIO_Port,LED50_Pin},
	{LED60_GPIO_Port,LED60_Pin},
	{LED70_GPIO_Port,LED70_Pin},
};

#define LED_SET_X(COORD)		HAL_GPIO_WritePin(sg_led_coord_x[(COORD)].port, sg_led_coord_x[(COORD)].pin,GPIO_PIN_SET)
#define LED_RESET_X(COORD)		HAL_GPIO_WritePin(sg_led_coord_x[(COORD)].port, sg_led_coord_x[(COORD)].pin,GPIO_PIN_RESET)

#define LED_SET_Y(COORD)		HAL_GPIO_WritePin(sg_led_coord_y[(COORD)].port, sg_led_coord_y[(COORD)].pin,GPIO_PIN_SET)
#define LED_RESET_Y(COORD)		HAL_GPIO_WritePin(sg_led_coord_y[(COORD)].port, sg_led_coord_y[(COORD)].pin,GPIO_PIN_RESET)


#define LED_SIGN_NUM 24

uint8_t sign_k[LED_SIGN_NUM] =
{
	0x18,0x3C,0x66,0xC3,0x81,0x18,0x3C,0x66,
	0xC3,0x81,0x18,0x3C,0x66,0xC3,0x81,0x00,
	0xE1,0x33,0x1B,0x0F,0x1F,0x33,0x63,0xE3,

};

uint8_t sign_m[LED_SIGN_NUM] =
{
	0x18,0x3C,0x66,0xC3,0x81,0x18,0x3C,0x66,
	0xC3,0x81,0x18,0x3C,0x66,0xC3,0x81,0x00,
	0x24,0x66,0x66,0x7E,0xDB,0xDB,0x99,0x81,
};

uint8_t sign_v[LED_SIGN_NUM] =
{
	0x18,0x3C,0x66,0xC3,0x81,0x18,0x3C,0x66,
	0xC3,0x81,0x18,0x3C,0x66,0xC3,0x81,0x00,
	0x00,0xC3,0x66,0x66,0x66,0x24,0x3C,0x18,
};

uint8_t sign_e[LED_SIGN_NUM] =
{
	0xC3,0x66,0x3C,0x18,0x3C,0x66,0xC3,0x00,
	0xC3,0x66,0x3C,0x18,0x3C,0x66,0xC3,0x00,
	0xC3,0x66,0x3C,0x18,0x3C,0x66,0xC3,0x81,
};


//#define LED_DATA_IN  0
//#define LED_DATA_OUT 1


typedef struct
{
	uint8_t* sign;
	uint8_t pos;
	//uint8_t fg;
	uint8_t priority;
}led_buffer;


#define LED_PRIORITY_K 1
#define LED_PRIORITY_M 2
#define LED_PRIORITY_V 3
#define LED_PRIORITY_E 4


#define LED_PRIORITY_NONE 0



#define LED_BUFFER_WORKING 0
#define LED_BUFFER_NEXT 	0
static led_buffer sg_ledbuffer[2] = {0};

#define LED_SHOW_SPEED 		200
#define LED_SHOW_BRIGHT 	1//5ms一个周期
#define LED_SHOW_BRIGHT_UP 1//66%亮度



static void LED_ShowLine(uint8_t line,uint8_t x)
{
	line %= 8;
	if(line)
		LED_SET_Y(line-1);
	else
		LED_SET_Y(7);

	for(uint8_t i=0;i<8;i++)
	{
		if(x & 0x80)
		{
			LED_SET_X(i);
		}
		else
		{
			LED_RESET_X(i);
		}
		x <<= 1;
	}
	
	LED_RESET_Y(line);
}


static uint8_t LED_SpeedCtrl()
{
	static uint8_t cout = 0;

	if(LED_SHOW_SPEED <= cout)
	{
		cout = 0;
		sg_ledbuffer[LED_BUFFER_WORKING].pos++;
		if((LED_SIGN_NUM+8) < sg_ledbuffer[LED_BUFFER_WORKING].pos)
		{
			sg_ledbuffer[LED_BUFFER_WORKING].priority = LED_PRIORITY_NONE;
			return FALSE;
		}
		return TRUE;
	}
	cout++;
	
	return TRUE;
}

static uint8_t LED_ShowCtrl()
{
	if(LED_PRIORITY_NONE == sg_ledbuffer[LED_BUFFER_WORKING].priority 
		&& LED_PRIORITY_NONE == sg_ledbuffer[LED_BUFFER_NEXT].priority)
	{
		return FALSE;
	}

	if(LED_PRIORITY_NONE == sg_ledbuffer[LED_BUFFER_WORKING].priority)
	{
		memcpy(&sg_ledbuffer[LED_BUFFER_WORKING],&sg_ledbuffer[LED_BUFFER_NEXT],sizeof(led_buffer));
		sg_ledbuffer[LED_BUFFER_NEXT].priority = LED_PRIORITY_NONE;
		LED_Clear();
	}


	return TRUE;
}

static void LED_Scan()
{
	static uint8_t showline = 0;
	uint8_t pos = sg_ledbuffer[LED_BUFFER_WORKING].pos;
	uint8_t value = 0;
	
	//if(LED_DATA_IN == sg_ledbuffer[LED_BUFFER_NEXT].fg)
	{
		/**********
		pos		7	6	5
		0		0	N	N
		20		20 	19 	18
		25		25	24 	23
		**********/

		if(((7-showline) > pos)|| LED_SIGN_NUM <= (pos+showline-7))
			value = 0;
		else 
			value = sg_ledbuffer[LED_BUFFER_WORKING].sign[pos+showline-7];
	
	}
#if 0	
	else
	{
		/**********
		pos		0	1	2
		0		0	N	N
		20		20 	19 	18
		25		25	24	23
		**********/

		if((showline>pos)||(LED_SIGN_NUM <= (pos-showline)))
			value = 0;
		else
			value = sg_ledbuffer[LED_BUFFER_WORKING].sign[pos-showline];
	}

#endif
	LED_ShowLine(showline,value);

	showline++;
	if(8 <= showline)
		showline = 0;
}



void LED_Show()//10ms
{
	if(FALSE == LED_ShowCtrl())
	{
		return;
	}

	if(FALSE == LED_SpeedCtrl())
	{
		return;
	}

	LED_Scan();
}


void LED_Clear()
{
	for(uint8_t i=0;i<8;i++)
	{
		LED_RESET_X(i);
		LED_SET_Y(i);
	}
}


//#define TEST_LED
#ifdef TEST_LED

void test_show()
{
	while(1)
	{
		for(uint8_t i=0;i<8;i++)
		{
				LED_ShowLine(i,sign_k[i]);
				HAL_Delay(1);
		}
	}
}

void test_bit()
{
	for(uint8_t i=0;i<8;i++)
	{
		for(uint8_t j=0;j<8;j++)
		{
			LED_ShowLine(i,1<<j);
			HAL_Delay(1000);
		}
	}
}


void test_led()
{
	//test_show();
	LED_ShowSignE();
}

#endif


void LED_ShowInit(void)
{
	memset(sg_ledbuffer,0,sizeof(sg_ledbuffer));
	LED_Clear();

	#ifdef TEST_LED
	test_led();
	#endif
}


void LED_ShowSignK(void)
{
	if(sg_ledbuffer[LED_BUFFER_NEXT].priority < LED_PRIORITY_K)
	{
		sg_ledbuffer[LED_BUFFER_NEXT].priority = LED_PRIORITY_K;
		//sg_ledbuffer[LED_BUFFER_NEXT].fg = LED_DATA_IN ;
		sg_ledbuffer[LED_BUFFER_NEXT].pos = 0;
		sg_ledbuffer[LED_BUFFER_NEXT].sign = sign_k;
	}
}

void LED_ShowSignM(void)
{
	if(sg_ledbuffer[LED_BUFFER_NEXT].priority < LED_PRIORITY_M)
	{
		sg_ledbuffer[LED_BUFFER_NEXT].priority = LED_PRIORITY_M;
		//sg_ledbuffer[LED_BUFFER_NEXT].fg = LED_DATA_IN ;
		sg_ledbuffer[LED_BUFFER_NEXT].pos = 0;
		sg_ledbuffer[LED_BUFFER_NEXT].sign = sign_m;
	}
}
void LED_ShowSignV(void)
{
	if(sg_ledbuffer[LED_BUFFER_NEXT].priority < LED_PRIORITY_V)
	{
		sg_ledbuffer[LED_BUFFER_NEXT].priority = LED_PRIORITY_V;
		//sg_ledbuffer[LED_BUFFER_NEXT].fg = LED_DATA_IN ;
		sg_ledbuffer[LED_BUFFER_NEXT].pos = 0;
		sg_ledbuffer[LED_BUFFER_NEXT].sign = sign_v;
	}
}

void LED_ShowSignE(void)
{
	if(sg_ledbuffer[LED_BUFFER_NEXT].priority < LED_PRIORITY_E)
	{
		sg_ledbuffer[LED_BUFFER_NEXT].priority = LED_PRIORITY_E;
		//sg_ledbuffer[LED_BUFFER_NEXT].fg = LED_DATA_IN ;
		sg_ledbuffer[LED_BUFFER_NEXT].pos = 0;
		sg_ledbuffer[LED_BUFFER_NEXT].sign = sign_e;
	}
}




