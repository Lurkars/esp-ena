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