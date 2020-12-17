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
#ifndef _PMW_AXP192_H_
#define _PWM_AXP192_H_

#include "stdio.h"

#define AXP192_ADDRESS 0x34

void axp192_start(void);
void axp192_screen_breath(uint8_t brightness);
bool axp192_get_bat_state();

void axp192_enable_coulombcounter(void);
void axp192_disable_coulombcounter(void);
void axp192_stop_coulombcounter(void);
void axp192_clear_coulombcounter(void);
uint32_t axp192_get_coulombcharge_data(void);
uint32_t axp192_get_coulombdischarge_data(void);
float axp192_get_coulomb_data(void);

uint8_t axp192_get_btn_press(void);

// -- sleep
void axp192_set_sleep(void);
void axp192_deep_sleep(uint64_t time_in_us);
void axp192_light_sleep(uint64_t time_in_us);
uint8_t axp192_get_warning_leve(void);

float axp192_get_bat_voltage();
float axp192_get_bat_current();
float axp192_get_vin_voltage();
float axp192_get_vin_current();
float axp192_get_vbus_voltage();
float axp192_get_vbus_current();
float axp192_get_temp();
float axp192_get_bat_power();
float axp192_get_bat_charge_current();
float axp192_get_aps_voltage();
float axp192_get_bat_coulomb_input();
float axp192_get_bat_coulomb_out();
uint8_t axp192_get_warning_level(void);
void axp192_set_coulomb_clear();
void axp192_set_ldo2(bool state);

void axp192_write_byte(uint8_t addr, uint8_t data);
uint8_t axp192_read_8bit(uint8_t addr);
uint16_t axp192_read_12bit(uint8_t addr);
uint16_t axp192_read_13bit(uint8_t addr);
uint16_t axp192_read_16bit(uint8_t addr);
uint32_t axp192_read_24bit(uint8_t addr);
uint32_t axp192_read_32bit(uint8_t addr);
void axp192_read_buff(uint8_t addr, uint8_t size, uint8_t *buff);

#endif