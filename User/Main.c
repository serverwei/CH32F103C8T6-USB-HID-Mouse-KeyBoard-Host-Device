#include "ch32f10x.h"

#include "Main.h"
#include "tim.h"

#include "debug.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_prop.h"
#include "usb_endp.h"

#include "usb_host_config.h"
#include "app_km.h"
#include "usb_hid_mskb.h"
/* Global Variable */
#define KB_EP ENDP1
#define MS_EP ENDP2

uint8_t index = 0;
// uint8_t hub_port = 0;
uint8_t intf_num = 0, in_num = 0;
uint8_t status = 0;

__IO uint8_t KB_LED_Last_Status = 0;
__IO uint8_t SCAND_TIME_UP      = FALSE;
uint8_t usbd_buf[32]            = {0};
uint16_t usbd_buf_len           = 0;
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(921600);
    DUG_PRINTF("SystemClk:%d\r\n", (int)SystemCoreClock);
    DUG_PRINTF("ChipID:%08x\r\n", (int)DBGMCU_GetCHIPID());

    /* Initialize timer for Keyboard and mouse scan timing */
    TIM3_Init(999, 71);
    TIM_Cmd(TIM3, ENABLE);

    // USB从机初始化
    Set_USBConfig();
    USB_Init(PWR_VDD_SupplyVoltage());
    USB_Interrupts_Config();

    // USB主机初始化
    USBHD_RCC_Init();
    USBHD_Host_Init(ENABLE, PWR_VDD_SupplyVoltage());
    memset(&RootHubDev.bStatus, 0, sizeof(ROOT_HUB_DEVICE));
    memset(&HostCtl[DEF_USBHD_PORT_INDEX * DEF_ONE_USB_SUP_DEV_TOTAL].InterfaceNum, 0, DEF_ONE_USB_SUP_DEV_TOTAL * sizeof(HOST_CTL));

    while (1) {
        USBH_MainDeal();

        if (SCAND_TIME_UP) {
            SCAND_TIME_UP = FALSE;
            if (bDeviceState == CONFIGURED) {
                if (hid_ms.data_done) {
                    hid_ms.data_done = 0;
                    usbd_buf_len     = 0;
                    if (hid_ms.id_exist) {
                        usbd_buf[usbd_buf_len++] = hid_ms.id;
                    }
                    usbd_buf[usbd_buf_len++] = hid_ms.data[hid_ms.btn_shift];
                    usbd_buf[usbd_buf_len++] = hid_ms.data[hid_ms.x_shift];
                    usbd_buf[usbd_buf_len++] = hid_ms.data[hid_ms.y_shift];
                    usbd_buf[usbd_buf_len++] = hid_ms.data[hid_ms.wheel_shift];

                    USBD_ENDPx_DataUp(MS_EP, usbd_buf, usbd_buf_len);
                }

                if (hid_kb.data_done) {
                    hid_kb.data_done = 0;
                    usbd_buf_len     = 0;
                    if (hid_kb.id_exist) {
                        usbd_buf[usbd_buf_len++] = hid_kb.id;
                    }
                    usbd_buf[usbd_buf_len++] = hid_kb.data[hid_kb.key_spec_shift];
                    usbd_buf[usbd_buf_len++] = 0;
                    memcpy(&usbd_buf[usbd_buf_len], &hid_kb.data[hid_kb.key_norm_shift], 6);
                    usbd_buf_len += 6;
                    USBD_ENDPx_DataUp(KB_EP, usbd_buf, usbd_buf_len);
                }
            }
        }

        if (hid_kb.exist) {
            // 下发LED状态
            if (KB_LED_Cur_Status != hid_kb.led_status) {
                hid_kb.led_status = KB_LED_Cur_Status;
                KB_SetReport(hid_kb.index, RootHubDev.bEp0MaxPks, hid_kb.intfnum);
            }
        }
        // USB Device
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        // USB_Scan_Flag = TRUE;
        /* Clear interrupt flag */
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        SCAND_TIME_UP = TRUE;

        // USB主机轮询计数
        if (RootHubDev.bStatus >= ROOT_DEV_SUCCESS) {
            index = RootHubDev.DeviceIndex;
            if (RootHubDev.bType == USB_DEV_CLASS_HID) {
                for (intf_num = 0; intf_num < HostCtl[index].InterfaceNum; intf_num++) {
                    for (in_num = 0; in_num < HostCtl[index].Interface[intf_num].InEndpNum; in_num++) {

                        HostCtl[index].Interface[intf_num].InEndpTimeCount[in_num]++;
                    }
                }
            }
            // else if (RootHubDev.bType == USB_DEV_CLASS_HUB) {
            //     HostCtl[index].Interface[0].InEndpTimeCount[0]++;
            //     for (hub_port = 0; hub_port < RootHubDev.bPortNum; hub_port++) {
            //         if (RootHubDev.Device[hub_port].bStatus >= ROOT_DEV_SUCCESS) {
            //             index = RootHubDev.Device[hub_port].DeviceIndex;

            //             if (RootHubDev.Device[hub_port].bType == USB_DEV_CLASS_HID) {
            //                 for (intf_num = 0; intf_num < HostCtl[index].InterfaceNum; intf_num++) {
            //                     for (in_num = 0; in_num < HostCtl[index].Interface[intf_num].InEndpNum; in_num++) {
            //                         HostCtl[index].Interface[intf_num].InEndpTimeCount[in_num]++;
            //                     }
            //                 }
            //             }
            //         }
            //     }
            // }
        }
    }
}

void Delay_Init(void)
{
    TIM4_Init(0xFFFF, 71);
    TIM_Cmd(TIM4, ENABLE);
}

void Delay_Us(uint32_t n)
{
    TIM4->CNT = 0;
    while (n > TIM4->CNT) {}

    return;
}

void Delay_Ms(uint16_t n)
{
    while (n-- > 0) {
        Delay_Us(1000);
    }

    return;
}