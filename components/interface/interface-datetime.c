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
#include "interface.h"
#include "driver/touch_pad.h"
#include "interface-menu.h"

#include "interface-datetime.h"

static int current_interface_datetime_state = INTERFACE_DATETIME_STATE_YEAR;

const uint32_t interface_datetime_steps[6] = {
    31557600, 2629800, 86400, 3600, 60, 1};

void interface_datetime_esc(void)
{
    interface_menu_start();
}

void interface_datetime_ok(void)
{
    current_interface_datetime_state++;
    if (current_interface_datetime_state > INTERFACE_DATETIME_STATE_SECONDS)
    {
        current_interface_datetime_state = INTERFACE_DATETIME_STATE_YEAR;
    }
    ESP_LOGD(INTERFACE_LOG, "datetime to %d", current_interface_datetime_state);
}

void interface_datetime_up(void)
{
    time_t curtime = time(NULL);
    curtime += interface_datetime_steps[current_interface_datetime_state];
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);
    ESP_LOGD(INTERFACE_LOG, "increment %d about %u %s", current_interface_datetime_state, interface_datetime_steps[current_interface_datetime_state], ctime(&curtime));
}

void interface_datetime_down(void)
{
    time_t curtime = time(NULL);
    curtime -= interface_datetime_steps[current_interface_datetime_state];
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);
    ESP_LOGD(INTERFACE_LOG, "decrement %d about %u %s", current_interface_datetime_state, interface_datetime_steps[current_interface_datetime_state], ctime(&curtime));
}

void interface_datetime_start(void)
{
    interface_set_state(INTERFACE_STATE_SET_DATETIME);
    interface_register_touch_callback(TOUCH_PAD_ESC, &interface_datetime_esc);
    interface_register_touch_callback(TOUCH_PAD_OK, &interface_datetime_ok);
    interface_register_touch_callback(TOUCH_PAD_UP, &interface_datetime_up);
    interface_register_touch_callback(TOUCH_PAD_DOWN, &interface_datetime_down);
    ESP_LOGD(INTERFACE_LOG, "start datetime interface");
}

int interface_datetime_state(void)
{
    return current_interface_datetime_state;
}