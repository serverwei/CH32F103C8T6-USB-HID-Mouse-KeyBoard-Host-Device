#include "string.h"
#include "usb_hid_mskb.h"

hid_ms_def hid_ms = {0};
hid_kb_def hid_kb = {0};

int HID_Data_Recovery(unsigned char *data, unsigned char size)
{
    unsigned int value = 0;
    switch (size) {
        case 1:
            value = (char)data[1];
            break;

        case 2: {
            short tmp = (short)data[2] << 8;
            tmp += (short)data[1] << 0;
            value = tmp;
        } break;
        case 3:
            value = (int)data[4] << 24;
            value += (int)data[3] << 16;
            value += (int)data[2] << 8;
            value += (int)data[1] << 0;
            break;
    }

    return value;
}

unsigned char HID_MSandKeyboard(unsigned char *hid, unsigned char len)
{
    const char tag_size_list[] = {0, sizeof(char), sizeof(short), sizeof(int)};
    unsigned char result       = 0;
    unsigned char tag_name     = 0;
    unsigned char tag_size     = 0;
    unsigned int report_size = 0, report_cnt = 0, tag_value = 0;
    unsigned char current_bits = 0, page_bits = 0;
    unsigned short total_bits = 0;
    unsigned char lu_size     = 0; // logic and usage size [7:6]Logic min, [5:4]Logic max, [3:2]Usage min, [1:0]Usage max
    int logic_min = 0, logic_max = 0, usage_min = 0, usage_max = 0;
    unsigned short usage_page_item[8] = {0};
    unsigned char usage_page_item_num = 0;
    unsigned char collection_cnt      = 0;
    unsigned short usage_page         = 0;
    unsigned char i = 0, j = 0;
    unsigned char ms_start = 0;
    unsigned char kb_start = 0;

    while (i < len) {
        tag_name = hid[i] & 0xFC;
        tag_size = tag_size_list[hid[i] & 0x03];
        if (tag_name == HID_TAG_USAGE && hid[i + 1] == HID_USAGE_MOUSE) {
            // found mouse usage
            hid_ms.exist = 1;
            ms_start     = i;
            result |= 0b01;
        } else if (tag_name == HID_TAG_USAGE && hid[i + 1] == HID_USAGE_KEYBOARD) {
            hid_kb.exist = 1;
            kb_start     = i;
            result |= 0b10;
        }
        i += tag_size + 1;
    }

    // mouse hid report
    if (result & 1) {
        i = ms_start;

        collection_cnt      = 0;
        usage_page_item_num = 0;
        total_bits          = 0;
        while (i < len) {
            tag_name = hid[i] & 0xFC;
            tag_size = tag_size_list[hid[i] & 0x03];

            if (tag_name == HID_TAG_COLLECTION) {
                collection_cnt++;
            } else if (tag_name == HID_TAG_ECOLLECTION) {
                collection_cnt--;
                if (!collection_cnt) {
                    // done
                    break;
                }
            } else if (tag_name == HID_TAG_USAGE_PAGE) {
                usage_page = HID_Data_Recovery(&hid[i], tag_size);
            } else if (tag_name == HID_TAG_REPORT_ID) {
                hid_ms.id       = HID_Data_Recovery(&hid[i], tag_size);
                hid_ms.id_shift = total_bits >> 3;
                total_bits += tag_size * 8;
            } else if (tag_name == HID_TAG_USAGE) {
                if (HID_Data_Recovery(&hid[i], tag_size) != HID_USAGE_POINTER) {
                    usage_page_item[usage_page_item_num++] = HID_Data_Recovery(&hid[i], tag_size);
                }
            } else if (tag_name == HID_TAG_LOGICAL_MIN) {
                logic_min = HID_Data_Recovery(&hid[i], tag_size);
                lu_size &= 0x3F;
                lu_size += (hid[i] & 0x03) << 6;
            } else if (tag_name == HID_TAG_LOGICAL_MAX) {
                logic_max = HID_Data_Recovery(&hid[i], tag_size);
                lu_size &= 0xCF;
                lu_size += (hid[i] & 0x03) << 4;
            } else if (tag_name == HID_TAG_USAGE_MIN) {
                usage_min = HID_Data_Recovery(&hid[i], tag_size);
                lu_size &= 0xF3;
                lu_size += (hid[i] & 0x03) << 2;
            } else if (tag_name == HID_TAG_USAGE_MAX) {
                usage_max = HID_Data_Recovery(&hid[i], tag_size);
                lu_size &= 0xFC;
                lu_size += (hid[i] & 0x03) << 0;
            } else if (tag_name == HID_TAG_REPORT_SIZE) {
                report_size = HID_Data_Recovery(&hid[i], tag_size);
            } else if (tag_name == HID_TAG_REOIRT_CNT) {
                report_cnt = HID_Data_Recovery(&hid[i], tag_size);
            } else if (tag_name == HID_TAG_INTPU) {
                // input type data
                current_bits = report_size * report_cnt;
                total_bits += current_bits;
                tag_value = HID_Data_Recovery(&hid[i], tag_size);

                switch (usage_page) {
                    case HID_BUTTON_PAGE: {
                        if (tag_value & 1) {
                            hid_ms.btn_data_bits = current_bits;
                        } else {
                            hid_ms.btn_const_bits = current_bits;
                        }
                        switch ((lu_size >> 2) & 0b11) {
                            case 0:
                                hid_ms.btn_min = 0;
                                break;
                            case 1:
                                hid_ms.btn_min = (unsigned char)usage_min;
                                break;
                            case 2:
                                hid_ms.btn_min = (unsigned short)usage_min;
                                break;
                            case 3:
                                hid_ms.btn_min = (unsigned int)usage_min;
                                break;
                        }
                        switch ((lu_size >> 0) & 0b11) {
                            case 0:
                                hid_ms.btn_max = 0;
                                break;
                            case 1:
                                hid_ms.btn_max = (unsigned char)usage_max;
                                break;
                            case 2:
                                hid_ms.btn_max = (unsigned short)usage_max;
                                break;
                            case 3:
                                hid_ms.btn_max = (unsigned int)usage_max;
                                break;
                        }
                        hid_ms.btn_nums  = hid_ms.btn_max - hid_ms.btn_min + 1;
                        hid_ms.btn_bytes = (hid_ms.btn_data_bits + hid_ms.btn_const_bits) >> 3;
                        hid_ms.btn_shift = (total_bits - (hid_ms.btn_bytes << 3)) >> 3;
                    } break;

                    case HID_DESKTOP_PAGE: {
                        for (j = 0; j < usage_page_item_num; j++) {
                            switch (usage_page_item[j]) {
                                case HID_USAGE_X:
                                    switch ((lu_size >> 6) & 0b11) {
                                        case 0:
                                            hid_ms.x_min = 0;
                                            break;
                                        case 1:
                                            hid_ms.x_min = (char)logic_min;
                                            break;
                                        case 2:
                                            hid_ms.x_min = (short)logic_min;
                                            break;
                                        case 3:
                                            hid_ms.x_min = logic_min;
                                            break;
                                    }
                                    switch ((lu_size >> 4) & 0b11) {
                                        case 0:
                                            hid_ms.x_max = 0;
                                            break;
                                        case 1:
                                            hid_ms.x_max = (char)logic_max;
                                            break;
                                        case 2:
                                            hid_ms.x_max = (short)logic_max;
                                            break;
                                        case 3:
                                            hid_ms.x_max = logic_max;
                                            break;
                                    }
                                    hid_ms.x_bytes = (current_bits / usage_page_item_num) >> 3;
                                    hid_ms.x_shift = (total_bits - (hid_ms.x_bytes << 3) * (usage_page_item_num - j)) >> 3;
                                    break;

                                case HID_USAGE_Y:
                                    switch ((lu_size >> 6) & 0b11) {
                                        case 0:
                                            hid_ms.y_min = 0;
                                            break;
                                        case 1:
                                            hid_ms.y_min = (char)logic_min;
                                            break;
                                        case 2:
                                            hid_ms.y_min = (short)logic_min;
                                            break;
                                        case 3:
                                            hid_ms.y_min = logic_min;
                                            break;
                                    }
                                    switch ((lu_size >> 4) & 0b11) {
                                        case 0:
                                            hid_ms.y_max = 0;
                                            break;
                                        case 1:
                                            hid_ms.y_max = (char)logic_max;
                                            break;
                                        case 2:
                                            hid_ms.y_max = (short)logic_max;
                                            break;
                                        case 3:
                                            hid_ms.y_max = logic_max;
                                            break;
                                    }
                                    hid_ms.y_bytes = (current_bits / usage_page_item_num) >> 3;
                                    hid_ms.y_shift = (total_bits - (hid_ms.y_bytes << 3) * (usage_page_item_num - j)) >> 3;
                                    break;

                                case HID_USAGE_WHEEL:
                                    switch ((lu_size >> 6) & 0b11) {
                                        case 0:
                                            hid_ms.wheel_min = 0;
                                            break;
                                        case 1:
                                            hid_ms.wheel_min = (char)logic_min;
                                            break;
                                        case 2:
                                            hid_ms.wheel_min = (short)logic_min;
                                            break;
                                        case 3:
                                            hid_ms.wheel_min = logic_min;
                                            break;
                                    }
                                    switch ((lu_size >> 4) & 0b11) {
                                        case 0:
                                            hid_ms.wheel_max = 0;
                                            break;
                                        case 1:
                                            hid_ms.wheel_max = (char)logic_max;
                                            break;
                                        case 2:
                                            hid_ms.wheel_max = (short)logic_max;
                                            break;
                                        case 3:
                                            hid_ms.wheel_max = logic_max;
                                            break;
                                    }
                                    hid_ms.wheel_bytes = (current_bits / usage_page_item_num) >> 3;
                                    hid_ms.wheel_shift = (total_bits - (hid_ms.wheel_bytes << 3) * (usage_page_item_num - j)) >> 3;
                                    break;

                                default:
                                    break;
                            }
                        }
                    } break;

                    default:
                        break;
                }
                usage_page_item_num = 0;
            }

            i += tag_size + 1;
        }
    }

    // keyboard hid report
    if ((result >> 1) & 1) {
        i = kb_start;

        collection_cnt      = 0;
        usage_page_item_num = 0;
        total_bits          = 0;
        while (i < len) {
            tag_name = hid[i] & 0xFC;
            tag_size = tag_size_list[hid[i] & 0x03];

            if (tag_name == HID_TAG_COLLECTION) {
                collection_cnt++;
            } else if (tag_name == HID_TAG_ECOLLECTION) {
                collection_cnt--;
                if (!collection_cnt) {
                    // done
                    break;
                }
            } else if (tag_name == HID_TAG_USAGE_PAGE) {
                page_bits  = 0;
                usage_page = HID_Data_Recovery(&hid[i], tag_size);
            } else if (tag_name == HID_TAG_REPORT_ID) {
                hid_kb.id       = HID_Data_Recovery(&hid[i], tag_size);
                hid_kb.id_exist = 1;
                hid_kb.id_shift = total_bits >> 3;
                total_bits += tag_size * 8;
            } else if (tag_name == HID_TAG_USAGE) {
                if (HID_Data_Recovery(&hid[i], tag_size) != HID_USAGE_POINTER) {
                    usage_page_item[usage_page_item_num++] = HID_Data_Recovery(&hid[i], tag_size);
                }
            } else if (tag_name == HID_TAG_LOGICAL_MIN) {
                logic_min = HID_Data_Recovery(&hid[i], tag_size);
                lu_size &= 0x3F;
                lu_size += (hid[i] & 0x03) << 6;
            } else if (tag_name == HID_TAG_LOGICAL_MAX) {
                logic_max = HID_Data_Recovery(&hid[i], tag_size);
                lu_size &= 0xCF;
                lu_size += (hid[i] & 0x03) << 4;
            } else if (tag_name == HID_TAG_USAGE_MIN) {
                usage_min = HID_Data_Recovery(&hid[i], tag_size);
                lu_size &= 0xF3;
                lu_size += (hid[i] & 0x03) << 2;
            } else if (tag_name == HID_TAG_USAGE_MAX) {
                usage_max = HID_Data_Recovery(&hid[i], tag_size);
                lu_size &= 0xFC;
                lu_size += (hid[i] & 0x03) << 0;
            } else if (tag_name == HID_TAG_REPORT_SIZE) {
                report_size = HID_Data_Recovery(&hid[i], tag_size);
            } else if (tag_name == HID_TAG_REOIRT_CNT) {
                report_cnt = HID_Data_Recovery(&hid[i], tag_size);
            } else if (tag_name == HID_TAG_INTPU) {
                // input type data
                current_bits = report_size * report_cnt;
                total_bits += current_bits;
                tag_value = HID_Data_Recovery(&hid[i], tag_size);

                switch (usage_page) {
                    case HID_KEYCODE_PAGE: {
                        if (!(tag_value & 1)) {
                            // data
                            if (logic_max == 1) {
                                switch ((lu_size >> 2) & 0b11) {
                                    case 0:
                                        hid_kb.key_spec_min = 0;
                                        break;
                                    case 1:
                                        hid_kb.key_spec_min = (unsigned char)usage_min;
                                        break;
                                    case 2:
                                        hid_kb.key_spec_min = (unsigned short)usage_min;
                                        break;
                                    case 3:
                                        hid_kb.key_spec_min = (unsigned int)usage_min;
                                        break;
                                }
                                switch ((lu_size >> 0) & 0b11) {
                                    case 0:
                                        hid_kb.key_spec_max = 0;
                                        break;
                                    case 1:
                                        hid_kb.key_spec_max = (unsigned char)usage_max;
                                        break;
                                    case 2:
                                        hid_kb.key_spec_max = (unsigned short)usage_max;
                                        break;
                                    case 3:
                                        hid_kb.key_spec_max = (unsigned int)usage_max;
                                        break;
                                }
                                hid_kb.key_spec_nums  = hid_kb.key_spec_max - hid_kb.key_spec_min + 1;
                                hid_kb.key_spec_bytes = current_bits >> 3;
                                hid_kb.key_spec_shift = (total_bits - current_bits) >> 3;
                            } else {
                                switch ((lu_size >> 2) & 0b11) {
                                    case 0:
                                        hid_kb.key_norm_min = 0;
                                        break;
                                    case 1:
                                        hid_kb.key_norm_min = (unsigned char)usage_min;
                                        break;
                                    case 2:
                                        hid_kb.key_norm_min = (unsigned short)usage_min;
                                        break;
                                    case 3:
                                        hid_kb.key_norm_min = (unsigned int)usage_min;
                                        break;
                                }
                                switch ((lu_size >> 0) & 0b11) {
                                    case 0:
                                        hid_kb.key_norm_max = 0;
                                        break;
                                    case 1:
                                        hid_kb.key_norm_max = (unsigned char)usage_max;
                                        break;
                                    case 2:
                                        hid_kb.key_norm_max = (unsigned short)usage_max;
                                        break;
                                    case 3:
                                        hid_kb.key_norm_max = (unsigned int)usage_max;
                                        break;
                                }
                                hid_kb.key_norm_nums  = hid_kb.key_norm_max - hid_kb.key_norm_min + 1;
                                hid_kb.key_norm_bytes = current_bits >> 3;
                                hid_kb.key_norm_shift = (total_bits - current_bits) >> 3;
                            }
                        }
                    } break;

                    default:
                        break;
                }
                usage_page_item_num = 0;
            } else if (tag_name == HID_TAG_OUTPUT) {
                // input type data
                current_bits = report_size * report_cnt;
                // total_bits += current_bits;
                page_bits += current_bits;
                tag_value = HID_Data_Recovery(&hid[i], tag_size);

                switch (usage_page) {
                    case HID_LED_PAGE: {
                        switch ((lu_size >> 2) & 0b11) {
                            case 0:
                                hid_kb.led_min = 0;
                                break;
                            case 1:
                                hid_kb.led_min = (unsigned char)usage_min;
                                break;
                            case 2:
                                hid_kb.led_min = (unsigned short)usage_min;
                                break;
                            case 3:
                                hid_kb.led_min = (unsigned int)usage_min;
                                break;
                        }
                        switch ((lu_size >> 0) & 0b11) {
                            case 0:
                                hid_kb.led_max = 0;
                                break;
                            case 1:
                                hid_kb.led_max = (unsigned char)usage_max;
                                break;
                            case 2:
                                hid_kb.led_max = (unsigned short)usage_max;
                                break;
                            case 3:
                                hid_kb.led_max = (unsigned int)usage_max;
                                break;
                        }
                        hid_kb.led_nums  = hid_kb.led_max - hid_kb.led_min + 1;
                        hid_kb.led_bytes = page_bits >> 3;
                        // hid_kb.led_shift = (total_bits - page_bits) >> 3;
                    } break;

                    default:
                        break;
                }
                usage_page_item_num = 0;
            }

            i += tag_size + 1;
        }
    }

    return result;
}