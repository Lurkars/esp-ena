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
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "display.h"
#include "display-gfx.h"

#if defined(CONFIG_ENA_INTERFACE_M5STICKC) || defined(CONFIG_ENA_INTERFACE_M5STICKC_PLUS)
#include "mpu6886.h"
#include "axp192.h"
#endif

#include "interface.h"

static bool runTask = true;
static TaskHandle_t debugTaskHandle = NULL;

void interface_debug_set(void)
{
  runTask = false;
  vTaskDelay(100 / portTICK_PERIOD_MS);
  vTaskSuspend(debugTaskHandle);
  interface_main_start();
}

void interface_debug_rst(void)
{
}

void interface_debug_lft(void)
{
  runTask = false;
  vTaskDelay(100 / portTICK_PERIOD_MS);
  vTaskSuspend(debugTaskHandle);
  interface_data_start();
}

void interface_debug_rht(void)
{
  runTask = false;
  vTaskDelay(100 / portTICK_PERIOD_MS);
  vTaskSuspend(debugTaskHandle);
  interface_info_start();
}

void interface_debug_mid(void)
{
}

void interface_debug_up(void)
{
}

void interface_debug_dwn(void)
{
}

void interface_debug_task(void *pvParameter)
{

#if defined(CONFIG_ENA_INTERFACE_M5STICKC) || defined(CONFIG_ENA_INTERFACE_M5STICKC_PLUS)
  float ax = 0;
  float ay = 0;
  float az = 0;

  float gx = 0;
  float gy = 0;
  float gz = 0;
#endif

  while (1)
  {

    if (!interface_is_idle() && runTask)
    {
#if defined(CONFIG_ENA_INTERFACE_M5STICKC) || defined(CONFIG_ENA_INTERFACE_M5STICKC_PLUS)
      mpu6886_get_accel_data(&ax, &ay, &az);
      mpu6886_get_gyro_data(&gx, &gy, &gz);

      char data_chars[32];
      sprintf(data_chars, "acc x:%3.2f", ax);
      display_text_line(data_chars, 2, false);
      sprintf(data_chars, "acc y:%3.2f", ay);
      display_text_line(data_chars, 3, false);
      sprintf(data_chars, "acc z:%3.2f", az);
      display_text_line(data_chars, 4, false);
      sprintf(data_chars, "gyr x:%3.2f", gx);
      display_text_line(data_chars, 5, false);
      sprintf(data_chars, "gyr y:%3.2f", gy);
      display_text_line(data_chars, 6, false);
      sprintf(data_chars, "gyr z:%3.2f", gz);
      display_text_line(data_chars, 7, false);

      float bat_v = axp192_get_bat_voltage();

      sprintf(data_chars, "Battery: %.2f V", bat_v);
      display_text_line(data_chars, 7, false);

#endif
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void interface_debug_start(void)
{

  interface_register_command_callback(INTERFACE_COMMAND_RST, &interface_debug_rst);
  interface_register_command_callback(INTERFACE_COMMAND_SET, &interface_debug_set);
  interface_register_command_callback(INTERFACE_COMMAND_LFT, &interface_debug_lft);
  interface_register_command_callback(INTERFACE_COMMAND_RHT, &interface_debug_rht);
  interface_register_command_callback(INTERFACE_COMMAND_MID, &interface_debug_mid);
  interface_register_command_callback(INTERFACE_COMMAND_UP, &interface_debug_up);
  interface_register_command_callback(INTERFACE_COMMAND_DWN, &interface_debug_dwn);
  interface_register_command_callback(INTERFACE_COMMAND_RST_LONG, NULL);
  interface_register_command_callback(INTERFACE_COMMAND_SET_LONG, NULL);

  interface_set_display_function(NULL);

  display_menu_headline(interface_get_label_text(&interface_text_headline_debug), true, 0);

  if (debugTaskHandle == NULL)
  {
    xTaskCreate(&interface_debug_task, "interface_debug_task", 4096 * 2, NULL, 5, &debugTaskHandle);
  }
  else
  {
    vTaskResume(debugTaskHandle);
  }

  runTask = true;
}
