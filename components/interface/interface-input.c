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

#include "interface.h"

static interface_input_callback current_input_callback_rst;
static interface_input_callback current_input_callback_set;
static char current_text[255];
static uint8_t current_cursor;
static char current_char_set[32];
static uint8_t current_char_index;
static uint8_t current_limit;
static uint8_t current_max_index;

static char char_set_uppercase[32];
static char char_set_lowercase[32];
static char char_set_numeric[32];
static char char_set_special1[32];
static char char_set_special_uppercasecase[32];
static char char_set_special_lowercase[32];

void interface_input_set_char_set(void)
{

    char *ret;
    char cur_char = current_text[current_cursor];

    if ((ret = strchr(char_set_lowercase, cur_char)) != NULL)
    {
        strcpy(current_char_set, char_set_lowercase);
        current_char_index = strlen(current_char_set) - strlen(ret);
    }
    else if ((ret = strchr(char_set_numeric, cur_char)) != NULL)
    {
        strcpy(current_char_set, char_set_numeric);
        current_char_index = strlen(current_char_set) - strlen(ret);
    }
    else if ((ret = strchr(char_set_special1, cur_char)) != NULL)
    {
        strcpy(current_char_set, char_set_special1);
        current_char_index = strlen(current_char_set) - strlen(ret);
    }
    else if ((ret = strchr(char_set_special_uppercasecase, cur_char)) != NULL)
    {
        strcpy(current_char_set, char_set_special_uppercasecase);
        current_char_index = strlen(current_char_set) - strlen(ret);
    }
    else if ((ret = strchr(char_set_special_lowercase, cur_char)) != NULL)
    {
        strcpy(current_char_set, char_set_special_lowercase);
        current_char_index = strlen(current_char_set) - strlen(ret);
    }
    else if ((ret = strchr(char_set_uppercase, cur_char)) != NULL)
    {
        strcpy(current_char_set, char_set_uppercase);
        current_char_index = strlen(current_char_set) - strlen(ret);
    }
    printf("current_char_set: %d %s\n", strlen(current_char_set), current_char_set);
}

void interface_input_set(void)
{
    if (current_input_callback_rst != NULL)
    {
        (*current_input_callback_rst)(current_text, current_cursor);
    }
}

void interface_input_rst(void)
{
    if (current_input_callback_set != NULL)
    {
        (*current_input_callback_set)(current_text, current_cursor);
    }
}

void interface_input_lft(void)
{
    if (current_cursor > 0)
    {
        current_cursor--;
        interface_input_set_char_set();
    }
}

void interface_input_rht(void)
{
    if (current_cursor < current_limit)
    {
        current_cursor++;
        if (current_cursor > current_max_index)
        {
            current_max_index = current_cursor;
            strcpy(current_char_set, char_set_uppercase);

            current_char_index = 0;
            current_text[current_cursor] = current_char_set[current_char_index];
        }
        else
        {
            interface_input_set_char_set();
        }
    }
}

void interface_input_mid(void)
{
    if (current_char_set[0] == char_set_uppercase[0])
    {
        strcpy(current_char_set, char_set_lowercase);
    }
    else if (current_char_set[0] == char_set_lowercase[0])
    {
        strcpy(current_char_set, char_set_numeric);
    }
    else if (current_char_set[0] == char_set_numeric[0])
    {
        strcpy(current_char_set, char_set_special1);
    }
    else if (current_char_set[0] == char_set_special1[0])
    {
        strcpy(current_char_set, char_set_special_uppercasecase);
    }
    else if (current_char_set[0] == char_set_special_uppercasecase[0])
    {
        strcpy(current_char_set, char_set_special_lowercase);
    }
    else if (current_char_set[0] == char_set_special_lowercase[0])
    {
        strcpy(current_char_set, char_set_uppercase);
    }
    current_char_index = 0;
    current_text[current_cursor] = current_char_set[current_char_index];
    printf("current_char_set: %d %s\n", strlen(current_char_set), current_char_set);
}
void interface_input_up(void)
{
    if (current_char_index == 0)
    {
        current_char_index = strlen(current_char_set) - 1;
    }
    else
    {
        current_char_index--;
    }

    current_text[current_cursor] = current_char_set[current_char_index];
}

void interface_input_dwn(void)
{
    if (current_char_index == strlen(current_char_set) - 1)
    {
        current_char_index = 0;
    }
    else
    {
        current_char_index++;
    }

    current_text[current_cursor] = current_char_set[current_char_index];
}

void interface_input_display(void)
{

    // buttons
    ssd1306_set_button(SSD1306_ADDRESS, interface_get_label_text(&interface_text_button_cancel), true, false);
    ssd1306_set_button(SSD1306_ADDRESS, interface_get_label_text(&interface_text_button_ok), false, true);

    size_t start = 0;
    uint8_t display_cursor = current_cursor + 1;
    if (current_cursor > 13)
    {
        start = current_cursor - 13;
        display_cursor = 14;
    }

    // arrow
    if (current_cursor > 0)
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_left, 8, 2, 0, false);
    }
    else
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_clear, 8, 2, 0, false);
    }
    // bounday
    ssd1306_text_line_column(SSD1306_ADDRESS, "______________", 2, 1, false);
    // arrow
    if (current_cursor < current_limit)
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_right, 8, 2, 15 * 8, false);
    }
    else
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_clear, 8, 2, 15 * 8, false);
    }
    // text
    size_t text_length = strlen(current_text);
    if (strlen(current_text) > 14)
    {
        text_length = 14;
    }
    size_t length = text_length * 8;
    uint8_t *textdata = calloc(length, sizeof(uint8_t));
    for (uint8_t i = 0; i < text_length; i++)
    {
        memcpy(&textdata[i * 8], ssd1306_gfx_font[(uint8_t)current_text[i + start] - 32], 8);
    }

    ssd1306_data(SSD1306_ADDRESS, textdata, length, 2, 8, true);
    free(textdata);

    // clear
    ssd1306_clear_line(SSD1306_ADDRESS, 0, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 1, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 3, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 4, false);

    uint8_t current_char = (uint8_t)current_char_set[current_char_index] - 32;
    uint8_t prev_char = (uint8_t)current_char_set[current_char_index - 1] - 32;
    uint8_t next_char = (uint8_t)current_char_set[current_char_index + 1] - 32;

    if (current_char_index == 0)
    {
        prev_char = (uint8_t)current_char_set[strlen(current_char_set) - 1] - 32;
    }

    if (current_char_index == strlen(current_char_set) - 1)
    {
        next_char = (uint8_t)current_char_set[0] - 32;
    }

    // arrow
    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_up, 8, 0, display_cursor * 8, false);
    // upper char
    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_font[prev_char], 8, 1, display_cursor * 8, false);
    // sel char
    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_font[current_char], 8, 2, display_cursor * 8, false);
    // lower char
    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_font[next_char], 8, 3, display_cursor * 8, false);
    // arrow
    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_down, 8, 4, display_cursor * 8, false);
}

void interface_input_set_text(char *text)
{
    ssd1306_utf8_to_ascii(text, current_text);
    current_cursor = strlen(current_text) - 1;
    current_max_index = current_cursor;
    interface_input_set_char_set();
}

void interface_input(interface_input_callback callback_rst, interface_input_callback callback_set, uint8_t limit)
{
    current_input_callback_rst = callback_rst;
    current_input_callback_set = callback_set;
    current_cursor = 0;
    current_limit = limit - 1;

    ssd1306_utf8_to_ascii("ABCDEFGHIJKLMNOPQRSTUVWXYZ", char_set_uppercase);
    ssd1306_utf8_to_ascii("abcdefghijklmnopqrstuvwxyz", char_set_lowercase);
    ssd1306_utf8_to_ascii(" !\"#$%&'()*+,-,&:;<=>@[\\]^_´`{}", char_set_special1);
    ssd1306_utf8_to_ascii("0123456789", char_set_numeric);
    ssd1306_utf8_to_ascii("ÄÖÜ", char_set_special_uppercasecase);
    ssd1306_utf8_to_ascii("äöü", char_set_special_lowercase);

    strcpy(current_char_set, char_set_uppercase);

    printf("char_set_uppercase: %d %s\n", strlen(char_set_uppercase), char_set_uppercase);
    printf("char_set_lowercase: %d %s\n", strlen(char_set_lowercase), char_set_lowercase);
    printf("char_set_numeric: %d %s\n", strlen(char_set_numeric), char_set_numeric);
    printf("char_set_special1: %d %s\n", strlen(char_set_special1), char_set_special1);
    printf("char_set_special_uppercasecase: %d %s\n", strlen(char_set_special_uppercasecase), char_set_special_uppercasecase);
    printf("char_set_special_lowercase: %d %s\n", strlen(char_set_special_lowercase), char_set_special_lowercase);

    current_char_index = 0;
    current_max_index = 0;
    if (current_limit == 0)
    {
        current_limit = 255;
    }

    current_text[current_cursor] = current_char_set[current_char_index];

    interface_register_button_callback(INTERFACE_BUTTON_RST, &interface_input_rst);
    interface_register_button_callback(INTERFACE_BUTTON_SET, &interface_input_set);
    interface_register_button_callback(INTERFACE_BUTTON_LFT, &interface_input_lft);
    interface_register_button_callback(INTERFACE_BUTTON_RHT, &interface_input_rht);
    interface_register_button_callback(INTERFACE_BUTTON_MID, &interface_input_mid);
    interface_register_button_callback(INTERFACE_BUTTON_UP, &interface_input_up);
    interface_register_button_callback(INTERFACE_BUTTON_DWN, &interface_input_dwn);

    interface_set_display_function(&interface_input_display);
    ESP_LOGD(INTERFACE_LOG, "start input interface");
}
