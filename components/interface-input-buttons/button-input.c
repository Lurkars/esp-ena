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

#include "interface.h"

#include "button-input.h"

static float button_states[INTERFACE_COMMANDS_SIZE];
static int button_command_mapping[INTERFACE_COMMANDS_SIZE];

void button_input_check(interface_command_t command)
{
    int button_level = gpio_get_level(button_command_mapping[command]);

    if (button_level == 0)
    {
        button_states[command] = button_states[command] + ((float)INTERFACE_INPUT_TICKS_MS / 1000);

        if (command == INTERFACE_COMMAND_SET && button_states[command] > INTERFACE_LONG_STATE_SECONDS)
        {
            button_states[command] = 0;
            button_states[INTERFACE_COMMAND_SET_LONG] = button_states[INTERFACE_COMMAND_SET_LONG] + 1;
            interface_execute_command(INTERFACE_COMMAND_SET_LONG);
        }
        else if (command == INTERFACE_COMMAND_RST && button_states[command] > INTERFACE_LONG_STATE_SECONDS)
        {
            button_states[command] = 0;
            button_states[INTERFACE_COMMAND_RST_LONG] = button_states[INTERFACE_COMMAND_RST_LONG] + 1;
            interface_execute_command(INTERFACE_COMMAND_RST_LONG);
        }
    }
    else if (button_level == 1 && button_states[command] > 0)
    {
        button_states[command] = 0;

        if (command == INTERFACE_COMMAND_SET && button_states[INTERFACE_COMMAND_SET_LONG] > 0)
        {
            button_states[INTERFACE_COMMAND_SET_LONG] = 0;
        }
        else if (command == INTERFACE_COMMAND_RST && button_states[INTERFACE_COMMAND_RST_LONG] > 0)
        {
            button_states[INTERFACE_COMMAND_RST_LONG] = 0;
        }
        else
        {
            interface_execute_command(command);
        }
    }
}

void button_input_task(void *pvParameter)
{
    while (1)
    {
        button_input_check(INTERFACE_COMMAND_SET);
        if (!interface_is_idle())
        {
            button_input_check(INTERFACE_COMMAND_RST);
            button_input_check(INTERFACE_COMMAND_MID);
            button_input_check(INTERFACE_COMMAND_RHT);
            button_input_check(INTERFACE_COMMAND_LFT);
            button_input_check(INTERFACE_COMMAND_DWN);
            button_input_check(INTERFACE_COMMAND_UP);
        }
        vTaskDelay(INTERFACE_INPUT_TICKS_MS / portTICK_PERIOD_MS);
    }
}

void button_input_start(void)
{
    gpio_config_t io_conf;

    io_conf.pin_bit_mask = (1ULL << BUTTON_RST) | (1ULL << BUTTON_SET) |
                           (1ULL << BUTTON_MID) | (1ULL << BUTTON_RHT) |
                           (1ULL << BUTTON_LFT) | (1ULL << BUTTON_DWN) |
                           (1ULL << BUTTON_UP);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    button_command_mapping[INTERFACE_COMMAND_RST] = BUTTON_RST;
    button_command_mapping[INTERFACE_COMMAND_SET] = BUTTON_SET;
    button_command_mapping[INTERFACE_COMMAND_MID] = BUTTON_MID;
    button_command_mapping[INTERFACE_COMMAND_RHT] = BUTTON_RHT;
    button_command_mapping[INTERFACE_COMMAND_LFT] = BUTTON_LFT;
    button_command_mapping[INTERFACE_COMMAND_DWN] = BUTTON_DWN;
    button_command_mapping[INTERFACE_COMMAND_UP] = BUTTON_UP;

    xTaskCreate(&button_input_task, "button_input_task", 4096, NULL, 5, NULL);
}