/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_pwr.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2019/10/15
 * Description        : Connection/disconnection & power management header
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __USB_PWR_H
#define __USB_PWR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32f10x.h"
typedef enum _RESUME_STATE {
    RESUME_EXTERNAL,
    RESUME_INTERNAL,
    RESUME_LATER,
    RESUME_WAIT,
    RESUME_START,
    RESUME_ON,
    RESUME_OFF,
    RESUME_ESOF
} RESUME_STATE;

typedef enum _DEVICE_STATE {
    UNCONNECTED, // 未连接
    ATTACHED,
    POWERED,
    SUSPENDED, // 暂停
    ADDRESSED,
    CONFIGURED // 已配置
} DEVICE_STATE;

void Suspend(void);
void Resume_Init(void);
void Resume(RESUME_STATE eResumeSetVal);
RESULT PowerOn(void);
RESULT PowerOff(void);

extern __IO uint32_t bDeviceState; /* USB device status */
extern __IO bool fSuspendEnabled;  /* true when suspend is possible */

#ifdef __cplusplus
}
#endif

#endif /*__USB_PWR_H*/
