
#ifndef _USBD_AGENT_H_
#define _USBD_AGENT_H_

typedef void (*VPC_CB)(void);


uint8_t VPC_Transmit(uint8_t *Buf, uint16_t Len);

uint8_t VPC_Receive(uint8_t*Buf,uint32_t Len,VPC_CB appcb);
void VPC_ReceiveClear(void);

uint8_t Keyboard_Transmit(uint8_t *report, uint16_t Len);
uint8_t Mouse_Transmit(uint8_t *report, uint16_t Len);





#endif

