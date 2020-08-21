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

#include "ssd1306.h"
#include "ssd1306-gfx.h"

#include "ena-storage.h"

#include "interface.h"

typedef enum
{
    INTERFACE_SETTINGS_LOCALE = 0,
    INTERFACE_SETTINGS_TIMEZONE,
} interface_settings_state_t;

static int current_interface_settings_state;

void interface_settings_set(void)
{
    interface_main_start();
}

void interface_settings_rst(void)
{
}

void interface_settings_lft(void)
{
    interface_data_start();
}

void interface_settings_rht(void)
{
    interface_datetime_start();
}

void interface_settings_mid(void)
{

    current_interface_settings_state++;
    if (current_interface_settings_state > INTERFACE_SETTINGS_TIMEZONE)
    {
        current_interface_settings_state = INTERFACE_SETTINGS_LOCALE;
    }

    ssd1306_clear_line(SSD1306_ADDRESS, 2, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 3, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 4, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 5, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 6, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 7, false);
}

void interface_settings_up(void)
{
    if (current_interface_settings_state == INTERFACE_SETTINGS_LOCALE)
    {
        if (interface_get_locale() == 0)
        {
            interface_set_locale(INTERFACE_NUM_LOCALE - 1);
        }
        else
        {
            interface_set_locale(interface_get_locale() - 1);
        }
    }
}

void interface_settings_dwn(void)
{

    if (current_interface_settings_state == INTERFACE_SETTINGS_LOCALE)
    {
        if (interface_get_locale() + 1 == INTERFACE_NUM_LOCALE)
        {
            interface_set_locale(0);
        }
        else
        {
            interface_set_locale(interface_get_locale() + 1);
        }
    }
}

void interface_settings_display(void)
{
    ssd1306_menu_headline(SSD1306_ADDRESS, interface_get_label_text(&interface_text_headline_settings), true, 0);

    ssd1306_text_line_column(SSD1306_ADDRESS, interface_get_label_text(&interface_text_settings_locale), 3, 1, false);

    ssd1306_text_line_column(SSD1306_ADDRESS, interface_get_label_text(&interface_text_settings_timezone), 6, 1, false);

    if (current_interface_settings_state == INTERFACE_SETTINGS_LOCALE)
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_up, 8, 2, 12 * 8 + 4, false);
        ssd1306_text_line_column(SSD1306_ADDRESS,
                                 interface_get_label_text(&interface_text_settings_locales[interface_get_locale()]), 3, 12, true);
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_down, 8, 4, 12 * 8 + 4, false);
    }
    else
    {
        ssd1306_text_line_column(SSD1306_ADDRESS,
                                 interface_get_label_text(&interface_text_settings_locales[interface_get_locale()]), 3, 12, true);
    }

    if (current_interface_settings_state == INTERFACE_SETTINGS_TIMEZONE)
    {

        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_up, 8, 5, 12 * 8 + 4, false);
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_down, 8, 7, 12 * 8 + 4, false);
    }
}

void interface_settings_start(void)
{
    current_interface_settings_state = INTERFACE_SETTINGS_LOCALE;

    interface_register_button_callback(INTERFACE_BUTTON_RST, &interface_settings_rst);
    interface_register_button_callback(INTERFACE_BUTTON_SET, &interface_settings_set);
    interface_register_button_callback(INTERFACE_BUTTON_LFT, &interface_settings_lft);
    interface_register_button_callback(INTERFACE_BUTTON_RHT, &interface_settings_rht);
    interface_register_button_callback(INTERFACE_BUTTON_MID, &interface_settings_mid);
    interface_register_button_callback(INTERFACE_BUTTON_UP, &interface_settings_up);
    interface_register_button_callback(INTERFACE_BUTTON_DWN, &interface_settings_dwn);
    interface_set_display_function(&interface_settings_display);

    ESP_LOGD(INTERFACE_LOG, "start settings interface");
}
