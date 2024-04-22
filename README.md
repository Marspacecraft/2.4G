# 2.4G键盘鼠标无线接收器
    stm32f103c8主控
    nrf24l01无线模块接受
## nrf24无线通信协议
    见[G24.h](Core/Inc/G24.h)文件
    3D-freecad做的一些3D模型
    Frameware-键盘固件
        bootloader-引导固件
        PaceandLoveKeyboard-键盘固件
            CherryUSB-usbd代理，主要实现usb的hid和mcp的串口设别
            core-键盘主要代码
                Epaper，Oled，Fonts-调试的一些显示模块驱动
                Flash-外部flash，无法作为MSC设别，暂未调试成功
                joystick-摇杆驱动
                Keyboard-键盘相关代码
                Src-系统启动等相关代码
            Fatfs-文件系统。主要用于stm32内部flash和外部flash
            OBJ-键盘固件生成目录
            SFUD-外部flash驱动
            Ymodem-串口Ymodem协议，用于串口传输文件
    Hardware-嘉立创做的硬件电路（Flash处可能存在问题，无法做将外部flash做成MSC，问题目前没有解决，flash暂时没有使用）---本人纯属自学电路制作，电路图仅供参考。
    BLE-低功耗蓝牙，采用的是nrf52832，融合了nrf的两个demo，数据可以正常发送，但问题太多，以后有时间解决。

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

## 灯效
    见[rgbmode.h](Core/Inc/G24.h)文件说明


