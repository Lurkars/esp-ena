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
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"

#include "display.h"
#include "display-gfx.h"

#include "axp192.h"

#include "st7789.h"

static spi_device_handle_t st7789_handle;

bool spi_master_write(uint8_t *data, size_t len, uint8_t dc)
{
	spi_transaction_t spi_trans;

	gpio_set_level(M5_ST7789_DC_GPIO, dc);

	memset(&spi_trans, 0, sizeof(spi_transaction_t));
	spi_trans.length = len * 8;
	spi_trans.tx_buffer = data;
	spi_device_transmit(st7789_handle, &spi_trans);

	return true;
}

bool spi_master_write_command(uint8_t cmd)
{
	return spi_master_write(&cmd, 1, SPI_COMMAND_MODE);
}

bool spi_master_write_data_byte(uint8_t data)
{
	return spi_master_write(&data, 1, SPI_DATA_MODE);
}

bool spi_master_write_data(uint8_t *data, size_t len)
{
	return spi_master_write(data, len, SPI_DATA_MODE);
}

bool spi_master_write_addr(uint16_t addr1, uint16_t addr2)
{
	uint8_t data[4];
	data[0] = (addr1 >> 8) & 0xFF;
	data[1] = addr1 & 0xFF;
	data[2] = (addr2 >> 8) & 0xFF;
	data[3] = addr2 & 0xFF;

	return spi_master_write_data(data, 4);
}

bool spi_master_write_color(uint16_t color, size_t size)
{
	uint8_t data[size * 2];
	int index = 0;

	uint8_t msbColor = (color >> 8) & 0xFF;
	uint8_t lsbColor = color & 0xFF;

	for (int i = 0; i < size; i++)
	{
		data[index++] = msbColor;
		data[index++] = lsbColor;
	}
	return spi_master_write_data(data, size * 2);
}

bool spi_master_write_colors(uint16_t *colors, size_t size)
{
	uint8_t data[size * 2];
	int index = 0;
	for (int i = 0; i < size; i++)
	{
		data[index++] = (colors[i] >> 8) & 0xFF;
		data[index++] = colors[i] & 0xFF;
	}
	return spi_master_write_data(data, size * 2);
}

void display_start(void)
{

	axp192_start();
	axp192_screen_breath(0);

	esp_err_t ret;

	gpio_pad_select_gpio(M5_ST7789_CS_GPIO);
	gpio_set_direction(M5_ST7789_CS_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(M5_ST7789_CS_GPIO, 0);

	gpio_pad_select_gpio(M5_ST7789_DC_GPIO);
	gpio_set_direction(M5_ST7789_DC_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(M5_ST7789_DC_GPIO, 0);

	gpio_pad_select_gpio(M5_ST7789_RESET_GPIO);
	gpio_set_direction(M5_ST7789_RESET_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(M5_ST7789_RESET_GPIO, 1);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	gpio_set_level(M5_ST7789_RESET_GPIO, 0);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	gpio_set_level(M5_ST7789_RESET_GPIO, 1);
	vTaskDelay(50 / portTICK_PERIOD_MS);

	gpio_pad_select_gpio(M5_ST7789_BL_GPIO);
	gpio_set_direction(M5_ST7789_BL_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(M5_ST7789_BL_GPIO, 0);

	spi_bus_config_t buscfg = {
			.sclk_io_num = M5_ST7789_SCLK_GPIO,
			.mosi_io_num = M5_ST7789_MOSI_GPIO,
			.miso_io_num = -1,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1};

	ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
	assert(ret == ESP_OK);

	spi_device_interface_config_t devcfg = {
			.clock_speed_hz = SPI_MASTER_FREQ_20M,
			.queue_size = 7,
			.mode = 2,
			.flags = SPI_DEVICE_NO_DUMMY,
	};

	devcfg.spics_io_num = M5_ST7789_CS_GPIO;

	ret = spi_bus_add_device(HSPI_HOST, &devcfg, &st7789_handle);
	assert(ret == ESP_OK);

	spi_master_write_command(0x01); //Software Reset
	vTaskDelay(150 / portTICK_PERIOD_MS);

	spi_master_write_command(0x11); //Sleep Out
	vTaskDelay(255 / portTICK_PERIOD_MS);

	spi_master_write_command(0x3A); //Interface Pixel Format
	spi_master_write_data_byte(0x55);
	vTaskDelay(10 / portTICK_PERIOD_MS);

	// landscape RGB
	spi_master_write_command(0x36); //Memory Data Access Control
	spi_master_write_data_byte(M5_ST7789_LANDSCAPE);

	spi_master_write_command(0x2A); //Column Address Set
	spi_master_write_data_byte(0x00);
	spi_master_write_data_byte(0x00);
	spi_master_write_data_byte(0x00);
	spi_master_write_data_byte(0xF0);

	spi_master_write_command(0x2B); //Row Address Set
	spi_master_write_data_byte(0x00);
	spi_master_write_data_byte(0x00);
	spi_master_write_data_byte(0x00);
	spi_master_write_data_byte(0xF0);

	spi_master_write_command(0x21); //Display Inversion On
	vTaskDelay(10 / portTICK_PERIOD_MS);

	spi_master_write_command(0x13); //Normal Display Mode On
	vTaskDelay(10 / portTICK_PERIOD_MS);

	spi_master_write_command(0x29); //Display ON
	vTaskDelay(255 / portTICK_PERIOD_MS);

	gpio_set_level(M5_ST7789_BL_GPIO, 1);

	axp192_screen_breath(10);
}

void display_flipped(bool flipped)
{
	spi_master_write_command(0x36); //Memory Data Access Control
	if (flipped)
	{
		spi_master_write_data_byte(M5_ST7789_LANDSCAPE_FLIPPED);
	}
	else
	{
		spi_master_write_data_byte(M5_ST7789_LANDSCAPE);
	}
}

void display_clear_line(uint8_t line, bool invert)
{
	uint16_t _x1 = 0 + M5_ST7789_OFFSETX;
	uint16_t _x2 = M5_ST7789_WIDTH + M5_ST7789_OFFSETX - 1;
	uint16_t _y1 = line * 8 + M5_ST7789_OFFSETY + M5_ST7789_INTERFACE_OFFSETY;
	uint16_t _y2 = line * 8 + 8 + M5_ST7789_OFFSETY - 1 + M5_ST7789_INTERFACE_OFFSETY;

	spi_master_write_command(0x2A); // set column(x) address
	spi_master_write_addr(_x1, _x2);
	spi_master_write_command(0x2B); // set Page(y) address
	spi_master_write_addr(_y1, _y2);
	spi_master_write_command(0x2C); //	Memory Write
	for (int i = _x1; i <= _x2; i++)
	{
		uint16_t size = _y2 - _y1 + 1;
		spi_master_write_color(invert ? display_get_color() : BLACK, size);
	}
}

void display_clear(void)
{
	uint16_t _x1 = 0 + M5_ST7789_OFFSETX;
	uint16_t _x2 = M5_ST7789_WIDTH + M5_ST7789_OFFSETX - 1;
	uint16_t _y1 = 0 + M5_ST7789_OFFSETY;
	uint16_t _y2 = M5_ST7789_HEIGHT + M5_ST7789_OFFSETY - 1;

	spi_master_write_command(0x2A); // set column(x) address
	spi_master_write_addr(_x1, _x2);
	spi_master_write_command(0x2B); // set Page(y) address
	spi_master_write_addr(_y1, _y2);
	spi_master_write_command(0x2C); //	Memory Write
	for (int i = _x1; i <= _x2; i++)
	{
		uint16_t size = _y2 - _y1 + 1;
		spi_master_write_color(BLACK, size);
	}
}

void display_on(bool on)
{
	axp192_screen_breath(on ? 10 : 0);
}

void display_data(uint8_t *data, size_t length, uint8_t line, uint8_t offset, bool invert)
{
	uint16_t _x1 = offset + M5_ST7789_OFFSETX + M5_ST7789_INTERFACE_OFFSETX;
	uint16_t _x2 = offset + length + M5_ST7789_OFFSETX - 1 + M5_ST7789_INTERFACE_OFFSETX;
	uint16_t _y1 = line * 8 + M5_ST7789_OFFSETY + M5_ST7789_INTERFACE_OFFSETY;
	uint16_t _y2 = line * 8 + 8 + M5_ST7789_OFFSETY - 1 + M5_ST7789_INTERFACE_OFFSETY;

	spi_master_write_command(0x2A); // set column(x) address
	spi_master_write_addr(_x1, _x2);
	spi_master_write_command(0x2B); // set Page(y) address
	spi_master_write_addr(_y1, _y2);
	spi_master_write_command(0x2C); //Memory Write

	uint8_t msbColor = (display_get_color() >> 8) & 0xFF;
	uint8_t lsbColor = display_get_color() & 0xFF;

	for (int j = 0; j < 8; j++)
	{
		uint8_t color[length * 2];
		int index = 0;

		for (int i = 0; i < length; i++)
		{
			bool bit = (data[i] & (1 << j));
			if (invert)
			{
				bit = !bit;
			}
			color[index++] = bit ? msbColor : 0x00;
			color[index++] = bit ? lsbColor : 0x00;
		}
		spi_master_write_data(color, length * 2);
	}
}
