
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

#include "ena-interface.h"

static int interface_state = ENA_INTERFACE_STATE_IDLE;

static bool touch_status[TOUCH_PAD_MAX] = {0};
static ena_interface_touch_callback touch_callbacks[TOUCH_PAD_MAX];

void ena_interface_register_touch_callback(int touch_pad, ena_interface_touch_callback callback)
{
    touch_callbacks[touch_pad] = callback;
}

void ena_interface_run(void *pvParameter)
{
    uint16_t touch_value;
    bool touch_status_current[4] = {0};
    while (1)
    {
        for (int i = 0; i < TOUCH_PAD_MAX; i++)
        {

            touch_pad_read_filtered(i, &touch_value);
            touch_status_current[i] = touch_value < TOUCHPAD_TOUCH_THRESHOLD;

            if (!touch_status[i] & touch_status_current[i] && touch_callbacks[i] != NULL)
            {
                (*touch_callbacks[i])();
            }
            touch_status[i] = touch_status_current[i];
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void ena_interface_start(void)
{
    ESP_ERROR_CHECK(touch_pad_init());
    ESP_ERROR_CHECK(touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V));
    ESP_ERROR_CHECK(touch_pad_set_trigger_mode(TOUCH_TRIGGER_BELOW));
    ESP_ERROR_CHECK(touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD));
    ESP_ERROR_CHECK(touch_pad_config(TOUCH_PAD_ESC, TOUCHPAD_TOUCH_THRESHOLD));
    ESP_ERROR_CHECK(touch_pad_config(TOUCH_PAD_OK, TOUCHPAD_TOUCH_THRESHOLD));
    ESP_ERROR_CHECK(touch_pad_config(TOUCH_PAD_UP, TOUCHPAD_TOUCH_THRESHOLD));
    ESP_ERROR_CHECK(touch_pad_config(TOUCH_PAD_DOWN, TOUCHPAD_TOUCH_THRESHOLD));
    xTaskCreate(&ena_interface_run, "ena_interface_run", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
}

int ena_interface_get_state(void)
{
    return interface_state;
}

void ena_interface_set_state(ena_interface_state state)
{
    interface_state = state;
}
