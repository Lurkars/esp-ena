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

#include "mpu6886.h"

int Gyscale = GFS_2000DPS;
int Acscale = AFS_8G;
float aRes, gRes;

void mpu6886_i2c_read_bytes(uint8_t driver_addr, uint8_t start_addr, uint8_t number_Bytes, uint8_t *read_buffer)
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

void mpu6886_i2c_write_bytes(uint8_t driver_addr, uint8_t start_addr, uint8_t number_Bytes, uint8_t *write_buffer)
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



void mpu6886_getGres()
{

    switch (Gyscale)
    {
        // Possible gyro scales (and their register bit settings) are:
    case GFS_250DPS:
        gRes = 250.0 / 32768.0;
        break;
    case GFS_500DPS:
        gRes = 500.0 / 32768.0;
        break;
    case GFS_1000DPS:
        gRes = 1000.0 / 32768.0;
        break;
    case GFS_2000DPS:
        gRes = 2000.0 / 32768.0;
        break;
    }
}

void mpu6886_getAres()
{
    switch (Acscale)
    {
        // Possible accelerometer scales (and their register bit settings) are:
        // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
    case AFS_2G:
        aRes = 2.0 / 32768.0;
        break;
    case AFS_4G:
        aRes = 4.0 / 32768.0;
        break;
    case AFS_8G:
        aRes = 8.0 / 32768.0;
        break;
    case AFS_16G:
        aRes = 16.0 / 32768.0;
        break;
    }
}

int mpu6886_start(void)
{
    unsigned char tempdata[1];
    unsigned char regdata;

    if (!i2c_is_initialized())
    {
        i2c_main_init();
    }

    mpu6886_i2c_read_bytes(MPU6886_ADDRESS, MPU6886_WHOAMI, 1, tempdata);
    if (tempdata[0] != 0x19)
        return -1;
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x00;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_PWR_MGMT_1, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);;

    regdata = (0x01 << 7);
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_PWR_MGMT_1, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);;

    regdata = (0x01 << 0);
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_PWR_MGMT_1, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);;

    regdata = 0x10;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_ACCEL_CONFIG, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x18;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_GYRO_CONFIG, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x01;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_CONFIG, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x05;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_SMPLRT_DIV, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x00;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_INT_ENABLE, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x00;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_ACCEL_CONFIG2, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x00;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_USER_CTRL, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x00;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_FIFO_EN, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x22;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_INT_PIN_CFG, 1, &regdata);
    vTaskDelay(1 / portTICK_PERIOD_MS);;

    regdata = 0x01;
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_INT_ENABLE, 1, &regdata);

    vTaskDelay(100 / portTICK_PERIOD_MS);;
    mpu6886_getGres();
    mpu6886_getAres();
    return 0;
}

void mpu6886_get_accel_adc(int16_t *ax, int16_t *ay, int16_t *az)
{

    uint8_t buf[6];
    mpu6886_i2c_read_bytes(MPU6886_ADDRESS, MPU6886_ACCEL_XOUT_H, 6, buf);

    *ax = ((int16_t)buf[0] << 8) | buf[1];
    *ay = ((int16_t)buf[2] << 8) | buf[3];
    *az = ((int16_t)buf[4] << 8) | buf[5];
}
void mpu6886_get_gyro_adc(int16_t *gx, int16_t *gy, int16_t *gz)
{

    uint8_t buf[6];
    mpu6886_i2c_read_bytes(MPU6886_ADDRESS, MPU6886_GYRO_XOUT_H, 6, buf);

    *gx = ((uint16_t)buf[0] << 8) | buf[1];
    *gy = ((uint16_t)buf[2] << 8) | buf[3];
    *gz = ((uint16_t)buf[4] << 8) | buf[5];
}

void mpu6886_get_temp_adc(int16_t *t)
{

    uint8_t buf[2];
    mpu6886_i2c_read_bytes(MPU6886_ADDRESS, MPU6886_TEMP_OUT_H, 2, buf);

    *t = ((uint16_t)buf[0] << 8) | buf[1];
}

void mpu6886_set_gyro_fsr(int scale)
{
    //return IIC_Write_Byte(MPU_GYRO_CFG_REG,scale<<3);//设置陀螺仪满量程范围
    unsigned char regdata;
    regdata = (scale << 3);
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_GYRO_CONFIG, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);;

    Gyscale = scale;
    mpu6886_getGres();
}

void mpu6886_set_accel_fsr(int scale)
{
    unsigned char regdata;
    regdata = (scale << 3);
    mpu6886_i2c_write_bytes(MPU6886_ADDRESS, MPU6886_ACCEL_CONFIG, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);;

    Acscale = scale;
    mpu6886_getAres();
}

void mpu6886_get_accel_data(float *ax, float *ay, float *az)
{

    int16_t accX = 0;
    int16_t accY = 0;
    int16_t accZ = 0;
    mpu6886_get_accel_adc(&accX, &accY, &accZ);

    *ax = (float)accX * aRes;
    *ay = (float)accY * aRes;
    *az = (float)accZ * aRes;
}

void mpu6886_get_gyro_data(float *gx, float *gy, float *gz)
{
    int16_t gyroX = 0;
    int16_t gyroY = 0;
    int16_t gyroZ = 0;
    mpu6886_get_gyro_adc(&gyroX, &gyroY, &gyroZ);

    *gx = (float)gyroX * gRes;
    *gy = (float)gyroY * gRes;
    *gz = (float)gyroZ * gRes;
}

void mpu6886_get_temp_data(float *t)
{

    int16_t temp = 0;
    mpu6886_get_temp_adc(&temp);

    *t = (float)temp / 326.8 + 25.0;
}
