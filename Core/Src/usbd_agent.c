
#include "usb_device.h"
#include "usbd_cdc_acm_if.h"
#include "usbd_hid_keyboard.h"
#include "usbd_hid_mouse.h"
#include "usbd_agent.h"

#define VPC_RX_BUFFER_SIZE 		2048

static uint8_t sg_vpc_rxbuf[VPC_RX_BUFFER_SIZE] = {0};
volatile static uint32_t sg_vpc_rxbuf_in = 0;
volatile static uint32_t sg_vpc_rxbuf_out = 0;

volatile static uint8_t* 	sg_recvbuffaddr = NULL;
volatile static uint32_t 	sg_recvbufflen = 0;
static VPC_CB				sg_recvappcb = NULL;


static void VPC_Receive_APPCB()
{
	if(sg_recvbuffaddr&&sg_recvappcb)
	{
		if(((sg_vpc_rxbuf_in-sg_vpc_rxbuf_out)%VPC_RX_BUFFER_SIZE) >= sg_recvbufflen)
		{
			if((sg_vpc_rxbuf_out+sg_recvbufflen) <= VPC_RX_BUFFER_SIZE)
			{
				memcpy((uint8_t*)sg_recvbuffaddr,&sg_vpc_rxbuf[sg_vpc_rxbuf_out],sg_recvbufflen);
				sg_vpc_rxbuf_out += sg_recvbufflen;
			}
			else
			{
				memcpy((uint8_t*)sg_recvbuffaddr,&sg_vpc_rxbuf[sg_vpc_rxbuf_out],VPC_RX_BUFFER_SIZE - sg_vpc_rxbuf_out);
				sg_recvbufflen = sg_recvbufflen + sg_vpc_rxbuf_out - VPC_RX_BUFFER_SIZE;
				memcpy((uint8_t*)sg_recvbuffaddr+VPC_RX_BUFFER_SIZE - sg_vpc_rxbuf_out,&sg_vpc_rxbuf[0],sg_recvbufflen);
				sg_vpc_rxbuf_out = sg_recvbufflen;
				
			}

			sg_recvbufflen = 0;
			sg_recvbuffaddr = NULL;
			sg_recvappcb();
		}
	}
}


void VPC_Receive_CB(uint8_t *Buf, uint32_t len)
{	
	if((VPC_RX_BUFFER_SIZE-sg_vpc_rxbuf_in)>= len)
	{
		memcpy(&sg_vpc_rxbuf[sg_vpc_rxbuf_in],Buf,len);
		sg_vpc_rxbuf_in += len;
	}
	else
	{
		memcpy(&sg_vpc_rxbuf[sg_vpc_rxbuf_in],Buf,VPC_RX_BUFFER_SIZE-sg_vpc_rxbuf_in);
		len = len + sg_vpc_rxbuf_in -  VPC_RX_BUFFER_SIZE;
		memcpy(&sg_vpc_rxbuf[0],Buf+VPC_RX_BUFFER_SIZE-sg_vpc_rxbuf_in,len);
		sg_vpc_rxbuf_in = len;
	}

	VPC_Receive_APPCB();
}

uint8_t VPC_Receive(uint8_t*Buf,uint32_t Len,VPC_CB appcb)
{
	if(sg_recvbufflen || VPC_RX_BUFFER_SIZE < Len || NULL == Buf || NULL == appcb)
		return 0;
	sg_recvbufflen = Len;
	sg_recvbuffaddr = Buf;	
	sg_recvappcb = appcb;	
	
	return 1;
}

void VPC_ReceiveClear()
{

	sg_vpc_rxbuf_out = sg_vpc_rxbuf_in;
}




uint8_t VPC_Transmit(uint8_t *Buf, uint16_t Len)
{
	HAL_Delay(10);
	return (USBD_OK == CDC_Transmit(0,Buf,Len));
}

uint8_t Keyboard_Transmit(uint8_t *report, uint16_t Len)
{

	return (USBD_OK == USBD_HID_Keybaord_SendReport(&hUsbDevice,report, Len));

}
uint8_t Mouse_Transmit(uint8_t *report, uint16_t Len)
{

	return (USBD_OK == USBD_HID_Mouse_SendReport(&hUsbDevice,report, Len));

}


