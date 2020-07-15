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
#ifndef _ena_BLUETOOTH_SCAN_H_
#define _ena_BLUETOOTH_SCAN_H_

#define ENA_SCAN_LOG "ESP-ENA-scan"                          // TAG for Logging
#define ENA_SCANNING_TIME (CONFIG_ENA_SCANNING_TIME)         // time how long a scan should run
#define ENA_SCANNING_INTERVAL (CONFIG_ENA_SCANNING_INTERVAL) // interval for next scan to happen

/**
 * @brief status of BLE scan
 */
typedef enum
{
    ENA_SCAN_STATUS_SCANNING = 0, // scan is running
    ENA_SCAN_STATUS_NOT_SCANNING, // scan is not running
    ENA_SCAN_STATUS_WAITING,      // scan is not running but stopped manually
} ena_bluetooth_scan_status;

/**
 * @brief       initialize the BLE scanning
 * 
 */
void ena_bluetooth_scan_init(void);

/**
 * @brief       start BLE scanning for a given duration
 * 
 * Source documents (Section: Scanning Behavior)
 * 
 * https://blog.google/documents/70/Exposure_Notification_-_Bluetooth_Specification_v1.2.2.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf
 * 
 * @param[in]   duration    duration of the scan in seconds
 */
void ena_bluetooth_scan_start(uint32_t duration);

/**
 * @brief       stop a running BLE scanning
 */
void ena_bluetooth_scan_stop(void);

/**
 * @brief       return the current scanning status
 * 
 * @return
 *              current scan status
 */
int ena_bluetooth_scan_get_status(void);

#endif