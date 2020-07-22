// Copyright 2020 Lukas Haubaum
//
// Licensed under the GNU Affero General Public License, Version 3;
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     https://www.gnu.org/licenses/agpl-3.0.html
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

#include "ena-interface.h"

static int interface_state = ENA_INTERFACE_STATE_IDLE;

static int touch_mapping[TOUCH_PAD_COUNT] = {0};
static bool touch_status[TOUCH_PAD_COUNT] = {0};
static ena_interface_touch_callback touch_callbacks[TOUCH_PAD_MAX];

void ena_interface_register_touch_callback(int touch_pad, ena_interface_touch_callback callback)
{
    touch_callbacks[touch_pad] = callback;
}

void ena_interface_run(void *pvParameter)
{
    static uint16_t touch_value;
    static uint16_t touch_thresh;
    static bool touch_status_current[TOUCH_PAD_MAX] = {0};
    while (1)
    {
        for (int i = 0; i < TOUCH_PAD_COUNT; i++)
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(touch_pad_read_filtered(touch_mapping[i], &touch_value));
            ESP_ERROR_CHECK_WITHOUT_ABORT(touch_pad_get_thresh(touch_mapping[i], &touch_thresh));
            touch_status_current[i] = touch_value < touch_thresh;

            if (!touch_status[i] & touch_status_current[i])
            {
                ESP_LOGD(ENA_INTERFACE_LOG, "touch %u at %d (thresh %u)", touch_value, touch_mapping[i], touch_thresh);
                if (touch_callbacks[touch_mapping[i]] != NULL)
                {
                    (*touch_callbacks[touch_mapping[i]])();
                }
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

    touch_mapping[0] = TOUCH_PAD_ESC;
    touch_mapping[1] = TOUCH_PAD_OK;
    touch_mapping[2] = TOUCH_PAD_UP;
    touch_mapping[3] = TOUCH_PAD_DOWN;

    for (int i = 0; i < TOUCH_PAD_COUNT; i++)
    {
        ESP_ERROR_CHECK(touch_pad_config(touch_mapping[i], 0));
    }

    ESP_ERROR_CHECK(touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD));
    uint16_t touch_value;
    for (int i = 0; i < TOUCH_PAD_COUNT; i++)
    {
        ESP_ERROR_CHECK(touch_pad_read_filtered(touch_mapping[i], &touch_value));
        ESP_ERROR_CHECK(touch_pad_set_thresh(touch_mapping[i], touch_value * 2 / 3));
        ESP_LOGD(ENA_INTERFACE_LOG, "calibrate %u at %u (thresh %u)", touch_mapping[i], touch_value, (touch_value * 2 / 3));
    }

    xTaskCreate(&ena_interface_run, "ena_interface_run", 4096, NULL, 5, NULL);
}

int ena_interface_get_state(void)
{
    return interface_state;
}

void ena_interface_set_state(ena_interface_state state)
{
    interface_state = state;
}
