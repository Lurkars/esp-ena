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

#include "mpu6886.h"

#include "interface.h"

#include "m5-input.h"

static float old_input_states[INTERFACE_COMMANDS_SIZE];
static float input_states[INTERFACE_COMMANDS_SIZE];
static int input_command_mapping[INTERFACE_COMMANDS_SIZE];
static bool flipped = false;

void button_input_check(interface_command_t command)
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
    }
    else if (button_level == 1 && input_states[command] > 0)
    {
        input_states[command] = 0;
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
            if (!interface_is_idle() && flipped)
            {
                if (command == INTERFACE_COMMAND_SET)
                {
                    command = INTERFACE_COMMAND_RST;
                }
                else if (command == INTERFACE_COMMAND_RST)
                {
                    command = INTERFACE_COMMAND_SET;
                }
            }
            interface_execute_command(command);
        }
    }
}

void accel_input_up(float axis)
{

    input_states[INTERFACE_COMMAND_UP] = axis;
    if (axis < -0.3 && old_input_states[INTERFACE_COMMAND_UP] > -0.3 && old_input_states[INTERFACE_COMMAND_UP] + axis < -0.5)
    {
        interface_execute_command(INTERFACE_COMMAND_UP);
        old_input_states[INTERFACE_COMMAND_UP] = axis;
    }
    else if (axis > -0.3)
    {
        old_input_states[INTERFACE_COMMAND_UP] = 0;
    }
}

void accel_input_left(float axis)
{

    input_states[INTERFACE_COMMAND_LFT] = axis;
    if (axis < -0.3 && old_input_states[INTERFACE_COMMAND_LFT] > -0.3 && old_input_states[INTERFACE_COMMAND_LFT] + axis < -0.5)
    {
        interface_execute_command(INTERFACE_COMMAND_LFT);
        old_input_states[INTERFACE_COMMAND_LFT] = axis;
    }
    else if (axis > -0.3)
    {
        old_input_states[INTERFACE_COMMAND_LFT] = 0;
    }
}

void accel_input_right(float axis)
{
    input_states[INTERFACE_COMMAND_RHT] = axis;
    if (axis > 0.3 && old_input_states[INTERFACE_COMMAND_RHT] < 0.3 && old_input_states[INTERFACE_COMMAND_RHT] + axis > 0.5)
    {
        interface_execute_command(INTERFACE_COMMAND_RHT);
        old_input_states[INTERFACE_COMMAND_RHT] = axis;
    }
    else if (axis < 0.3)
    {
        old_input_states[INTERFACE_COMMAND_RHT] = 0;
    }
}

void accel_input_down(float axis)
{
    input_states[INTERFACE_COMMAND_DWN] = axis;
    if (axis > 0.3 && old_input_states[INTERFACE_COMMAND_DWN] < 0.3 && old_input_states[INTERFACE_COMMAND_DWN] + axis > 0.5)
    {
        interface_execute_command(INTERFACE_COMMAND_DWN);
        old_input_states[INTERFACE_COMMAND_DWN] = axis;
    }
    else if (axis < 0.3)
    {
        old_input_states[INTERFACE_COMMAND_DWN] = 0;
    }
}

void gyro_input(float axis)
{
    input_states[INTERFACE_COMMAND_MID] = axis;
    if (axis < -50 && old_input_states[INTERFACE_COMMAND_MID] > 50)
    {
        // interface_execute_command(INTERFACE_COMMAND_MID);
        old_input_states[INTERFACE_COMMAND_MID] = axis;
        ESP_LOGI(INTERFACE_LOG, "gyro: %f", axis);
    }
    else if (axis > 50)
    {
        old_input_states[INTERFACE_COMMAND_MID] = axis;
    }
}

void m5_input_task(void *pvParameter)
{
    float ax = 0;
    float ay = 0;
    float az = 0;

    while (1)
    {
        button_input_check(INTERFACE_COMMAND_SET);

        if (!interface_is_idle())
        {
            button_input_check(INTERFACE_COMMAND_RST);
            mpu6886_get_accel_data(&ax, &ay, &az);
            accel_input_up(flipped ? -ax : ax);
            accel_input_left(flipped ? -ay : ay);
            accel_input_right(flipped ? -ay : ay);
            accel_input_down(flipped ? -ax : ax);

            if (ax >= 1)
            {
                flipped = false;
                interface_flipped(flipped);
            }
            else if (ax <= -1)
            {
                flipped = true;
                interface_flipped(flipped);
            }
        }

        vTaskDelay(INTERFACE_INPUT_TICKS_MS / portTICK_PERIOD_MS);
    }
}

void m5_input_start(void)
{
    gpio_config_t io_conf;

    io_conf.pin_bit_mask = (1ULL << BUTTON_RST) | (1ULL << BUTTON_SET);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    input_command_mapping[INTERFACE_COMMAND_RST] = BUTTON_RST;
    input_command_mapping[INTERFACE_COMMAND_SET] = BUTTON_SET;

    mpu6886_start();

    xTaskCreate(&m5_input_task, "m5_input_task", 4096, NULL, 5, NULL);
}