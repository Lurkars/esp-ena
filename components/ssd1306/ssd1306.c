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

#include "driver/i2c.h"
#include "esp_log.h"

#include "i2c-main.h"
#include "ssd1306-gfx.h"

#include "ssd1306.h"

uint8_t ssd1306_utf8_to_ascii_char(uint8_t ascii)
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

void ssd1306_utf8_to_ascii(char *input, char *output)
{
    strcpy(output, input);
    int k = 0;
    char c;
    for (int i = 0; i < strlen(output); i++)
    {
        c = ssd1306_utf8_to_ascii_char(output[i]);
        if (c != 0)
            output[k++] = c;
    }
    output[k] = 0;
}

void ssd1306_start(uint8_t i2address)
{
    if (!i2c_is_initialized())
    {
        i2c_main_init();
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    // Begin the I2C comm with SSD1306's address (SLA+Write)
    i2c_master_write_byte(cmd, (i2address << 1) | I2C_MASTER_WRITE, true);
    // Tell the SSD1306 that a command stream is incoming
    i2c_master_write_byte(cmd, SSD1306_CONTROL_CMD_STREAM, true);
    // Turn the Display OFF
    i2c_master_write_byte(cmd, SSD1306_CMD_OFF, true);
    // Set mux ration tp select max number of rows - 64
    i2c_master_write_byte(cmd, SSD1306_CMD_MULTIPLEX_RATIO, true);
    i2c_master_write_byte(cmd, 0x3F, true);
    // Set the display offset to 0
    i2c_master_write_byte(cmd, SSD1306_CMD_OFFSET, true);
    i2c_master_write_byte(cmd, 0x00, true);
    // Display start line to 0
    i2c_master_write_byte(cmd, SSD1306_CMD_START_LINE, true);
    // Mirror the x-axis. In case you set it up such that the pins are north.
    i2c_master_write_byte(cmd, SSD1306_CMD_SEGMENT_HIGH, true);
    // Mirror the y-axis. In case you set it up such that the pins are north.
    i2c_master_write_byte(cmd, SSD1306_CMD_SCAN_DIRECTION_REMAPPED, true);
    // Default - alternate COM pin map
    i2c_master_write_byte(cmd, SSD1306_CMD_COM_PINS, true);
    i2c_master_write_byte(cmd, 0x12, true);
    // set contrast
    i2c_master_write_byte(cmd, SSD1306_CMD_CONTRAST, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    // Set display to enable rendering from GDDRAM (Graphic Display Data RAM)
    i2c_master_write_byte(cmd, SSD1306_CMD_RAM, true);
    // Normal mode!
    i2c_master_write_byte(cmd, SSD1306_CMD_NORMAL, true);
    // Default oscillator clock
    i2c_master_write_byte(cmd, SSD1306_CMD_CLOCK, true);
    i2c_master_write_byte(cmd, 0x80, true);
    // Enable the charge pump
    i2c_master_write_byte(cmd, SSD1306_CMD_CHARGE_PUMP, true);
    i2c_master_write_byte(cmd, 0x14, true);
    // Set precharge cycles to high cap type
    i2c_master_write_byte(cmd, SSD1306_CMD_PRE_CHARGE_PERIOD, true);
    i2c_master_write_byte(cmd, 0x22, true);
    // Set the V_COMH deselect volatage to max
    i2c_master_write_byte(cmd, SSD1306_CMD_VCOMH, true);
    i2c_master_write_byte(cmd, 0x30, true);
    // Horizonatal addressing mode to page addressing
    i2c_master_write_byte(cmd, SSD1306_CMD_MEMORY_MODE, true);
    i2c_master_write_byte(cmd, 0x02, true);
    //i2c_master_write_byte(cmd, 0x00, true);
    // i2c_master_write_byte(cmd, 0x10, true);
    // i2c_master_write_byte(cmd, SSD1306_CMD_SCROLL_STOP, true);
    // Turn the Display ON
    i2c_master_write_byte(cmd, SSD1306_CMD_ON, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

void ssd1306_init_data(uint8_t i2address)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Begin the I2C comm with SSD1306's address (SLA+Write)
    i2c_master_write_byte(cmd, (i2address << 1) | I2C_MASTER_WRITE, true);
    // Tell the SSD1306 that a command stream is incoming
    i2c_master_write_byte(cmd, SSD1306_CONTROL_CMD_STREAM, true);

    // set column start + end
    i2c_master_write_byte(cmd, SSD1306_CMD_COLUMN_LOW, true);
    i2c_master_write_byte(cmd, SSD1306_CMD_COLUMN_HIGH, true);

    // set page
    i2c_master_write_byte(cmd, SSD1306_CMD_PAGE, true);

    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

void ssd1306_clear_line(uint8_t i2address, uint8_t line, bool invert)
{
    i2c_cmd_handle_t cmd;
    uint8_t *zeros = calloc(SSD1306_COLUMNS, sizeof(uint8_t));
    // set line
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2address << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, SSD1306_CONTROL_CMD_STREAM, true);
    i2c_master_write_byte(cmd, SSD1306_CMD_COLUMN_LOW, true);
    i2c_master_write_byte(cmd, SSD1306_CMD_COLUMN_HIGH, true);
    i2c_master_write_byte(cmd, SSD1306_CMD_PAGE | line, true);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    // fill line with zeros
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SSD1306_CONTROL_DATA_STREAM, true);
    i2c_master_write(cmd, zeros, SSD1306_COLUMNS, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    free(zeros);
}

void ssd1306_clear(uint8_t i2address)
{
    for (uint8_t i = 0; i < SSD1306_PAGES; i++)
    {
        ssd1306_clear_line(i2address, i, false);
    }
}

void ssd1306_on(uint8_t i2address, bool on)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    // Begin the I2C comm with SSD1306's address (SLA+Write)
    i2c_master_write_byte(cmd, (i2address << 1) | I2C_MASTER_WRITE, true);
    // Tell the SSD1306 that a command stream is incoming
    i2c_master_write_byte(cmd, SSD1306_CONTROL_CMD_STREAM, true);
    if (on)
    {
        // Turn the Display ON
        i2c_master_write_byte(cmd, SSD1306_CMD_ON, true);
    }
    else
    {
        // Turn the Display OFF
        i2c_master_write_byte(cmd, SSD1306_CMD_OFF, true);
    }

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

void ssd1306_data(uint8_t i2address, uint8_t *data, size_t length, uint8_t line, uint8_t offset, bool invert)
{

    uint8_t column = offset;
    if (column > SSD1306_COLUMNS)
    {
        column = 0;
    }
    size_t columns = length;
    if (columns > (SSD1306_COLUMNS - column))
    {
        columns = (SSD1306_COLUMNS - column);
    }

    ssd1306_init_data(i2address);
    i2c_cmd_handle_t cmd;

    // set line
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2address << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, SSD1306_CONTROL_CMD_STREAM, true);
    i2c_master_write_byte(cmd, SSD1306_CMD_COLUMN_LOW | (column & 0XF), true);
    i2c_master_write_byte(cmd, SSD1306_CMD_COLUMN_HIGH | (column >> 4), true);
    i2c_master_write_byte(cmd, SSD1306_CMD_PAGE | line, true);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (invert)
    {
        for (uint8_t i = 0; i < columns; i++)
        {
            data[i] = ~data[i];
        }
    }

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SSD1306_CONTROL_DATA_STREAM, true);

    i2c_master_write(cmd, data, columns, true);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

uint8_t *ssd1306_text_to_data(char *text, size_t text_length, size_t *length)
{
    char target_text[strlen(text)];
    ssd1306_utf8_to_ascii(text, target_text);

    uint8_t font_width = sizeof(ssd1306_gfx_font[0]);

    *length = text_length * font_width;

    uint8_t *data = calloc(*length, sizeof(uint8_t));
    for (uint8_t i = 0; i < text_length; i++)
    {
        memcpy(&data[i * font_width], ssd1306_gfx_font[(uint8_t)target_text[i] - 32], font_width);
    }

    return data;
}

void ssd1306_chars(uint8_t i2address, char *text, size_t length, uint8_t line, uint8_t offset, bool invert)
{
    if (length > 0)
    {
        uint8_t font_width = sizeof(ssd1306_gfx_font[0]);
        size_t res_length = 0;
        uint8_t *textdata = ssd1306_text_to_data(text, length, &res_length);
        ssd1306_data(i2address, textdata, res_length, line, offset * font_width, invert);
        free(textdata);
    }
}

void ssd1306_text_line_column(uint8_t i2address, char *text, uint8_t line, uint8_t offset, bool invert)
{
    ssd1306_chars(i2address, text, strlen(text), line, offset, invert);
}

void ssd1306_text_line(uint8_t i2address, char *text, uint8_t line, bool invert)
{
    ssd1306_text_line_column(i2address, text, line, 0, invert);
}

void ssd1306_text_input(uint8_t i2address, char *text, uint8_t position)
{
    size_t start = 0;
    if (position > 13)
    {
        position = 13;
        start = position - 13;
    }
    uint8_t cur_char = (uint8_t)text[start + position] - 32;

    // arrow
    ssd1306_data(i2address, ssd1306_gfx_arrow_left, 8, 2, 0, false);
    // bounday
    ssd1306_text_line_column(i2address, "______________", 2, 1, false);
    // arrow
    ssd1306_data(i2address, ssd1306_gfx_arrow_right, 8, 2, 15 * 8, false);
    // text
    size_t text_length = strlen(text);
    if (strlen(text) > 14)
    {
        text_length = 14;
    }
    size_t length = 0;
    uint8_t *textdata = ssd1306_text_to_data(text, text_length, &length);
    ssd1306_data(i2address, textdata, length, 2, 8, true);
    free(textdata);
    // arrow
    ssd1306_data(i2address, ssd1306_gfx_arrow_up, 8, 0, (position + 1) * 8, false);
    // upper char
    ssd1306_data(i2address, ssd1306_gfx_font[cur_char - 1], 8, 1, (position + 1) * 8, false);
    // sel char
    ssd1306_data(i2address, ssd1306_gfx_font[cur_char], 8, 2, (position + 1) * 8, false);
    // lower char
    ssd1306_data(i2address, ssd1306_gfx_font[cur_char + 1], 8, 3, (position + 1) * 8, false);
    // arrow
    ssd1306_data(i2address, ssd1306_gfx_arrow_down, 8, 4, (position + 1) * 8, false);
}

void ssd1306_set_button(uint8_t i2address, char *text, bool selected, bool primary)
{
    uint8_t start = 0;
    if (primary)
    {
        start = 64;
    }
    if (selected)
    {
        ssd1306_data(i2address, ssd1306_gfx_button_sel[0], 64, 5, start, false);
        ssd1306_data(i2address, ssd1306_gfx_button_sel[1], 64, 6, start, false);
        ssd1306_data(i2address, ssd1306_gfx_button_sel[2], 64, 7, start, false);
    }
    else
    {
        ssd1306_data(i2address, ssd1306_gfx_button[0], 64, 5, start, false);
        ssd1306_data(i2address, ssd1306_gfx_button[1], 64, 6, start, false);
        ssd1306_data(i2address, ssd1306_gfx_button[2], 64, 7, start, false);
    }
    // text
    size_t text_length = strlen(text);
    if (strlen(text) > 6)
    {
        text_length = 6;
    }
    size_t length = 0;
    uint8_t *textdata = ssd1306_text_to_data(text, text_length, &length);

    uint8_t offset = 0;
    if (text_length < 6)
    {
        offset = (6 - text_length) / 2 * 8;
    }

    ssd1306_data(i2address, textdata, length, 6, start + 8 + offset, selected);
    free(textdata);
}

void ssd1306_menu_headline(uint8_t i2address, char *text, bool arrows, uint8_t line)
{
    if (arrows)
    {
        ssd1306_data(i2address, ssd1306_gfx_arrow_left, 8, line, 0, false);
        ssd1306_data(i2address, ssd1306_gfx_arrow_right, 8, line, 15 * 8, false);
    }
    // bounday
    ssd1306_data(i2address, ssd1306_gfx_menu_head, 112, line, 8, false);

    // text
    size_t text_length = strlen(text);
    if (strlen(text) > 10)
    {
        text_length = 10;
    }
    size_t length = 0;
    uint8_t *textdata = ssd1306_text_to_data(text, text_length, &length);

    uint8_t offset = 0;
    if (text_length < 10)
    {
        offset = (10 - text_length) / 2 * 8;
    }

    ssd1306_data(i2address, textdata, length, line, 24 + offset, true);
    free(textdata);
}