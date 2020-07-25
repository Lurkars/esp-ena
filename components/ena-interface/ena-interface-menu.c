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
#include "ena-interface.h"
#include "ena-interface-datetime.h"
#include "ena-interface-status.h"

#include "ena-interface-menu.h"

static int interface_menu_state = ENA_INTERFACE_MENU_STATE_IDLE;

void ena_interface_menu_ok(void)
{
    if (interface_menu_state == ENA_INTERFACE_MENU_STATE_SELECT_TIME)
    {
        ena_interface_datetime_start();
    }
    else if (interface_menu_state == ENA_INTERFACE_MENU_STATE_SELECT_STATUS)
    {
        ena_interface_status_start();
    }
    else if (interface_menu_state == ENA_INTERFACE_MENU_STATE_SELECT_DEBUG)
    {
    }
    else if (interface_menu_state == ENA_INTERFACE_MENU_STATE_IDLE)
    {
        if (ena_interface_get_state() == ENA_INTERFACE_STATE_MENU)
        {
            ena_interface_set_state(ENA_INTERFACE_STATE_IDLE);
            ena_interface_register_touch_callback(TOUCH_PAD_UP, NULL);
            ena_interface_register_touch_callback(TOUCH_PAD_DOWN, NULL);
        }
        else
        {
            ena_interface_menu_start();
        }
    }
}

void ena_interface_menu_up(void)
{
    interface_menu_state--;
    if (interface_menu_state < ENA_INTERFACE_MENU_STATE_IDLE)
    {
        interface_menu_state = ENA_INTERFACE_MENU_STATE_SELECT_STATUS;
    }
    ESP_LOGD(ENA_INTERFACE_LOG, "menu up to %d", interface_menu_state);
}

void ena_interface_menu_down(void)
{
    interface_menu_state++;
    if (interface_menu_state > ENA_INTERFACE_MENU_STATE_SELECT_STATUS)
    {
        interface_menu_state = ENA_INTERFACE_MENU_STATE_IDLE;
    }
    ESP_LOGD(ENA_INTERFACE_LOG, "menu down to %d", interface_menu_state);
}

void ena_interface_menu_start(void)
{
    ena_interface_set_state(ENA_INTERFACE_STATE_MENU);
    ena_interface_register_touch_callback(TOUCH_PAD_ESC, NULL);
    ena_interface_register_touch_callback(TOUCH_PAD_OK, &ena_interface_menu_ok);
    ena_interface_register_touch_callback(TOUCH_PAD_UP, &ena_interface_menu_up);
    ena_interface_register_touch_callback(TOUCH_PAD_DOWN, &ena_interface_menu_down);

    ESP_LOGD(ENA_INTERFACE_LOG, "start menu interface");
}

int ena_interface_menu_get_state(void)
{
    return interface_menu_state;
}