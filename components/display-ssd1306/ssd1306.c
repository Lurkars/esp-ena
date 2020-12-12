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

#include "display.h"
#include "display-gfx.h"
#include "ssd1306.h"

void display_start(void)
{
    if (!i2c_is_initialized())
    {
        i2c_main_init();
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    // Begin the I2C comm with SSD1306's address (SLA+Write)
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);
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

void display_init_data(void)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Begin the I2C comm with SSD1306's address (SLA+Write)
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);
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

void display_clear_line(uint8_t line, bool invert)
{
    i2c_cmd_handle_t cmd;
    uint8_t *zeros = calloc(SSD1306_COLUMNS, sizeof(uint8_t));
    // set line
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);

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
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SSD1306_CONTROL_DATA_STREAM, true);
    i2c_master_write(cmd, zeros, SSD1306_COLUMNS, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    free(zeros);
}

void display_clear(void)
{
    for (uint8_t i = 0; i < SSD1306_PAGES; i++)
    {
        display_clear_line(i, false);
    }
}

void display_on(bool on)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    // Begin the I2C comm with SSD1306's address (SLA+Write)
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);
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

void display_data(uint8_t *data, size_t length, uint8_t line, uint8_t offset, bool invert)
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

    display_init_data();
    i2c_cmd_handle_t cmd;

    // set line
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);

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
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SSD1306_CONTROL_DATA_STREAM, true);

    i2c_master_write(cmd, data, columns, true);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

void display_flipped(bool flipped)
{

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Begin the I2C comm with SSD1306's address (SLA+Write)
    i2c_master_write_byte(cmd, (SSD1306_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    // Tell the SSD1306 that a command stream is incoming
    i2c_master_write_byte(cmd, SSD1306_CONTROL_CMD_STREAM, true);
    i2c_master_write_byte(cmd, SSD1306_CMD_SEGMENT_HIGH, true);

    if (flipped)
    {
        i2c_master_write_byte(cmd, SSD1306_CMD_SCAN_DIRECTION_NORMAL, true);
    }
    else
    {
        i2c_master_write_byte(cmd, SSD1306_CMD_SCAN_DIRECTION_REMAPPED, true);
    }

    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}