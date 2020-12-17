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

#include "custom-input.h"

static float input_states[INTERFACE_COMMANDS_SIZE];
static float input_trigger_state[INTERFACE_COMMANDS_SIZE];
static int input_command_mapping[INTERFACE_COMMANDS_SIZE];

void custom_input_check(interface_command_t command)
{
    int button_level = gpio_get_level(input_command_mapping[command]);

    if (button_level == 0)
    {
        input_states[command] = input_states[command] + ((float)INTERFACE_INPUT_TICKS_MS / 1000);

        if (command == INTERFACE_COMMAND_SET && input_states[command] > INTERFACE_LONG_STATE_SECONDS)
        {
            input_states[command] = 0;
            input_states[INTERFACE_COMMAND_SET_LONG] = input_states[INTERFACE_COMMAND_SET_LONG] + 1;
            interface_execute_command(INTERFACE_COMMAND_SET_LONG);
        }
        else if (command == INTERFACE_COMMAND_RST && input_states[command] > INTERFACE_LONG_STATE_SECONDS)
        {
            input_states[command] = 0;
            input_states[INTERFACE_COMMAND_RST_LONG] = input_states[INTERFACE_COMMAND_RST_LONG] + 1;
            interface_execute_command(INTERFACE_COMMAND_RST_LONG);
        }
        else if (input_states[command] > input_trigger_state[command])
        {
            input_trigger_state[command] = input_trigger_state[command] - (input_trigger_state[command] / 8);
            if (input_trigger_state[command] <= ((float)INTERFACE_INPUT_TICKS_MS / 200))
            {
                input_trigger_state[command] = ((float)INTERFACE_INPUT_TICKS_MS / 200);
            }
            input_states[command] = 0;
            interface_execute_command_trigger(command);
        }
    }
    else if (button_level == 1 && input_states[command] > 0)
    {
        input_states[command] = 0;
        input_trigger_state[command] = INTERFACE_LONG_STATE_SECONDS;
        if (command == INTERFACE_COMMAND_SET && input_states[INTERFACE_COMMAND_SET_LONG] > 0)
        {
            input_states[INTERFACE_COMMAND_SET_LONG] = 0;
        }
        else if (command == INTERFACE_COMMAND_RST && input_states[INTERFACE_COMMAND_RST_LONG] > 0)
        {
            input_states[INTERFACE_COMMAND_RST_LONG] = 0;
        }
        else
        {
            interface_execute_command(command);
        }
    }
    else if (button_level == 1)
    {
        input_trigger_state[command] = INTERFACE_LONG_STATE_SECONDS;
    }
}

void custom_input_task(void *pvParameter)
{
    while (1)
    {
        custom_input_check(INTERFACE_COMMAND_SET);
        if (!interface_is_idle())
        {
            custom_input_check(INTERFACE_COMMAND_RST);
            custom_input_check(INTERFACE_COMMAND_MID);
            custom_input_check(INTERFACE_COMMAND_RHT);
            custom_input_check(INTERFACE_COMMAND_LFT);
            custom_input_check(INTERFACE_COMMAND_DWN);
            custom_input_check(INTERFACE_COMMAND_UP);
        }
        vTaskDelay(INTERFACE_INPUT_TICKS_MS / portTICK_PERIOD_MS);
    }
}

void interface_input_start(void)
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

    input_command_mapping[INTERFACE_COMMAND_RST] = BUTTON_RST;
    input_command_mapping[INTERFACE_COMMAND_SET] = BUTTON_SET;
    input_command_mapping[INTERFACE_COMMAND_MID] = BUTTON_MID;
    input_command_mapping[INTERFACE_COMMAND_RHT] = BUTTON_RHT;
    input_command_mapping[INTERFACE_COMMAND_LFT] = BUTTON_LFT;
    input_command_mapping[INTERFACE_COMMAND_DWN] = BUTTON_DWN;
    input_command_mapping[INTERFACE_COMMAND_UP] = BUTTON_UP;

    for (int i = 0; i < INTERFACE_COMMANDS_SIZE; i++)
    {
        input_trigger_state[i] = INTERFACE_LONG_STATE_SECONDS;
    }

    xTaskCreate(&custom_input_task, "custom_input_task", 4096, NULL, 5, NULL);
}