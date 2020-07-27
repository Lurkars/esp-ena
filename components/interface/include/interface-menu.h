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
/**
 * @file
 * 
 * @brief interface menu to navigate through interface
 *  
 */
#ifndef _interface__MENU_H_
#define _interface__MENU_H_

typedef enum
{
    INTERFACE_MENU_STATE_IDLE = 0,
    INTERFACE_MENU_STATE_SELECT_TIME,
    INTERFACE_MENU_STATE_SELECT_DEBUG,
    INTERFACE_MENU_STATE_SELECT_STATUS,
} interface_menu_state_t;

void interface_menu_start(void);

int interface_menu_get_state(void);

#endif