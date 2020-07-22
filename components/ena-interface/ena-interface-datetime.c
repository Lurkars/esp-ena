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
#include <sys/time.h>

#include "esp_log.h"
#include "ena-interface.h"
#include "driver/touch_pad.h"
#include "ena-interface-menu.h"

#include "ena-interface-datetime.h"

static int interface_datetime_state = ENA_INTERFACE_DATETIME_STATE_YEAR;

const uint32_t interface_datetime_steps[6] = {
    31557600, 2629800, 86400, 3600, 60, 1};

void ena_interface_datetime_esc(void)
{
    ena_interface_menu_start();
}

void ena_interface_datetime_ok(void)
{
    interface_datetime_state++;
    if (interface_datetime_state > ENA_INTERFACE_DATETIME_STATE_SECONDS)
    {
        interface_datetime_state = ENA_INTERFACE_DATETIME_STATE_YEAR;
    }
    ESP_LOGD(ENA_INTERFACE_LOG, "datetime to %d", interface_datetime_state);
}

void ena_interface_datetime_up(void)
{
    time_t curtime = time(NULL);
    curtime += interface_datetime_steps[interface_datetime_state];
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);
    ESP_LOGD(ENA_INTERFACE_LOG, "increment %d about %u %s", interface_datetime_state, interface_datetime_steps[interface_datetime_state], ctime(&curtime));
}

void ena_interface_datetime_down(void)
{
    time_t curtime = time(NULL);
    curtime -= interface_datetime_steps[interface_datetime_state];
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);
    ESP_LOGD(ENA_INTERFACE_LOG, "decrement %d about %u %s", interface_datetime_state, interface_datetime_steps[interface_datetime_state], ctime(&curtime));
}

void ena_interface_datetime_start(void)
{
    ena_interface_set_state(ENA_INTERFACE_STATE_SET_DATETIME);
    ena_interface_register_touch_callback(TOUCH_PAD_ESC, &ena_interface_datetime_esc);
    ena_interface_register_touch_callback(TOUCH_PAD_OK, &ena_interface_datetime_ok);
    ena_interface_register_touch_callback(TOUCH_PAD_UP, &ena_interface_datetime_up);
    ena_interface_register_touch_callback(TOUCH_PAD_DOWN, &ena_interface_datetime_down);
    ESP_LOGD(ENA_INTERFACE_LOG, "start datetime interface");
}

int ena_interface_datetime_state(void)
{
    return interface_datetime_state;
}