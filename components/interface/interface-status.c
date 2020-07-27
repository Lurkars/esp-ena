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
#include "interface-menu.h"

#include "interface-status.h"

void interface_status_esc(void)
{
    interface_menu_start();
}

void interface_status_start(void)
{
    interface_set_state(INTERFACE_STATE_STATUS);
    interface_register_touch_callback(TOUCH_PAD_ESC, &interface_status_esc);
    interface_register_touch_callback(TOUCH_PAD_OK, NULL);
    interface_register_touch_callback(TOUCH_PAD_UP, NULL);
    interface_register_touch_callback(TOUCH_PAD_DOWN, NULL);

    ESP_LOGD(INTERFACE_LOG, "start status interface");
}
