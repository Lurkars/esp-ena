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

#include "lsm9ds1.h"

#include "interface.h"

#include "ttgo-input.h"

void ttgo_input_task(void *pvParameter)
{
    float ax = 0;
    float ay = 0;
    float az = 0;

    while (1)
    {
        lsm9ds1_get_accel_data(&ax, &ay, &az);
       //  ESP_LOGI(INTERFACE_LOG, "ax: %f ay:%f az:%f", ax, ay, az);
        vTaskDelay(INTERFACE_INPUT_TICKS_MS / portTICK_PERIOD_MS);
    }
}

void ttgo_input_start(void)
{
    lsm9ds1_start();
    xTaskCreate(&ttgo_input_task, "ttgo_input_task", 4096, NULL, 5, NULL);
}