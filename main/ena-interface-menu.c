#include <stdio.h>

#include "esp_log.h"
#include "ena-interface.h"
#include "ena-interface-datetime.h"

#include "ena-interface-menu.h"

static int interface_menu_state = ENA_INTERFACE_MENU_STATE_IDLE;

void ena_interface_menu_ok(void)
{
    if (interface_menu_state == ENA_INTERFACE_MENU_STATE_SELECT_TIME) {
        ena_interface_datetime_start();
    }
}

void ena_interface_menu_up(void)
{
    interface_menu_state--;
    if (interface_menu_state < 0)
    {
        interface_menu_state = sizeof(interface_menu_state) - 1;
    }
    ESP_LOGD(ENA_INTERFACE_LOG, "menu up to %d", interface_menu_state);
}

void ena_interface_menu_down(void)
{
    interface_menu_state++;
    if (interface_menu_state == sizeof(interface_menu_state))
    {
        interface_menu_state = 0;
    }
    ESP_LOGD(ENA_INTERFACE_LOG, "menu down to %d", interface_menu_state);
}

void ena_interface_menu_start(void)
{
    ena_interface_set_state(ENA_INTERFACE_STATE_MENU);
    ena_interface_register_touch_callback(TOUCH_PAD_ESC, NULL);
    ena_interface_register_touch_callback(TOUCH_PAD_OK, &ena_interface_menu_ok);
    ena_interface_register_touch_callback(TOUCH_PAD_UP, &ena_interface_menu_up);
    ena_interface_register_touch_callback(TOUCH_PAD_DOWN, &ena_interface_menu_down);

    ESP_LOGD(ENA_INTERFACE_LOG, "start menu interface");
}

int ena_interface_menu_get_state(void)
{
    return interface_menu_state;
}