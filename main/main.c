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
#include "ena-detection.h"
#include "ena-storage.h"
#include "ena-interface.h"
#include "ena-interface-menu.h"

#include "sdkconfig.h"

void app_main(void)
{
    // DEBUG set time
    struct timeval tv = {1594459800, 0}; // current hardcoded timestamp (2020-07-11 09:30:00) ¯\_(ツ)_/¯
    settimeofday(&tv, NULL);

    esp_log_level_set(ENA_STORAGE_LOG, ESP_LOG_INFO);
    // ena_storage_erase(); // only needed on first start! TODO automatically check (how?)

    ena_interface_start();
    ena_interface_menu_start();
    ena_start();
}
