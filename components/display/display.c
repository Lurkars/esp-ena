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

#include "display.h"
#include "display-gfx.h"

static uint16_t current_color = WHITE;

uint8_t display_utf8_to_ascii_char(uint8_t ascii)
{
    static uint8_t c1;
    if (ascii < 128) // Standard ASCII-set 0..0x7F handling
    {
        c1 = 0;
        return (ascii);
    }

    // get previous input
    uint8_t last = c1; // get last char
    c1 = ascii;        // remember actual character

    switch (last) // conversion depending on first UTF8-character
    {
    case 0xC2:
        return (ascii);
        break;
    case 0xC3:
        return (ascii | 0xC0);
        break;
    case 0x82:
        if (ascii == 0xAC)
            return (0x80); // special case Euro-symbol
    }

    return (0); // otherwise: return zero, if character has to be ignored
}

void display_utf8_to_ascii(char *input, char *output)
{
    strcpy(output, input);
    int k = 0;
    char c;
    for (int i = 0; i < strlen(output); i++)
    {
        c = display_utf8_to_ascii_char(output[i]);
        if (c != 0)
            output[k++] = c;
    }
    output[k] = 0;
}

uint8_t *display_text_to_data(char *text, size_t text_length, size_t *length)
{
    char target_text[strlen(text)];
    display_utf8_to_ascii(text, target_text);

    uint8_t font_width = sizeof(display_gfx_font[0]);

    *length = text_length * font_width;

    uint8_t *data = calloc(*length, sizeof(uint8_t));
    for (uint8_t i = 0; i < text_length; i++)
    {
        memcpy(&data[i * font_width], display_gfx_font[(uint8_t)target_text[i] - 32], font_width);
    }

    return data;
}

void display_chars(char *text, size_t length, uint8_t line, uint8_t offset, bool invert)
{
    if (length > 0)
    {
        uint8_t font_width = sizeof(display_gfx_font[0]);
        size_t res_length = 0;
        uint8_t *textdata = display_text_to_data(text, length, &res_length);
        display_data(textdata, res_length, line, offset * font_width, invert);
        free(textdata);
    }
}

void display_text_line_column(char *text, uint8_t line, uint8_t offset, bool invert)
{
    display_chars(text, strlen(text), line, offset, invert);
}

void display_text_line(char *text, uint8_t line, bool invert)
{
    display_text_line_column(text, line, 0, invert);
}

void display_text_input(char *text, uint8_t position)
{
    size_t start = 0;
    if (position > 13)
    {
        position = 13;
        start = position - 13;
    }
    uint8_t cur_char = (uint8_t)text[start + position] - 32;

    // arrow
    display_data(display_gfx_arrow_left, 8, 2, 0, false);
    // bounday
    display_text_line_column("______________", 2, 1, false);
    // arrow
    display_data(display_gfx_arrow_right, 8, 2, 15 * 8, false);
    // text
    size_t text_length = strlen(text);
    if (strlen(text) > 14)
    {
        text_length = 14;
    }
    size_t length = 0;
    uint8_t *textdata = display_text_to_data(text, text_length, &length);
    display_data(textdata, length, 2, 8, true);
    free(textdata);
    // arrow
    display_data(display_gfx_arrow_up, 8, 0, (position + 1) * 8, false);
    // upper char
    display_data(display_gfx_font[cur_char - 1], 8, 1, (position + 1) * 8, false);
    // sel char
    display_data(display_gfx_font[cur_char], 8, 2, (position + 1) * 8, false);
    // lower char
    display_data(display_gfx_font[cur_char + 1], 8, 3, (position + 1) * 8, false);
    // arrow
    display_data(display_gfx_arrow_down, 8, 4, (position + 1) * 8, false);
}

void display_set_button(char *text, bool selected, bool primary)
{
    uint8_t start = 0;
    if (primary)
    {
        start = 64;
    }
    if (selected)
    {
        display_data(display_gfx_button_sel[0], 64, 5, start, false);
        display_data(display_gfx_button_sel[1], 64, 6, start, false);
        display_data(display_gfx_button_sel[2], 64, 7, start, false);
    }
    else
    {
        display_data(display_gfx_button[0], 64, 5, start, false);
        display_data(display_gfx_button[1], 64, 6, start, false);
        display_data(display_gfx_button[2], 64, 7, start, false);
    }
    // text
    size_t text_length = strlen(text);
    if (strlen(text) > 6)
    {
        text_length = 6;
    }
    size_t length = 0;
    uint8_t *textdata = display_text_to_data(text, text_length, &length);

    uint8_t offset = 0;
    if (text_length < 6)
    {
        offset = (6 - text_length) / 2 * 8;
    }

    display_data(textdata, length, 6, start + 8 + offset, selected);
    free(textdata);
}

void display_menu_headline(char *text, bool arrows, uint8_t line)
{
    if (arrows)
    {
        display_data(display_gfx_arrow_left, 8, line, 0, false);
        display_data(display_gfx_arrow_right, 8, line, 15 * 8, false);
    }
    // bounday
    display_data(display_gfx_menu_head, 112, line, 8, false);

    // text
    size_t text_length = strlen(text);
    if (strlen(text) > 10)
    {
        text_length = 10;
    }
    size_t length = 0;
    uint8_t *textdata = display_text_to_data(text, text_length, &length);

    uint8_t offset = 0;
    if (text_length < 10)
    {
        offset = (10 - text_length) / 2 * 8;
    }

    display_data(textdata, length, line, 24 + offset, true);
    free(textdata);
}

void display_set_color(uint16_t color)
{
    current_color = color;
}

uint16_t display_get_color(void)
{
    return current_color;
}