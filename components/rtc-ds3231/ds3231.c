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

#include "ds3231.h"

uint8_t ds3231_dec2bcd(uint8_t value)
{
    return ((value / 10 * 16) + (value % 10));
}

uint8_t ds3231_bcd2dec(uint8_t value)
{
    return ((value / 16 * 10) + (value % 16));
}

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
    i2c_master_write_byte(cmd, (DS3231_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, DS3231_TIME, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 7, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);

    time->tm_sec = ds3231_bcd2dec(data[0]);
    time->tm_min = ds3231_bcd2dec(data[1]);
    if (data[2] & DS3231_12_HOUR_FLAG)
    {
        time->tm_hour = ds3231_bcd2dec(data[2] & DS3231_12_HOUR_MASK) - 1;
        if (data[2] & DS3231_PM_HOUR_FLAG)
        {
            time->tm_hour += 12;
        }
    }
    else
    {
        time->tm_hour = ds3231_bcd2dec(data[2]);
    }
    time->tm_wday = ds3231_bcd2dec(data[3]) - 1;
    time->tm_mday = ds3231_bcd2dec(data[4]);
    time->tm_mon = ds3231_bcd2dec(data[5] & DS3231_MONTH_MASK) - 1;
    uint8_t century = (data[5] & DS3231_CENTURY_FLAG) >> 7;
    if (century)
    {
        time->tm_year = ds3231_bcd2dec(data[6]) + 100;
    }
    else
    {
        time->tm_year = ds3231_bcd2dec(data[6]);
    }
    time->tm_isdst = 0;
}

void rtc_set_time(struct tm *time)
{
    if (!i2c_is_initialized())
    {
        i2c_main_init();
    }
    uint8_t data[7] = {0};
    data[0] = ds3231_dec2bcd(time->tm_sec);
    data[1] = ds3231_dec2bcd(time->tm_min);
    data[2] = ds3231_dec2bcd(time->tm_hour); // write 24h format
    data[3] = ds3231_dec2bcd(time->tm_wday + 1);
    data[4] = ds3231_dec2bcd(time->tm_mday);
    uint8_t century = 0;
    if (time->tm_year > 100)
    {
        century = DS3231_CENTURY_FLAG;
        data[6] = ds3231_dec2bcd(time->tm_year - 100);
    }
    else
    {
        data[6] = ds3231_dec2bcd(time->tm_year);
    }

    data[5] = ds3231_dec2bcd(time->tm_mon + 1) + century;

    i2c_cmd_handle_t cmd;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, DS3231_TIME, true);
    i2c_master_write(cmd, data, 7, true);

    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}
