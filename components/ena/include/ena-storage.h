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

#define ENA_STORAGE_LOG "ESP-ENA-storage"                                  // TAG for Logging
#define ENA_STORAGE_PARTITION_NAME (CONFIG_ENA_STORAGE_PARTITION_NAME)     // name of partition to use for storing
#define ENA_STORAGE_START_ADDRESS (CONFIG_ENA_STORAGE_START_ADDRESS)      // start address of storage
#define ENA_STORAGE_TEK_MAX (CONFIG_ENA_STORAGE_TEK_MAX)                   // Period of storing TEKs                                                                            // length of a stored beacon -> RPI keysize + AEM size + 4 Bytes for ENIN + 4 Bytes for RSSI
#define ENA_STORAGE_TEMP_BEACONS_MAX (CONFIG_ENA_STORAGE_TEMP_BEACONS_MAX) // Maximum number of temporary stored beacons

/**
 * @brief structure for TEK
 */
typedef struct __attribute__((__packed__))
{
    uint8_t key_data[ENA_KEY_LENGTH]; // key data for encryption
    uint32_t enin;                    // ENIN marking start of validity
    uint8_t rolling_period;           // period after validity start to mark key as expired
} ena_tek_t;

/**
 * @brief sturcture for storing a temporary beacon
 */
typedef struct __attribute__((__packed__))
{
    uint8_t rpi[ENA_KEY_LENGTH];          // received RPI of beacon
    uint8_t aem[ENA_AEM_METADATA_LENGTH]; // received AEM of beacon
    uint32_t timestamp_first;             // timestamp of first recognition
    uint32_t timestamp_last;              // timestamp of last recognition
    int rssi;                             // average measured RSSI
} ena_temp_beacon_t;

/**
 * @brief sturcture for permanently storing a beacon after threshold reached
 */
typedef struct __attribute__((__packed__))
{
    uint8_t rpi[ENA_KEY_LENGTH];          // received RPI of beacon
    uint8_t aem[ENA_AEM_METADATA_LENGTH]; // received AEM of beacon
    uint32_t timestamp;                   // timestamp of last recognition
    int rssi;                             // average measured RSSI
} ena_beacon_t;

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
 * @brief       get number of stored temporary beacons
 * 
 * @return
 *              total number of temporary beacons stored
 */
uint32_t ena_storage_temp_beacons_count(void);

/**
 * @brief       get temporary beacon at given index
 * 
 * @param[in]   index       the index of the temporary beacon to read
 * @param[out]  beacon   pointer to temporary to write to
 */
void ena_storage_get_temp_beacon(uint32_t index, ena_temp_beacon_t *beacon);

/**
 * @brief       store temporary beacon
 * 
 * @param[in]   beacon   new temporary beacon to store 
 * 
 * @return 
 *              index of new stored beacon
 */
uint32_t ena_storage_add_temp_beacon(ena_temp_beacon_t *beacon);

/**
 * @brief       store temporary beacon at given index
 * 
 * @param[in]   index       the index of the temporary beacon to overwrite
 * @param[in]   beacon   temporary beacon to store 
 */
void ena_storage_set_temp_beacon(uint32_t index, ena_temp_beacon_t *beacon);

/**
 * @brief       remove temporary beacon at given index
 * 
 * @param[in]   index       the index of the temporary beacon to remove
 */
void ena_storage_remove_temp_beacon(uint32_t index);

/**
 * @brief       get number of permanently stored beacons
 * 
 * @return
 *              total number of beacons stored
 */
uint32_t ena_storage_beacons_count(void);

/**
 * @brief       get permanently stored beacon at given index
 * 
 * @param[in]   index       the index of the beacon to read
 * @param[out] beacon    pointer to to write to
 */
void ena_storage_get_beacon(uint32_t index, ena_beacon_t *beacon);

/**
 * @brief       permanently store beacon
 * 
 * @param[in]   beacon   new beacon to permanently store 
 */
void ena_storage_add_beacon(ena_beacon_t *beacon);

/**
 * @brief       erase the storage
 * 
 * This function completely deletes all stored data and resets the counters 
 * of TEKs, temporary beacon and beacon to zero.
 */
void ena_storage_erase(void);

/**
 * @brief       erase all stored TEKs
 * 
 * This function deletes all stored TEKs and resets counter to zero.
 */
void ena_storage_erase_tek(void);

/**
 * @brief       erase all stored temporary beacons
 * 
 * This function deletes all stored temporary beacons and resets counter to zero.
 */
void ena_storage_erase_temporary_beacon(void);

/**
 * @brief       erase all permanently stored beacons
 * 
 * This function deletes all stored beacons and resets counter to zero.
 */
void ena_storage_erase_beacon(void);

/**
 * @brief       dump all stored TEKs to serial output
 * 
 * This function prints all stored TEKs to serial output in
 * the following CSV format: #,enin,tek
 */
void ena_storage_dump_tek(void);

/**
 * @brief       dump all stored temporary beacons to serial output
 * 
 * This function prints all stored temporary beacons to serial output in
 * the following CSV format: #,timestamp_first,timestamp_last,rpi,aem,rssi
 */
void ena_storage_dump_temp_beacons(void);

/**
 * @brief       dump all stored beacons to serial output
 * 
 * This function prints all stored beacons to serial output in
 * the following CSV format: #,timestamp,rpi,aem,rssi
 */
void ena_storage_dump_beacons(void);

#endif