#ifndef DISPLAY_ST7789_H_
#define DISPLAY_ST7789_H_

#include "driver/spi_master.h"

// M5stickC PLUS
#define M5_ST7789_WIDTH 240
#define M5_ST7789_HEIGHT 135
#define M5_ST7789_MOSI_GPIO 15
#define M5_ST7789_SCLK_GPIO 13
#define M5_ST7789_CS_GPIO 5
#define M5_ST7789_DC_GPIO 23
#define M5_ST7789_RESET_GPIO 18
#define M5_ST7789_BL_GPIO 32
#define M5_ST7789_OFFSETX 40
#define M5_ST7789_OFFSETY 52

#define M5_ST7789_LANDSCAPE 0x60
#define M5_ST7789_LANDSCAPE_FLIPPED 0xA0

#define M5_ST7789_INTERFACE_OFFSETX 56
#define M5_ST7789_INTERFACE_OFFSETY 35

#define SPI_COMMAND_MODE 0
#define SPI_DATA_MODE 1




#endif