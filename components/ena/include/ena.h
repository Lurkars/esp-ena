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
#ifndef _ena_H_
#define _ena_H_

#define ENA_LOG "ESP-ENA"                                                                              // TAG for Logging
#define ENA_RAM (CONFIG_ENA_RAM)                                                                       // change advertising payload and therefore the BT address
#define ENA_BT_ROTATION_TIMEOUT_INTERVAL (CONFIG_ENA_BT_ROTATION_TIMEOUT_INTERVAL)                     // change advertising payload and therefore the BT address
#define ENA_BT_RANDOMIZE_ROTATION_TIMEOUT_INTERVAL (CONFIG_ENA_BT_RANDOMIZE_ROTATION_TIMEOUT_INTERVAL) // random intervall change for BT address change

/**
 * @brief       Start Exposure Notification API
 * 
 * This initializes the complete stack of ESP_ENA. It will initialize BLE module and 
 * starting a task for managing advertising and scanning processes.
 * 
 */
void ena_start(void);

#endif