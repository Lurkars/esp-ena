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
#include "ena-interface.h"
#include "driver/touch_pad.h"
#include "ena-interface-menu.h"

#include "ena-interface-datetime.h"

static int interface_datetime_state = ENA_INTERFACE_DATETIME_STATE_YEAR;

void ena_interface_datetime_esc(void)
{
    ena_interface_menu_start();
}

void ena_interface_datetime_start(void)
{
    ena_interface_set_state(ENA_INTERFACE_STATE_SET_YEAR);
    ena_interface_register_touch_callback(TOUCH_PAD_ESC, &ena_interface_datetime_esc);
    ESP_LOGD(ENA_INTERFACE_LOG, "start datetime interface");
}

int ena_interface_datetime_state(void)
{
    return interface_datetime_state;
}