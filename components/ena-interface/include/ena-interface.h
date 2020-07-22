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
#ifndef _ena_INTERFACE_H_
#define _ena_INTERFACE_H_

#define ENA_INTERFACE_LOG "ESP-ENA-interface" // TAG for Logging

#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
#define TOUCH_PAD_COUNT (4)
#define TOUCH_PAD_ESC (TOUCH_PAD_NUM0)
#define TOUCH_PAD_OK (TOUCH_PAD_NUM6)
#define TOUCH_PAD_UP (TOUCH_PAD_NUM4)
#define TOUCH_PAD_DOWN (TOUCH_PAD_NUM3)

/**
 * @brief different interface states
 */
typedef enum
{
    ENA_INTERFACE_STATE_IDLE = 0,     // ilde state, do nothing
    ENA_INTERFACE_STATE_MENU,         // main menu
    ENA_INTERFACE_STATE_SET_DATETIME, // set current date and time
    ENA_INTERFACE_STATE_STATUS,       // current status
} ena_interface_state;

/**
 * @brief       callback function on touch event
 */
typedef void (*ena_interface_touch_callback)(void);

/**
 * @brief       register a callback function for touch event
 * 
 * @param[in]   touch_pad   id of the touchpad to listen touch
 * @param[in]   callback    callback function
 */
void ena_interface_register_touch_callback(int touch_pad, ena_interface_touch_callback callback);

/**
 * @brief       get current interface state
 * 
 * @return
 *              current state the interface is in
 */
int ena_interface_get_state(void);

/**
 * @brief       set current interface state
 * 
 * @param[in]   state   new state to set
 */
void ena_interface_set_state(ena_interface_state state);

/**
 * @brief       start interface logic
 * 
 * This will initialize the touch controls and start a task to listen to touch
 * inputs and calling the callbacks
 */
void ena_interface_start(void);

#endif