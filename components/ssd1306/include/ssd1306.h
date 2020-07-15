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
#ifndef _ssd1306_H_
#define _ssd1306_H_

#define SSD1306_ADDRESS (0x3C)
#define SSD1306_COLUMNS (128)
#define SSD1306_PAGES (8)

// Write mode for I2C https://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html
#define SSD1306_CONTROL_CMD_BYTE (0x80)
#define SSD1306_CONTROL_CMD_STREAM (0x00)
#define SSD1306_CONTROL_DATA_BYTE (0xC0)
#define SSD1306_CONTROL_DATA_STREAM (0x40)

// 1. Fundamental Command Table
#define SSD1306_CMD_CONTRAST (0x81)
#define SSD1306_CMD_RAM (0xA4)
#define SSD1306_CMD_ALL_ON (0xA5)
#define SSD1306_CMD_NORMAL (0xA6)
#define SSD1306_CMD_INVERSE (0xA7)
#define SSD1306_CMD_OFF (0xAE)
#define SSD1306_CMD_ON (0xAF)

// 2. Scrolling Command Table
#define SSD1306_CMD_SCROLL_HORI_RIGHT (0x26)
#define SSD1306_CMD_SCROLL_HORI_LEFT (0x27)
#define SSD1306_CMD_SCROLL_VERT_RIGHT (0x29)
#define SSD1306_CMD_SCROLL_VERT_LEFT (0x2A)
#define SSD1306_CMD_SCROLL_STOP (0x2E)
#define SSD1306_CMD_SCROLL_START (0x2F)
#define SSD1306_CMD_SCROLL_VERT_AREA (0xA3)

// 3. Addressing Setting Command Table
#define SSD1306_CMD_COLUMN_LOW (0x00)
#define SSD1306_CMD_COLUMN_HIGH (0x10)
#define SSD1306_CMD_MEMORY_MODE (0x20)
#define SSD1306_CMD_COLUMN_ADDRESS (0x21)
#define SSD1306_CMD_PAGE_ADDRESS (0x22)
#define SSD1306_CMD_PAGE (0xB0)

// 4. Hardware Configuration (Panel resolution & layout related) Command Table
#define SSD1306_CMD_START_LINE (0x40)
#define SSD1306_CMD_SEGMENT_LOW (0xA0)
#define SSD1306_CMD_SEGMENT_HIGH (0xA1)
#define SSD1306_CMD_MULTIPLEX_RATIO (0xA8)
#define SSD1306_CMD_SCAN_DIRECTION_NORMAL (0xC0)
#define SSD1306_CMD_SCAN_DIRECTION_REMAPPED (0xC8)
#define SSD1306_CMD_OFFSET (0xD3)
#define SSD1306_CMD_COM_PINS (0xDA)

// 5. Timing & Driving Scheme Setting Command Table
#define SSD1306_CMD_CLOCK (0xD5)
#define SSD1306_CMD_PRE_CHARGE_PERIOD (0xD9)
#define SSD1306_CMD_VCOMH (0xDB)
#define SSD1306_CMD_NOP (0xE3)

// 1. Charge Pump Command Table
#define SSD1306_CMD_CHARGE_PUMP (0x8D)

/**
 * @brief initalize SSD1306 with I2C at given address
 * 
 * @param[in] i2address I2C address of SSD1306
 */
void ssd1306_start(uint8_t i2address);

/**
 * @brief clear the display
 * 
 * @param[in] i2address I2C address of SSD1306 
 * @param[in] line the line to clear
 */
void ssd1306_clear_line(uint8_t i2address, uint8_t line, bool invert);

/**
 * @brief clear the display
 * 
 * @param[in] i2address I2C address of SSD1306 
 */
void ssd1306_clear(uint8_t i2address);

/**
 * @brief write text to display
 * 
 * @param[in] i2address I2C address of SSD1306
 * @param[in] text text to display  
 * @param[in] line the line to write to
 */
void ssd1306_text_line(uint8_t i2address, char *text, uint8_t line, bool invert);

#endif