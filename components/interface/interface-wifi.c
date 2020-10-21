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
#include "wifi-controller.h"

#include "interface.h"

#define APS_TO_DISPLAY 3

wifi_ap_record_t ap_info[10];
uint16_t ap_count = 0;
int ap_index = 0;
int ap_selected = 0;

static wifi_config_t current_wifi_config;

void interface_wifi_input_rst(char *text, uint8_t cursor)
{
    interface_wifi_start();
}

void interface_wifi_input_set(char *text, uint8_t cursor)
{
    memcpy(current_wifi_config.sta.password, text, cursor + 1);

    ESP_LOGD(INTERFACE_LOG, "ssid: '%s' password '%s'", current_wifi_config.sta.ssid, current_wifi_config.sta.password);

    if (wifi_controller_connect(current_wifi_config) == ESP_OK)
    {
        interface_main_start();
    }
    else
    {
        // what happens here?
        interface_wifi_start();
    }
}

void interface_wifi_set(void)
{
    interface_main_start();
}

void interface_wifi_lft(void)
{
    interface_datetime_start();
}

void interface_wifi_rht(void)
{
    interface_data_start();
}
void interface_wifi_mid(void)
{
    memset(&current_wifi_config, 0, sizeof(wifi_config_t));
    memcpy(current_wifi_config.sta.ssid, ap_info[ap_selected].ssid, strlen((char *)ap_info[ap_selected].ssid));
    interface_input(&interface_wifi_input_rst, &interface_wifi_input_set, 64);
    interface_input_set_text("muffimuffi");
}
void interface_wifi_up(void)
{
    ap_selected--;
    if (ap_selected < 0)
    {
        ap_selected = ap_count - 1;
        if (ap_count - APS_TO_DISPLAY < 0)
        {
            ap_index = 0;
        }
        else
        {
            ap_index = ap_count - APS_TO_DISPLAY;
        }
    }
    else if (ap_selected < ap_index)
    {
        ap_index--;
    }
}

void interface_wifi_dwn(void)
{
    ap_selected++;
    if (ap_selected >= ap_count)
    {
        ap_selected = 0;
        ap_index = 0;
    }
    else if (ap_selected >= (ap_index + APS_TO_DISPLAY))
    {
        ap_index++;
    }
}

void interface_wifi_display(void)
{
    display_menu_headline( interface_get_label_text(&interface_text_headline_wifi), true, 0);

    if (ap_count > 0)
    {
        display_clear_line( 2, false);
        display_clear_line( 4, false);
        display_clear_line( 6, false);
        for (int i = 0; i < 3; i++)
        {
            int index = i + ap_index;
            if (index < ap_count)
            {
                if (index == ap_selected)
                {
                    display_data( display_gfx_arrow_right, 8, i * 2 + 2, 8, false);
                }

                if (sizeof(ap_info[i].ssid) > 0)
                {
                    display_text_line_column( (char *)ap_info[index].ssid, i * 2 + 2, 2, false);
                }
                else
                {
                    display_text_line_column( " / ", i * 2 + 2, 2, false);
                }

                if (ap_info[index].rssi >= -67)
                {
                    display_data( display_gfx_wifi, 8, i * 2 + 2, 112, false);
                }
                else if (ap_info[index].rssi >= -80)
                {
                    display_data( display_gfx_wifi_low, 8, i * 2 + 2, 112, false);
                }
                else if (ap_info[index].rssi >= -90)
                {
                    display_data( display_gfx_wifi_lowest, 8, i * 2 + 2, 112, false);
                }
            }
        }
    }
    else
    {
        display_text_line_column( interface_get_label_text(&interface_text_wifi_scanning), 4, 1, false);
    }
}

void interface_wifi_start(void)
{
    interface_register_command_callback(INTERFACE_COMMAND_SET, &interface_wifi_set);
    interface_register_command_callback(INTERFACE_COMMAND_LFT, &interface_wifi_lft);
    interface_register_command_callback(INTERFACE_COMMAND_RHT, &interface_wifi_rht);
    interface_register_command_callback(INTERFACE_COMMAND_MID, &interface_wifi_mid);
    interface_register_command_callback(INTERFACE_COMMAND_UP, &interface_wifi_up);
    interface_register_command_callback(INTERFACE_COMMAND_DWN, &interface_wifi_dwn);
    interface_register_command_callback(INTERFACE_COMMAND_RST, NULL);

    interface_set_display_function(&interface_wifi_display);

    ESP_LOGD(INTERFACE_LOG, "start wifi interface and scanning");

    memset(ap_info, 0, sizeof(ap_info));
    ap_count = 0;
    ap_index = 0;
    ap_selected = 0;

    wifi_controller_scan(ap_info, &ap_count);
}
