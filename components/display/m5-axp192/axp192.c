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
#include "esp_sleep.h"
#include "esp_log.h"

#include "i2c-main.h"

#include "axp192.h"

void axp192_write_byte(uint8_t addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    // Begin the I2C comm with AXP192_ADDRESS's address (SLA+Write)
    i2c_master_write_byte(cmd, (AXP192_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, addr, true);
    i2c_master_write_byte(cmd, data, true);

    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

void axp192_read_buff(uint8_t addr, uint8_t size, uint8_t *buff)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Send register address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, addr, true);

    // Receive data
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buff, size, I2C_MASTER_LAST_NACK);

    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

uint8_t axp192_read_8bit(uint8_t addr)
{
    uint8_t data;
    axp192_read_buff(addr, 1, &data);
    return data;
}

uint16_t axp192_read_12bit(uint8_t addr)
{
    uint16_t data = 0;
    uint8_t buf[2];
    axp192_read_buff(addr, 2, buf);
    data = ((buf[0] << 4) + buf[1]); //
    return data;
}

uint16_t axp192_read_13bit(uint8_t addr)
{
    uint16_t data = 0;
    uint8_t buf[2];
    axp192_read_buff(addr, 2, buf);
    data = ((buf[0] << 5) + buf[1]); //
    return data;
}

uint16_t axp192_read_16bit(uint8_t addr)
{
    uint16_t data = 0;
    uint8_t buf[2];
    axp192_read_buff(addr, 2, buf);
    data = ((buf[0] << 8) + buf[1]);
    return data;
}

uint32_t axp192_read_24bit(uint8_t addr)
{
    uint32_t data = 0;
    uint8_t buf[3];
    axp192_read_buff(addr, 3, buf);
    data = ((buf[0] << 16) + (buf[1] << 8) + buf[2]);
    return data;
}

uint32_t axp192_read_32bit(uint8_t addr)
{
    uint32_t data = 0;
    uint8_t buf[4];
    axp192_read_buff(addr, 4, buf);
    data = ((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
    return data;
}

void axp192_screen_breath(uint8_t brightness)
{
    if (brightness > 12)
    {
        brightness = 12;
    }
    uint8_t buf = axp192_read_8bit(0x28);
    axp192_write_byte(0x28, ((buf & 0x0f) | (brightness << 4)));
}

void axp192_start(void)
{
    if (!i2c_is_initialized())
    {
        i2c_main_init();
    }

    // Set LDO2 & LDO3(ST7789_LED & TFT) 3.0V
    axp192_write_byte(0x28, 0xcc);

    // Set ADC sample rate to 200hz
    axp192_write_byte(0x84, 0xF2);

    // Set ADC to All Enable
    axp192_write_byte(0x82, 0xff);

    // Bat charge voltage to 4.2, Current 100MA
    axp192_write_byte(0x33, 0xc0);

    // Enable Bat,ACIN,VBUS,APS adc
    axp192_write_byte(0x82, 0xff);

    // Enable Ext, LDO2, LDO3, DCDC1
    axp192_write_byte(0x12, axp192_read_8bit(0x12) | 0x4D);

    // 128ms power on, 4s power off
    axp192_write_byte(0x36, 0x0C);

    // Set RTC voltage to 3.3V
    axp192_write_byte(0x91, 0xF0);

    // Set GPIO0 to LDO
    axp192_write_byte(0x90, 0x02);

    // Disable vbus hold limit
    axp192_write_byte(0x30, 0x80);

    // Set temperature protection
    axp192_write_byte(0x39, 0xfc);

    // Enable RTC BAT charge
    axp192_write_byte(0x35, 0xa2);

    // Enable bat detection
    axp192_write_byte(0x32, 0x46);
}

bool axp192_get_bat_state()
{
    if (axp192_read_8bit(0x01) | 0x20)
        return true;
    else
        return false;
}
//---------coulombcounter_from_here---------
//enable: void EnableCoulombcounter(void);
//disable: void DisableCOulombcounter(void);
//stop: void StopCoulombcounter(void);
//clear: void ClearCoulombcounter(void);
//get charge data: uint32_t GetCoulombchargedata(void);
//get discharge data: uint32_t GetCoulombdischargedata(void);
//get coulomb val affter calculation: float GetCoulombdata(void);
//------------------------------------------
void axp192_enable_coulombcounter(void)
{
    axp192_write_byte(0xB8, 0x80);
}

void axp192_disable_coulombcounter(void)
{
    axp192_write_byte(0xB8, 0x00);
}

void axp192_stop_coulombcounter(void)
{
    axp192_write_byte(0xB8, 0xC0);
}

void axp192_clear_coulombcounter(void)
{
    axp192_write_byte(0xB8, 0xA0);
}

uint32_t axp192_get_coulombcharge_data(void)
{
    return axp192_read_32bit(0xB0);
}

uint32_t axp192_get_coulombdischarge_data(void)
{
    return axp192_read_32bit(0xB4);
}

float axp192_get_coulomb_data(void)
{

    uint32_t coin = 0;
    uint32_t coout = 0;

    coin = axp192_get_coulombcharge_data();
    coout = axp192_get_coulombdischarge_data();

    //c = 65536 * current_LSB * (coin - coout) / 3600 / ADC rate
    //Adc rate can be read from 84H ,change this variable if you change the ADC reate
    float ccc = 65536 * 0.5 * (coin - coout) / 3600.0 / 25.0;
    return ccc;
}
//----------coulomb_end_at_here----------

uint16_t axp192_get_Vbatdata(void)
{

    uint16_t vbat = 0;
    uint8_t buf[2];
    axp192_read_buff(0x78, 2, buf);
    vbat = ((buf[0] << 4) + buf[1]); // V
    return vbat;
}

uint16_t axp192_get_Vindata(void)
{
    uint16_t vin = 0;
    uint8_t buf[2];
    axp192_read_buff(0x56, 2, buf);
    vin = ((buf[0] << 4) + buf[1]); // V
    return vin;
}

uint16_t axp192_get_Iindata(void)
{
    uint16_t iin = 0;
    uint8_t buf[2];
    axp192_read_buff(0x58, 2, buf);
    iin = ((buf[0] << 4) + buf[1]);
    return iin;
}

uint16_t axp192_get_Vusbindata(void)
{
    uint16_t vin = 0;
    uint8_t buf[2];
    axp192_read_buff(0x5a, 2, buf);
    vin = ((buf[0] << 4) + buf[1]); // V
    return vin;
}

uint16_t axp192_get_Iusbindata(void)
{
    uint16_t iin = 0;
    uint8_t buf[2];
    axp192_read_buff(0x5C, 2, buf);
    iin = ((buf[0] << 4) + buf[1]);
    return iin;
}

uint16_t axp192_get_Ichargedata(void)
{

    uint16_t icharge = 0;
    uint8_t buf[2];
    axp192_read_buff(0x7A, 2, buf);
    icharge = (buf[0] << 5) + buf[1];
    return icharge;
}

uint16_t axp192_get_Idischargedata(void)
{
    uint16_t idischarge = 0;
    uint8_t buf[2];
    axp192_read_buff(0x7C, 2, buf);
    idischarge = (buf[0] << 5) + buf[1];
    return idischarge;
}

uint16_t axp192_get_Tempdata(void)
{
    uint16_t temp = 0;
    uint8_t buf[2];
    axp192_read_buff(0x5e, 2, buf);
    temp = ((buf[0] << 4) + buf[1]);
    return temp;
}

uint32_t axp192_get_Powerbatdata(void)
{
    uint32_t power = 0;
    uint8_t buf[3];
    axp192_read_buff(0x70, 2, buf);
    power = (buf[0] << 16) + (buf[1] << 8) + buf[2];
    return power;
}

uint16_t axp192_get_Vapsdata(void)
{
    uint16_t vaps = 0;
    uint8_t buf[2];
    axp192_read_buff(0x7e, 2, buf);
    vaps = ((buf[0] << 4) + buf[1]);
    return vaps;
}

void axp192_set_sleep(void)
{
    uint8_t buf = axp192_read_8bit(0x31);
    buf = (1 << 3) | buf;
    axp192_write_byte(0x31, buf);
    axp192_write_byte(0x90, 0x00);
    axp192_write_byte(0x12, 0x09);
    axp192_write_byte(0x12, 0x00);
}

uint8_t axp192_get_warning_leve(void)
{
    uint8_t buf = axp192_read_8bit(0x47);
    return (buf & 0x01);
}

// -- sleep
void axp192_deep_sleep(uint64_t time_in_us)
{
    axp192_set_sleep();

    if (time_in_us > 0)
    {
        esp_sleep_enable_timer_wakeup(time_in_us);
    }
    else
    {
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    }
    (time_in_us == 0) ? esp_deep_sleep_start() : esp_deep_sleep(time_in_us);
}

void axp192_light_sleep(uint64_t time_in_us)
{
    axp192_set_sleep();

    if (time_in_us > 0)
    {
        esp_sleep_enable_timer_wakeup(time_in_us);
    }
    else
    {
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    }
    esp_light_sleep_start();
}

// 0 not press, 0x01 long press, 0x02 press
uint8_t axp192_get_btn_press()
{
    uint8_t state = axp192_read_8bit(0x46);
    if (state)
    {
        axp192_write_byte(0x46, 0x03);
    }
    return state;
}

uint8_t axp192_get_warning_level(void)
{
    return axp192_read_8bit(0x47) & 0x01;
}

float axp192_get_bat_voltage()
{
    float ADCLSB = 1.1 / 1000.0;
    uint16_t Redata = axp192_read_12bit(0x78);
    return Redata * ADCLSB;
}

float axp192_get_bat_current()
{
    float ADCLSB = 0.5;
    uint16_t CurrentIn = axp192_read_13bit(0x7A);
    uint16_t CurrentOut = axp192_read_13bit(0x7C);
    return (CurrentIn - CurrentOut) * ADCLSB;
}

float axp192_get_vin_voltage()
{
    float ADCLSB = 1.7 / 1000.0;
    uint16_t Redata = axp192_read_12bit(0x56);
    return Redata * ADCLSB;
}

float axp192_get_vin_current()
{
    float ADCLSB = 0.625;
    uint16_t Redata = axp192_read_12bit(0x58);
    return Redata * ADCLSB;
}

float axp192_get_vbus_voltage()
{
    float ADCLSB = 1.7 / 1000.0;
    uint16_t Redata = axp192_read_12bit(0x5A);
    return Redata * ADCLSB;
}

float axp192_get_vbus_current()
{
    float ADCLSB = 0.375;
    uint16_t Redata = axp192_read_12bit(0x5C);
    return Redata * ADCLSB;
}

float axp192_get_temp()
{
    float ADCLSB = 0.1;
    const float OFFSET_DEG_C = -144.7;
    uint16_t Redata = axp192_read_12bit(0x5E);
    return OFFSET_DEG_C + Redata * ADCLSB;
}

float axp192_get_bat_power()
{
    float VoltageLSB = 1.1;
    float CurrentLCS = 0.5;
    uint32_t Redata = axp192_read_24bit(0x70);
    return VoltageLSB * CurrentLCS * Redata / 1000.0;
}

float axp192_get_bat_charge_current()
{
    float ADCLSB = 0.5;
    uint16_t Redata = axp192_read_12bit(0x7A);
    return Redata * ADCLSB;
}
float axp192_get_aps_voltage()
{
    float ADCLSB = 1.4 / 1000.0;
    uint16_t Redata = axp192_read_12bit(0x7E);
    return Redata * ADCLSB;
}

float axp192_get_bat_coulomb_input()
{
    uint32_t Redata = axp192_read_32bit(0xB0);
    return Redata * 65536 * 0.5 / 3600 / 25.0;
}

float axp192_get_bat_coulomb_out()
{
    uint32_t Redata = axp192_read_32bit(0xB4);
    return Redata * 65536 * 0.5 / 3600 / 25.0;
}

void axp192_set_coulomb_clear()
{
    axp192_write_byte(0xB8, 0x20);
}

void axp192_set_ldo2(bool state)
{
    uint8_t buf = axp192_read_8bit(0x12);
    if (state == true)
        buf = (1 << 2) | buf;
    else
        buf = ~(1 << 2) & buf;
    axp192_write_byte(0x12, buf);
}
