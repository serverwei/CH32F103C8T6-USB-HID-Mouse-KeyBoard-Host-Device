#ifndef __USB_HID_MSKB_H
#define __USB_HID_MSKB_H

#define HID_DESKTOP_PAGE           0x01
#define HID_KEYCODE_PAGE           0x07
#define HID_LED_PAGE               0x08
#define HID_BUTTON_PAGE            0x09

#define HID_USAGE_POINTER          0x01
#define HID_USAGE_MOUSE            0x02
#define HID_USAGE_X                0x30
#define HID_USAGE_Y                0x31
#define HID_USAGE_WHEEL            0x38
#define HID_USAGE_KEYBOARD         0x06

#define HID_TAG_INTPU              0x80
#define HID_TAG_OUTPUT             0x90
#define HID_TAG_FEATURE            0xB0
#define HID_TAG_COLLECTION         0xA0
#define HID_TAG_ECOLLECTION        0xC0
#define HID_TAG_USAGE_PAGE         0x04
#define HID_TAG_LOGICAL_MIN        0x14
#define HID_TAG_LOGICAL_MAX        0x24
#define HID_TAG_PHYSICAL_MIN       0x34
#define HID_TAG_PHYSICAL_MAX       0x44
#define HID_TAG_REPORT_SIZE        0x74
#define HID_TAG_REPORT_ID          0x84
#define HID_TAG_REOIRT_CNT         0x94
#define HID_TAG_USAGE              0x08
#define HID_TAG_USAGE_MIN          0x18
#define HID_TAG_USAGE_MAX          0x28

#define HID_COLLECTION_PHYSICAL    0x00
#define HID_COLLECTION_APPLICATION 0x01

typedef struct _hid_ms_def {
    unsigned char index;
    unsigned char intfnum;
    volatile unsigned char exist;
    int id_exist;
    int id;
    unsigned char id_shift;
    unsigned char btn_nums;
    unsigned short btn_min;
    unsigned short btn_max;
    unsigned char btn_data_bits;
    unsigned char btn_const_bits;
    unsigned char btn_shift;
    unsigned char btn_bytes;
    int x_min;
    int x_max;
    unsigned char x_shift;
    unsigned char x_bytes;
    int y_min;
    int y_max;
    unsigned char y_shift;
    unsigned char y_bytes;
    int wheel_min;
    int wheel_max;
    unsigned char wheel_shift;
    unsigned char wheel_bytes;
    unsigned char len;
    unsigned char data[32];
    unsigned char data_done;
} hid_ms_def;

typedef struct _hid_kb_def {
    unsigned char index;
    unsigned char intfnum;
    volatile char exist;
    int id_exist;
    int id;
    unsigned char id_shift;
    unsigned char led_min;
    unsigned char led_max;
    unsigned char led_nums;
    unsigned char led_shift;
    unsigned char led_bytes;
    unsigned char led_status;
    unsigned char led_flag;
    unsigned char key_spec_nums;
    unsigned char key_spec_min;
    unsigned short key_spec_max;
    unsigned char key_spec_shift;
    unsigned char key_spec_bytes;
    unsigned short key_norm_nums;
    unsigned char key_norm_min;
    unsigned short key_norm_max;
    unsigned char key_norm_shift;
    unsigned char key_norm_bytes;
    unsigned char len;
    unsigned char data[32];
    unsigned char data_done;
} hid_kb_def;

extern hid_ms_def hid_ms;
extern hid_kb_def hid_kb;

unsigned char HID_MSandKeyboard(unsigned char *hid, unsigned char len);

#endif // !__USB_HID_DEFINE_H
