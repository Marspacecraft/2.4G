
#ifndef _G24_H_
#define _G24_H_
#include "stdint.h"
#include "usbd_agent.h"

/******************************************
	2.4G串口协议

	请求：
		头+版本+类型+序号+数据长度+数据+数据crc
		头（4 byte） "PLKB"
		版本（1 byte）"1"
		类型 （1 byte）'K' 'M' 'V' 'H'
		序号（1 byte） 0,1,2~255
		数据长度（1 byte）0~23
		数据（n byte）
		数据CRC（1 byte）
	
******************************************/

#define G24_HEAD_LEN 		6
#define VERSION 			"PLKB1"

#define G24_TYPE_KEYBOARD 	'K'
#define G24_TYPE_MOUSE 		'M'
#define G24_TYPE_VPC 		'V'

#pragma pack(push) 
#pragma pack(1)  

#define USBD_KEYBOARD_REPORT_SIZE 			21//22

/*************************************************************

	8bit功能按键位
	8bit填充位
	6*8bit所有按键位
	104bit普通按键位
	5bit LED显示位
	3bit填充位
	
	共22byte


keyboard hid report discriber:

 	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)  
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
	0x95, 0x06, 	   //	Report Count (6)
	0x75, 0x08, 	   //	Report Size (8)
	0x15, 0x00, 	   //	Logical Minimum (0)
	0x25, USBD_KEYBOARD_REPORT_CODEC_EXT_MAX, 	   //	Logical Maximum (231)
	0x05, 0x07, 	   //	Usage Page (Kbrd/Keypad)
	0x19, 0x00, 	   //	Usage Minimum (0x00)
	0x29, USBD_KEYBOARD_REPORT_CODEC_EXT_MAX, 	   //	Usage Maximum (0xe7)
	0x81, 0x00, 	   //	Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)



	 
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, USBD_KEYBOARD_REPORT_CODEC_START,                    //   USAGE_MINIMUM (Keyboard a and A)
    0x29, USBD_KEYBOARD_REPORT_CODEC_MAX,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x68,                    //   REPORT_COUNT (104)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0xc0                           // END_COLLECTION




*************************************************************/
/*!< keyboard report struct */
typedef struct
{
	uint8_t bioskeypos;//填充位
	uint8_t buffer[USBD_KEYBOARD_REPORT_SIZE];
}hid_keyboard;

/*!< mouse report struct */
typedef struct  
{
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
}hid_mouse;

typedef struct
{
	#define VPC_BUFFER_LEN 23//nrf24l01 数据帧最大长度32byte，扣除其他长度最多23byte
	uint8_t data[VPC_BUFFER_LEN];

}intf_vpc;


typedef struct
{
	uint8_t 	type;
	uint8_t 	len;
	uint8_t*	data;
}g24_data;


#pragma pack(pop) 

void G24_Init(void);
g24_data* G24_Get(void);


#endif






