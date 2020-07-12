/**
 * Interface for different operations
 * 
 */
#include "driver/touch_pad.h"

#ifndef _ena_INTERFACE_H_
#define _ena_INTERFACE_H_

#define ENA_INTERFACE_LOG "ESP-ENA-interface" // TAG for Logging

#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
#define TOUCHPAD_TOUCH_THRESHOLD (600)

#define TOUCH_PAD_ESC (TOUCH_PAD_NUM0)
#define TOUCH_PAD_OK (TOUCH_PAD_NUM6)
#define TOUCH_PAD_UP (TOUCH_PAD_NUM4)
#define TOUCH_PAD_DOWN (TOUCH_PAD_NUM3)

typedef enum
{
    ENA_INTERFACE_STATE_IDLE = 0,
    ENA_INTERFACE_STATE_MENU,
    ENA_INTERFACE_STATE_SET_YEAR,
    ENA_INTERFACE_STATE_SET_MONTH,
    ENA_INTERFACE_STATE_SET_DAY,
    ENA_INTERFACE_STATE_SET_HOUR,
    ENA_INTERFACE_STATE_SET_MINUTE,
    ENA_INTERFACE_STATE_SET_SECONDS,
    ENA_INTERFACE_STATE_STATUS,
} ena_inerface_state;

typedef void (*ena_interface_touch_callback)(void);

void ena_interface_register_touch_callback(int touch_pad, ena_interface_touch_callback callback);

int ena_interface_get_state(void);

void ena_interface_set_state(ena_inerface_state state);

void ena_interface_start(void);

#endif