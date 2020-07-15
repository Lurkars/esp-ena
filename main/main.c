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
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_log.h"

#include "ena.h"
#include "ena-storage.h"
#include "ena-interface.h"
#include "ena-interface-menu.h"
#include "ssd1306.h"

#include "sdkconfig.h"

void app_main(void)
{
    // DEBUG set time
    struct timeval tv = {1594843200, 0}; // current hardcoded timestamp ¯\_(ツ)_/¯
    settimeofday(&tv, NULL);

    esp_log_level_set(ENA_STORAGE_LOG, ESP_LOG_INFO);

    ssd1306_start(SSD1306_ADDRESS);
    ssd1306_clear(SSD1306_ADDRESS);

    // TODO
    ssd1306_text_line(SSD1306_ADDRESS, "     TODO       TODO", 0, false);
    ssd1306_text_line(SSD1306_ADDRESS, "           TODO", 1, true);
    ssd1306_text_line(SSD1306_ADDRESS, "     TODO       TODO", 2, false);
    ssd1306_text_line(SSD1306_ADDRESS, "           TODO", 3, true);
    ssd1306_text_line(SSD1306_ADDRESS, "     TODO       TODO", 4, false);
    ssd1306_text_line(SSD1306_ADDRESS, "           TODO", 5, true);
    ssd1306_text_line(SSD1306_ADDRESS, "     TODO       TODO", 6, false);
    ssd1306_text_line(SSD1306_ADDRESS, "           TODO", 7, true);

    ena_interface_start();
    ena_interface_menu_start();
    ena_start();
}
