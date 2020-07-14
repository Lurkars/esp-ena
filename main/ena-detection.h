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

#ifndef _ena_DETECTION_H_
#define _ena_DETECTION_H_

#define ENA_DETECTION_LOG "ESP-ENA-detection" // TAG for Logging

#define ENA_DETECTION_TRESHOLD (300) // meet for longer than 5 minutes

/**
 * @brief       check temporary detection for full detection or expiring
 * 
 * This function checks all current temporary detections if the contact threshold is
 * reached or if the temporary contact can be discarded.
 * 
 * @param[in]   unix_timestamp  current time as UNIX timestamp to compate
 * 
 */
void ena_detections_temp_refresh(uint32_t unix_timestamp);

/**
 * @brief       handle new detection received from a BLE scan
 * 
 * This function gets called when a running BLE scan received a new ENA payload. 
 * On already detected RPI this will update just the timestamp and RSSI.
 * 
 * @param[in]   unix_timestamp  UNIX timestamp when detection was made
 * @param[in]   rpi             received RPI from scanned payload
 * @param[in]   aem             received AEM from scanned payload
 * @param[in]   rssi            measured RSSI on scan
 * 
 */
void ena_detection(uint32_t unix_timestamp, uint8_t *rpi, uint8_t *aem, int rssi);

#endif