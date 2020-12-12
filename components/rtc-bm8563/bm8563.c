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
#include <time.h>

#include "driver/i2c.h"
#include "esp_log.h"

#include "rtc.h"
#include "i2c-main.h"

#include "bm8563.h"

uint8_t bm8563_dec2bcd(uint8_t value)
{
    return (((value / 10) << 4) | (value % 10));
}

uint8_t bm8563_bcd2dec(uint8_t value)
{
    return (((value >> 4) * 10) + (value & 0x0f));
}

/**
 * @brief Read time 
 */
void rtc_get_time(struct tm *time)
{
    if (!i2c_is_initialized())
    {
        i2c_main_init();
    }
    uint8_t data[7];

    i2c_cmd_handle_t cmd;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BM8563_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, BM8563_SECONDS, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BM8563_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 7, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);

    time->tm_sec = bm8563_bcd2dec(data[0] & 0b01111111);
    time->tm_min = bm8563_bcd2dec(data[1] & 0b01111111);
    time->tm_hour = bm8563_bcd2dec(data[2] & 0b00111111);
    time->tm_mday = bm8563_bcd2dec(data[3] & 0b00111111);
    time->tm_wday = bm8563_bcd2dec(data[4] & 0b00000111);
    time->tm_mon = bm8563_bcd2dec(data[5] & 0b00011111) - 1;
    if (data[5] & BM8563_CENTURY_BIT)
    {
        time->tm_year = bm8563_bcd2dec(data[6] & 0b11111111) + 100;
    }
    else
    {
        time->tm_year = bm8563_bcd2dec(data[6] & 0b01111111);
    }
    time->tm_isdst = 0;

    mktime(time);
}

/**
 * @brief Write time
 */
void rtc_set_time(struct tm *time)
{
    if (!i2c_is_initialized())
    {
        i2c_main_init();
    }

    uint8_t data[7] = {0};
    data[0] = bm8563_dec2bcd(time->tm_sec) & 0b01111111;
    data[1] = bm8563_dec2bcd(time->tm_min) & 0b01111111;
    data[2] = bm8563_dec2bcd(time->tm_hour) & 0b00111111;
    data[3] = bm8563_dec2bcd(time->tm_mday) & 0b00111111;
    data[4] = bm8563_dec2bcd(time->tm_wday) & 0b00000111;
    data[5] = bm8563_dec2bcd(time->tm_mon + 1) & 0b00011111;
    if (time->tm_year > 100)
    {
        data[5] |= BM8563_CENTURY_BIT;
    }

    data[6] = bm8563_dec2bcd(time->tm_year % 100) & 0b11111111;

    i2c_cmd_handle_t cmd;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BM8563_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, BM8563_SECONDS, true);
    i2c_master_write(cmd, data, 7, true);

    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}