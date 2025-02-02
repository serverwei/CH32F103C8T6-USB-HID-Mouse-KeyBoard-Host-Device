#ifndef __USB_ENDP_H
#define __USB_ENDP_H

#include "ch32f10x_usb.h"

uint8_t USBD_ENDPx_DataUp(uint8_t endp, uint8_t *pbuf, uint16_t len);

#endif // !__USB_ENDP_H
