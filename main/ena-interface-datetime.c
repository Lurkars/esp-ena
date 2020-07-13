#include <stdio.h>

#include "esp_log.h"
#include "ena-interface.h"
#include "ena-interface-menu.h"

#include "ena-interface-datetime.h"

static int interface_datetime_state = ENA_INTERFACE_DATETIME_STATE_YEAR;

void ena_interface_datetime_esc(void)
{
    ena_interface_menu_start();
}

void ena_interface_datetime_start(void)
{
    ena_interface_set_state(ENA_INTERFACE_STATE_SET_YEAR);
    ena_interface_register_touch_callback(TOUCH_PAD_ESC, &ena_interface_datetime_esc);
    ESP_LOGD(ENA_INTERFACE_LOG, "start datetime interface");
}

int ena_interface_datetime_state(void)
{
    return interface_datetime_state;
}