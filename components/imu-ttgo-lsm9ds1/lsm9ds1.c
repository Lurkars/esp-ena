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

#include "i2c-main.h"

#include "lsm9ds1.h"

float aRes, gRes;

void lsm9ds1_i2c_read_bytes(uint8_t driver_addr, uint8_t start_addr, uint8_t number_Bytes, uint8_t *read_buffer)
{
    i2c_cmd_handle_t cmd;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (driver_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, start_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (driver_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, read_buffer, number_Bytes, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

void lsm9ds1_i2c_write_bytes(uint8_t driver_addr, uint8_t start_addr, uint8_t number_Bytes, uint8_t *write_buffer)
{
    i2c_cmd_handle_t cmd;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (driver_addr << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, start_addr, true);
    i2c_master_write(cmd, write_buffer, number_Bytes, true);

    i2c_master_stop(cmd);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

int lsm9ds1_start(void)
{
    unsigned char regdata;
    // init ACC
    regdata = 0x38;
    lsm9ds1_i2c_write_bytes(ACC_ADDR, CTRL_REG5_A, 1, &regdata);
    regdata = 0xC0;
    lsm9ds1_i2c_write_bytes(ACC_ADDR, CTRL_REG6_A, 1, &regdata);

    lsm9ds1_i2c_read_bytes(ACC_ADDR, CTRL_REG6_A, 1, &regdata);
    regdata &= ~(0b00011000);
    regdata |= ACCELRANGE_16G;
    lsm9ds1_i2c_write_bytes(ACC_ADDR, CTRL_REG6_A, 1, &regdata);

    // init gyr
    regdata = 0xC0;
    lsm9ds1_i2c_write_bytes(GYR_ADDR, CTRL_REG1_G, 1, &regdata);
    lsm9ds1_i2c_read_bytes(GYR_ADDR, CTRL_REG1_G, 1, &regdata);
    regdata &= ~(0b00011000);
    regdata |= GYROSCALE_500DPS;
    lsm9ds1_i2c_write_bytes(GYR_ADDR, CTRL_REG1_G, 1, &regdata);

    aRes = 16.0 / 32768.0;
    gRes = 500.0 / 32768.0;
    return 0; 
}

void lsm9ds1_get_accel_adc(int16_t *ax, int16_t *ay, int16_t *az)
{

    uint8_t buf[6];
    lsm9ds1_i2c_read_bytes(ACC_ADDR, OUT_X_L_A, 6, buf);

    *ax = ((int16_t)buf[0] << 8) | buf[1];
    *ay = ((int16_t)buf[2] << 8) | buf[3];
    *az = ((int16_t)buf[4] << 8) | buf[5];
}
void lsm9ds1_get_gyro_adc(int16_t *gx, int16_t *gy, int16_t *gz)
{

    uint8_t buf[6];
    lsm9ds1_i2c_read_bytes(GYR_ADDR, OUT_X_L_G, 6, buf);

    *gx = ((uint16_t)buf[0] << 8) | buf[1];
    *gy = ((uint16_t)buf[2] << 8) | buf[3];
    *gz = ((uint16_t)buf[4] << 8) | buf[5];
}

void lsm9ds1_get_accel_data(float *ax, float *ay, float *az)
{

    int16_t accX = 0;
    int16_t accY = 0;
    int16_t accZ = 0;
    lsm9ds1_get_accel_adc(&accX, &accY, &accZ);

    *ax = (float)accX * aRes;
    *ay = (float)accY * aRes;
    *az = (float)accZ * aRes;
}

void lsm9ds1_get_gyro_data(float *gx, float *gy, float *gz)
{
    int16_t gyroX = 0;
    int16_t gyroY = 0;
    int16_t gyroZ = 0;
    lsm9ds1_get_gyro_adc(&gyroX, &gyroY, &gyroZ);

    *gx = (float)gyroX * gRes;
    *gy = (float)gyroY * gRes;
    *gz = (float)gyroZ * gRes;
}
