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
#ifndef _wifi_CONTROLLER_H_
#define _wifi_CONTROLLER_H_

#include "esp_err.h"
#include "esp_wifi_types.h"

#define WIFI_LOG "wifi-controller" // TAG for Logging

/**
 * @brief scan for WiFis
 * 
 * @param[out] ap_info  scanned APs
 * @param[out] ap_count    number of scanned APs
 */
void wifi_controller_scan(wifi_ap_record_t ap_info[], uint16_t *ap_count);

/**
 * @brief connect to wifi ap
 * 
 * @param[in] wifi_config   config of wifi to connect
 * 
 * @return
 *          esp_err_t connection status
 */
esp_err_t wifi_controller_connect(wifi_config_t wifi_config);

/**
 * @brief reconnect to previous wifi
 *  
 * @return
 *          esp_err_t connection status
 */
esp_err_t wifi_controller_reconnect(void);

/**
 * @brief reconnect to previous wifi
 *  
 * @return
 *          wifi_ap_record_t pointer to current wifi connection, NULL if not connected
 */
wifi_ap_record_t *wifi_controller_connection(void);

#endif