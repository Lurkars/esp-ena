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

#ifndef _ena_STORAGE_H_
#define _ena_STORAGE_H_

#include "ena-crypto.h"
#include "ena-datastructures.h"

#define ENA_STORAGE_LOG "ESP-ENA-storage"      // TAG for Logging
#define PARTITION_NAME "ena"                   // name of partition to use for storing
#define ENA_STORAGE_TEK_STORE_PERIOD (14)      // Period of storing TEKs                                                                            // length of a stored detection -> RPI keysize + AEM size + 4 Bytes for ENIN + 4 Bytes for RSSI
#define ENA_STORAGE_TEMP_DETECTIONS_MAX (1000) // Maximum number of temporary stored detections

/**
 * @brief       read bytes at given address
 * 
 * @param[in]   address     the address to read bytes from
 * @param[out]  data        pointer to write the read data
 * @param[in]   size        how many bytes to read
 */
void ena_storage_read(size_t address, void *data, size_t size);

/**
 * @brief       store bytes at given address
 * 
 * @param[in]   address     the address to write bytes to
 * @param[in]   data        pointer to the data to write
 * @param[in]   size        how many bytes to write
 */
void ena_storage_write(size_t address, void *data, size_t size);

/**
 * @brief       deletes bytes at given address and shift other data back
 * 
 * @param[in] address       the address to delete from
 * @param[in] end_address   the address to mark end of shift
 * @param[in] size          how many bytes to delete
 */
void ena_storage_shift_delete(size_t address, size_t end_address, size_t size);

/**
 * @brief       get last stored TEK
 * 
 * @param[out]  tek         pointer to write last TEK to
 * 
 * @return 
 *              total number of TEKs stored
 */
uint32_t ena_storage_read_last_tek(ena_tek_t *tek);

/**
 * @brief       store given TEK
 * 
 * This will store the given TEK as new TEK.
 * 
 * @param[in]   tek         the tek to store
 */
void ena_storage_write_tek(ena_tek_t *tek);

/**
 * @brief       get number of stored temporary detections
 * 
 * @return
 *              total number of temporary detections stored
 */
uint32_t ena_storage_temp_detections_count(void);

/**
 * @brief       get temporary detection at given index
 * 
 * @param[in]   index       the index of the temporary detection to read
 * @param[out]  detection   pointer to temporary to write to
 */
void ena_storage_get_temp_detection(uint32_t index, ena_temp_detection_t *detection);

/**
 * @brief       store temporary detection
 * 
 * @param[in]   detection   new temporary detection to store 
 * 
 * @return 
 *              index of new stored detection
 */
uint32_t ena_storage_add_temp_detection(ena_temp_detection_t *detection);

/**
 * @brief       store temporary detection at given index
 * 
 * @param[in]   index       the index of the temporary detection to overwrite
 * @param[in]   detection   temporary detection to store 
 */
void ena_storage_set_temp_detection(uint32_t index, ena_temp_detection_t *detection);

/**
 * @brief       remove temporary detection at given index
 * 
 * @param[in]   index       the index of the temporary detection to remove
 */
void ena_storage_remove_temp_detection(uint32_t index);

/**
 * @brief       get number of stored detections
 * 
 * @return
 *              total number of detections stored
 */
uint32_t ena_storage_detections_count(void);

/**
 * @brief       get detection at given index
 * 
 * @param[in]   index       the index of the detection to read
 * @param[out] detection    pointer to to write to
 */
void ena_storage_get_detection(uint32_t index, ena_detection_t *detection);

/**
 * @brief       store detection
 * 
 * @param[in]   detection   new detection to store 
 */
void ena_storage_add_detection(ena_detection_t *detection);

/**
 * @brief       erase the storage
 * 
 * This function completely deletes all stored data and resets the counters 
 * of TEKs, temporary detection and detection to zero.
 */
void ena_storage_erase(void);

/**
 * @brief       erase stored TEKs
 * 
 * This function deletes all stored TEKs and resets counter to zero.
 */
void ena_storage_erase_tek(void);

/**
 * @brief       erase stored temporary detections
 * 
 * This function deletes all stored temporary detections and resets counter to zero.
 */
void ena_storage_erase_temporary_detection(void);

/**
 * @brief       erase stored detections
 * 
 * This function deletes all stored detections and resets counter to zero.
 */
void ena_storage_erase_detection(void);

/**
 * @brief       dump all stored TEKs to serial output
 * 
 * This function prints all stored TEKs to serial output in
 * the following CSV format: #,enin,tek
 */
void ena_storage_dump_tek(void);

/**
 * @brief       dump all stored temporary detections to serial output
 * 
 * This function prints all stored temporary detections to serial output in
 * the following CSV format: #,timestamp_first,timestamp_last,rpi,aem,rssi
 */
void ena_storage_dump_temp_detections(void);

/**
 * @brief       dump all stored detections to serial output
 * 
 * This function prints all stored detections to serial output in
 * the following CSV format: #,timestamp,rpi,aem,rssi
 */
void ena_storage_dump_detections(void);

#endif