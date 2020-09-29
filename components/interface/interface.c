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

#include "display.h"
#include "display-gfx.h"

#include "interface.h"

static interface_command_callback command_callbacks[INTERFACE_COMMANDS_SIZE];
static interface_display_function current_display_function;

void interface_register_command_callback(interface_command_t command, interface_command_callback callback)
{
    command_callbacks[command] = callback;
}

void interface_set_display_function(interface_display_function display_function)
{
    display_clear();
    current_display_function = display_function;
}

void interface_execute_command(interface_command_t command)
{
    if (command_callbacks[command] != NULL)
    {
        display_clear();
        (*command_callbacks[command])();
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
    xTaskCreate(&interface_display_task, "interface_display_task", 4096, NULL, 5, NULL);

    // init label
    interface_init_label();

    display_start();
    display_clear();

    for (int i = 0; i < 8; i++)
    {
        display_data(display_gfx_logo[i], 64, i, 32, false);
    }
}
