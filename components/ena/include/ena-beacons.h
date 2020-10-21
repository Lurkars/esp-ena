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
 * @brief handles scanned data by storing temporary beacons, check for threshold and store beacons permanently
 * 
 */
#ifndef _ena_BEACON_H_
#define _ena_BEACON_H_

#define ENA_BEACON_LOG "ESP-ENA-beacon"                                  // TAG for Logging
#define ENA_BEACON_TRESHOLD (CONFIG_ENA_BEACON_TRESHOLD)                 // meet for longer than 5 minutes
#define ENA_BEACON_CLEANUP_TRESHOLD (CONFIG_ENA_BEACON_CLEANUP_TRESHOLD) // threshold (in days) for stored beacons to be removed

/**
 * @brief       check temporary beacon for threshold or expiring
 * 
 * This function checks all current temporary beacons if the contact threshold is
 * reached or if the temporary contact can be discarded.
 * 
 * @param[in]   unix_timestamp  current time as UNIX timestamp to compare
 * 
 */
void ena_beacons_temp_refresh(uint32_t unix_timestamp);

/**
 * @brief       check stored beacons to expire
 * 
 * This function checks for all stored beacons if the last timestamp is over a threshold to remove the beacon.
 * 
 * @param[in]   unix_timestamp  current time as UNIX timestamp to compate
 * 
 */
void ena_beacons_cleanup(uint32_t unix_timestamp);

/**
 * @brief       handle new beacon received from a BLE scan
 * 
 * This function gets called when a running BLE scan received a new ENA payload. 
 * On already detected RPI this will update just the timestamp and RSSI.
 * 
 * @param[in]   unix_timestamp  UNIX timestamp when beacon was made
 * @param[in]   rpi             received RPI from scanned payload
 * @param[in]   aem             received AEM from scanned payload
 * @param[in]   rssi            measured RSSI on scan
 * 
 */
void ena_beacon(uint32_t unix_timestamp, uint8_t *rpi, uint8_t *aem, int rssi);

#endif