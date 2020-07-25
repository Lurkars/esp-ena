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
#ifndef _wifi_H_
#define _wifi_H_

#define WIFI_LOG "ESP-ENA-wifi" // TAG for Logging

#define WIFI_SSID (CONFIG_WIFI_SSID)
#define WIFI_PASSWORD (CONFIG_WIFI_PASSWORD)
#define WIFI_MAXIMUM_RETRY (CONFIG_WIFI_MAXIMUM_RETRY)

/**
 * @brief start wifi connection to configured AP
 */
void wifi_start(void);

/**
 * @brief stop wifi (restart does not work for now!)
 */
void wifi_stop(void);

/**
 * @brief check if a wifi is connected
 */
bool wifi_is_connected(void);

#endif