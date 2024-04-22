# 2.4G键盘鼠标无线接收器
    stm32f103c8主控
    nrf24l01无线模块接受
## nrf24无线通信协议
    见[G24.h](https://github.com/Marspacecraft/2.4G/blob/main/Core/Inc/G24.h)文件
## 键盘报告描述符
    
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


