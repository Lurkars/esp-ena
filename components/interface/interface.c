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
#include "driver/gpio.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "ssd1306-gfx.h"

#include "interface.h"

static int old_button_states[GPIO_PIN_COUNT];
static int button_states[GPIO_PIN_COUNT];
static interface_button_callback button_callbacks[GPIO_PIN_COUNT];
static interface_display_function current_display_function;

void interface_register_button_callback(int button_gpio, interface_button_callback callback)
{
    button_callbacks[button_gpio] = callback;
}

void interface_set_display_function(interface_display_function display_function)
{
    ssd1306_clear(SSD1306_ADDRESS);
    current_display_function = display_function;
}

void interface_input_check(uint8_t gpio)
{
    button_states[gpio] = gpio_get_level(gpio);

    if (old_button_states[gpio] != 0 && old_button_states[gpio] != 1)
    {
        old_button_states[gpio] = 1;
    }

    if (button_states[gpio] == 0 && button_states[gpio] != old_button_states[gpio] && button_callbacks[gpio] != NULL)
    {
        ssd1306_clear(SSD1306_ADDRESS);
        (*button_callbacks[gpio])();
    }

    old_button_states[gpio] = button_states[gpio];
}

void interface_input_task(void *pvParameter)
{
    while (1)
    {
        interface_input_check(INTERFACE_BUTTON_RST);
        interface_input_check(INTERFACE_BUTTON_SET);
        interface_input_check(INTERFACE_BUTTON_LFT);
        interface_input_check(INTERFACE_BUTTON_RHT);
        interface_input_check(INTERFACE_BUTTON_MID);
        interface_input_check(INTERFACE_BUTTON_UP);
        interface_input_check(INTERFACE_BUTTON_DWN);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void interface_display_task(void *pvParameter)
{
    while (1)
    {
        if (current_display_function != NULL)
        {
            (*current_display_function)();
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void interface_start(void)
{

    gpio_config_t io_conf;

    io_conf.pin_bit_mask = (1ULL << INTERFACE_BUTTON_RST) | (1ULL << INTERFACE_BUTTON_SET) |
                           (1ULL << INTERFACE_BUTTON_MID) | (1ULL << INTERFACE_BUTTON_RHT) |
                           (1ULL << INTERFACE_BUTTON_LFT) | (1ULL << INTERFACE_BUTTON_DWN) |
                           (1ULL << INTERFACE_BUTTON_UP);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    xTaskCreate(&interface_input_task, "interface_input_task", 4096, NULL, 5, NULL);
    xTaskCreate(&interface_display_task, "interface_display_task", 4096, NULL, 5, NULL);

    // init label
    interface_init_label();

    ssd1306_start(SSD1306_ADDRESS);
    ssd1306_clear(SSD1306_ADDRESS);

    for (int i = 0; i < 8; i++)
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_logo[i], 64, i, 32, false);
    }
}
