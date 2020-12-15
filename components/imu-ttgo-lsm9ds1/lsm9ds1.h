/*
 Note: The MPU6886 is an I2C sensor and uses the Arduino Wire library.
 Because the sensor is not 5V tolerant, we are using a 3.3 V 8 MHz Pro Mini or
 a 3.3 V Teensy 3.1. We have disabled the internal pull-ups used by the Wire
 library in the Wire.h/twi.c utility file. We are also using the 400 kHz fast
 I2C mode by setting the TWI_FREQ  to 400000L /twi.h utility file.
 */
#ifndef _IMU_LSM9DS1_H_
#define _IMU_LSM9DS1_H_

#include "stdio.h"

#define ID_AG   0x6F
#define ID_A    0x6F
#define ID_G    0x6F
#define ID_M    0x3D

#define FILENAME "/dev/i2c-1"
#define MAG_ADDR 0x1E
#define ACC_ADDR 0x6B
#define GYR_ADDR 0x6B

// Shared Accelerometer/Gyroscope Addresses
#define WHO_AM_I_AG	    0x0F
#define CTRL_REG1_AG    0x10
#define CTRL_REG2_AG    0x11
#define CTRL_REG3_AG    0x12
#define OUT_TEMP_L_AG   0x15
#define OUT_TEMP_H_AG   0x16
#define REG_STATUS_REG_AG 0x17
#define CTRL_REG4_AG    0x1E
#define CTRL_REG5_AG    0x1F
#define CTRL_REG6_AG    0x20
#define CTRL_REG7_AG    0x21
#define CTRL_REG8_AG    0x22
#define CTRL_REG9_AG    0x23
#define CTRL_REG10_AG   0x24

// Gyroscope addresses
#define WHO_AM_I_G  0x0F
#define CTRL_REG1_G 0x10
#define CTRL_REG2_G 0x11
#define CTRL_REG3_G 0x12
#define OUT_X_L_G   0x18
#define OUT_X_H_G   0x19
#define OUT_Y_L_G   0x1A
#define OUT_Y_H_G   0x1B
#define OUT_Z_L_G   0x1C
#define OUT_Z_H_G   0x1D

// Accelerometer addresses
#define WHO_AM_I_A	    0x0F
#define CTRL_REG5_A     0x1F
#define CTRL_REG6_A     0x20
#define CTRL_REG7_A     0x21
#define OUT_X_L_A       0x28
#define OUT_X_H_A       0x29
#define OUT_Y_L_A       0x2A
#define OUT_Y_H_A       0x2B
#define OUT_Z_L_A       0x2C
#define OUT_Z_H_A       0x2D

// Magnetometer addresses
#define WHO_AM_I_M  0x0F
#define CTRL_REG1_M 0x20
#define CTRL_REG2_M 0x21
#define CTRL_REG3_M 0x22
#define CTRL_REG4_M 0x23
#define CTRL_REG5_M 0x24
#define REG_STATUS_REG_M 0x27
#define OUT_X_L_M   0x28
#define OUT_X_H_M   0x29
#define OUT_Y_L_M   0x2A
#define OUT_Y_H_M   0x2B
#define OUT_Z_L_M   0x2C
#define OUT_Z_H_M   0x2D
#define REG_CFG_M   0x30
#define INT_SRC_M   0x31

// Settings
#define ACCELRANGE_2G           0x0 << 3
#define ACCELRANGE_16G          0x1 << 3
#define ACCELRANGE_4G           0x2 << 3
#define ACCELRANGE_8G           0x3 << 3

#define ACCELDATARATE_POWERDOWN 0x0 << 4
#define ACCELDATARATE_3_125HZ   0x1 << 4
#define ACCELDATARATE_6_25HZ    0x2 << 4
#define ACCELDATARATE_12_5HZ    0x3 << 4
#define ACCELDATARATE_25HZ      0x4 << 4
#define ACCELDATARATE_50HZ      0x5 << 4
#define ACCELDATARATE_100HZ     0x6 << 4
#define ACCELDATARATE_200HZ     0x7 << 4
#define ACCELDATARATE_400HZ     0x8 << 4
#define ACCELDATARATE_800HZ     0x9 << 4
#define ACCELDATARATE_1600HZ    0xa << 4

#define MAGGAIN_4GAUSS          0x0 << 5
#define MAGGAIN_8GAUSS          0x1 << 5
#define MAGGAIN_12GAUSS         0x2 << 5
#define MAGGAIN_16GAUSS         0x3 << 5

#define MAGDATARATE_3_125HZ     0x0 << 2
#define MAGDATARATE_6_25HZ      0x1 << 2
#define MAGDATARATE_12_5HZ      0x2 << 2
#define MAGDATARATE_25HZ        0x3 << 2
#define MAGDATARATE_50HZ        0x4 << 2
#define MAGDATARATE_100HZ       0x5 << 2

#define GYROSCALE_245DPS        0x0 << 4
#define GYROSCALE_500DPS        0x1 << 4
#define GYROSCALE_2000DPS       0x2 << 4

/* Conversions */
#define GRAVITY (9.80665F)

#define ACCEL_MG_LSB_2G  (0.061F)
#define ACCEL_MG_LSB_4G  (0.122F)
#define ACCEL_MG_LSB_8G  (0.244F)
#define ACCEL_MG_LSB_16G (0.732F) // Is this right? Was expecting 0.488F

#define MAG_MGAUSS_4GAUSS      (0.16F)
#define MAG_MGAUSS_8GAUSS      (0.32F)
#define MAG_MGAUSS_12GAUSS     (0.48F)
#define MAG_MGAUSS_16GAUSS     (0.58F)

#define GYRO_DPS_DIGIT_245DPS      (0.00875F)
#define GYRO_DPS_DIGIT_500DPS      (0.01750F)
#define GYRO_DPS_DIGIT_2000DPS     (0.07000F)

int lsm9ds1_start(void);
void lsm9ds1_get_accel_adc(int16_t *ax, int16_t *ay, int16_t *az);
void lsm9ds1_get_gyro_adc(int16_t *gx, int16_t *gy, int16_t *gz);

void lsm9ds1_get_accel_data(float *ax, float *ay, float *az);
void lsm9ds1_get_gyro_data(float *gx, float *gy, float *gz);

#endif