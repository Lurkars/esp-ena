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

static int old_button_states[INTERFACE_COMMANDS_SIZE];
static int button_states[INTERFACE_COMMANDS_SIZE];
static int button_command_mapping[INTERFACE_COMMANDS_SIZE];

void button_input_check(interface_command_t command)
{
    button_states[command] = gpio_get_level(button_command_mapping[command]);

    if (old_button_states[command] != 0 && old_button_states[command] != 1)
    {
        old_button_states[command] = 1;
    }

    if (button_states[command] == 0 && button_states[command] != old_button_states[command])
    {
        interface_execute_command(command);
    }

    old_button_states[command] = button_states[command];
}

void button_input_task(void *pvParameter)
{
    while (1)
    {
        button_input_check(INTERFACE_COMMAND_RST);
        button_input_check(INTERFACE_COMMAND_SET);
        button_input_check(INTERFACE_COMMAND_MID);
        button_input_check(INTERFACE_COMMAND_RHT);
        button_input_check(INTERFACE_COMMAND_LFT);
        button_input_check(INTERFACE_COMMAND_DWN);
        button_input_check(INTERFACE_COMMAND_UP);
        vTaskDelay(20 / portTICK_PERIOD_MS);
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