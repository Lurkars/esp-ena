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
/**
 * @file
 * 
 * @brief I2C driver for DS3231 Real Time Clock
 * 
 */
#ifndef _ds3231_H_
#define _ds3231_H_

#include <time.h>

// I2C address
#define DS3231_ADDRESS 0x68

// I2C registers
#define DS3231_TIME 0x00
#define DS3231_SECONDS 0x00
#define DS3231_MINUTES 0x01
#define DS3231_HOURS 0x02
#define DS3231_DAY 0x03
#define DS3231_DATE 0x04
#define DS3231_MONTH 0x05
#define DS3231_YEAR 0x06
#define DS3231_ALARM1_SECONDS 0x07
#define DS3231_ALARM1_MINUTES 0x08
#define DS3231_ALARM1_HOURS 0x09
#define DS3231_ALARM1_DATE 0x0A
#define DS3231_ALARM2_MINUTES 0x0B
#define DS3231_ALARM2_HOURS 0x0C
#define DS3231_ALARM2_DATE 0x0D
#define DS3231_CONTROL 0x0E
#define DS3231_STATUS 0x0F
#define DS3231_AGING_OFFSET 0x10
#define DS3231_MSB_TEMP 0x11
#define DS3231_LSB_TEMP 0x12

// control registers
#define DS3231_CONTROL_A1IE 0x01
#define DS3231_CONTROL_A2IE 0x02
#define DS3231_CONTROL_INTCN 0x04
#define DS3231_CONTROL_RS1 0x08
#define DS3231_CONTROL_RS2 0x10
#define DS3231_CONTROL_CONV 0x20
#define DS3231_CONTROL_BBSQW 0x40
#define DS3231_CONTROL_EOSC 0x80

// status registers
#define DS3231_STATUSL_A1F 0x01
#define DS3231_STATUSL_A2F 0x02
#define DS3231_STATUSL_BSY 0x04
#define DS3231_STATUSL_EN32KHZ 0x08
#define DS3231_STATUSL_OSF 0x80

// flags
#define DS3231_CENTURY_FLAG 0x80
#define DS3231_12_HOUR_FLAG 0x40
#define DS3231_PM_HOUR_FLAG 0x20
#define DS3231_12_HOUR_MASK 0x1F
#define DS3231_MONTH_MASK 0x1F

/**
 * @brief Read time from DS3231
 */
void ds3231_get_time(struct tm *time);

/**
 * @brief Write time to DS3231
 */
void ds3231_set_time(struct tm *time);

#endif