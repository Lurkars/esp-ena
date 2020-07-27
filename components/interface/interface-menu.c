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

#include "esp_log.h"
#include "driver/touch_pad.h"
#include "interface.h"
#include "interface-datetime.h"
#include "interface-status.h"

#include "interface-menu.h"

static int current_interface_menu_state = INTERFACE_MENU_STATE_IDLE;

void interface_menu_ok(void)
{
    if (current_interface_menu_state == INTERFACE_MENU_STATE_SELECT_TIME)
    {
        interface_datetime_start();
    }
    else if (current_interface_menu_state == INTERFACE_MENU_STATE_SELECT_STATUS)
    {
        interface_status_start();
    }
    else if (current_interface_menu_state == INTERFACE_MENU_STATE_SELECT_DEBUG)
    {
    }
    else if (current_interface_menu_state == INTERFACE_MENU_STATE_IDLE)
    {
        if (interface_get_state() == INTERFACE_STATE_MENU)
        {
            interface_set_state(INTERFACE_STATE_IDLE);
            interface_register_touch_callback(TOUCH_PAD_UP, NULL);
            interface_register_touch_callback(TOUCH_PAD_DOWN, NULL);
        }
        else
        {
            interface_menu_start();
        }
    }
}

void interface_menu_up(void)
{
    current_interface_menu_state--;
    if (current_interface_menu_state < INTERFACE_MENU_STATE_IDLE)
    {
        current_interface_menu_state = INTERFACE_MENU_STATE_SELECT_STATUS;
    }
    ESP_LOGD(INTERFACE_LOG, "menu up to %d", current_interface_menu_state);
}

void interface_menu_down(void)
{
    current_interface_menu_state++;
    if (current_interface_menu_state > INTERFACE_MENU_STATE_SELECT_STATUS)
    {
        current_interface_menu_state = INTERFACE_MENU_STATE_IDLE;
    }
    ESP_LOGD(INTERFACE_LOG, "menu down to %d", current_interface_menu_state);
}

void interface_menu_start(void)
{
    interface_set_state(INTERFACE_STATE_MENU);
    interface_register_touch_callback(TOUCH_PAD_ESC, NULL);
    interface_register_touch_callback(TOUCH_PAD_OK, &interface_menu_ok);
    interface_register_touch_callback(TOUCH_PAD_UP, &interface_menu_up);
    interface_register_touch_callback(TOUCH_PAD_DOWN, &interface_menu_down);

    ESP_LOGD(INTERFACE_LOG, "start menu interface");
}

int interface_menu_get_state(void)
{
    return current_interface_menu_state;
}