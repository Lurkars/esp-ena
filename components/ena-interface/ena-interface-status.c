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
#include "ena-interface-menu.h"

#include "ena-interface-status.h"

void ena_interface_status_esc(void)
{
    ena_interface_menu_start();
}

void ena_interface_status_start(void)
{
    ena_interface_set_state(ENA_INTERFACE_STATE_STATUS);
    ena_interface_register_touch_callback(TOUCH_PAD_ESC, &ena_interface_status_esc);
    ena_interface_register_touch_callback(TOUCH_PAD_OK, NULL);
    ena_interface_register_touch_callback(TOUCH_PAD_UP, NULL);
    ena_interface_register_touch_callback(TOUCH_PAD_DOWN, NULL);

    ESP_LOGD(ENA_INTERFACE_LOG, "start status interface");
}
