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

#ifndef _ena_INTERFACE_MENU_H_
#define _ena_INTERFACE_MENU_H_

typedef enum
{
    ENA_INTERFACE_MENU_STATE_IDLE = 0,
    ENA_INTERFACE_MENU_STATE_SELECT_TIME,
    ENA_INTERFACE_MENU_STATE_SELECT_INFO,
} ena_interface_menu_state;

void ena_interface_menu_start(void);

int ena_interface_menu_get_state(void);

#endif