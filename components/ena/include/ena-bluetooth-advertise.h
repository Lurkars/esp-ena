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
#ifndef _ena_BLUETOOTH_ADVERTISE_H_
#define _ena_BLUETOOTH_ADVERTISE_H_

#define ENA_ADVERTISE_LOG "ESP-ENA-advertise" // TAG for Logging
#define ENA_BLUETOOTH_TAG_DATA (0x1A)         // Data for BLE payload TAG

/**
 * @brief       Start BLE advertising
 */
void ena_bluetooth_advertise_start(void);

/**
 * @brief       Set payload for BLE advertising
 * 
 * This will set the payload for based on given ENIN and TEK.
 * 
 * Source documents (Section: Advertising Payload)
 * 
 * https://blog.google/documents/70/Exposure_Notification_-_Bluetooth_Specification_v1.2.2.pdf 
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf
 * 
 * @param[in]   enin    ENIN defining the start of the tek vadility. This should be the ENIN for the current timestamp
 * @param[in]   tek     pointer to the TEK used to encrypt the payload.
 */
void ena_bluetooth_advertise_set_payload(uint32_t enin, uint8_t *tek);

/**
 * @brief       Stop BLE advertising
 */
void ena_bluetooth_advertise_stop(void);

#endif