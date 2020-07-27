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
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_partition.h"

#include "ena-storage.h"
#include "ena-crypto.h"

#define BLOCK_SIZE (4096)

const int ENA_STORAGE_TEK_COUNT_ADDRESS = (ENA_STORAGE_START_ADDRESS); // starting address for TEK COUNT
const int ENA_STORAGE_TEK_START_ADDRESS = (ENA_STORAGE_TEK_COUNT_ADDRESS + sizeof(uint32_t));
const int ENA_STORAGE_EXPOSURE_INFORMATION_COUNT_ADDRESS = (ENA_STORAGE_TEK_START_ADDRESS + sizeof(ena_tek_t) * ENA_STORAGE_TEK_MAX);
const int ENA_STORAGE_EXPOSURE_INFORMATION_START_ADDRESS = (ENA_STORAGE_EXPOSURE_INFORMATION_COUNT_ADDRESS + sizeof(uint32_t));
const int ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS = (ENA_STORAGE_EXPOSURE_INFORMATION_COUNT_ADDRESS + sizeof(ena_exposure_information_t) * ENA_STORAGE_EXPOSURE_INFORMATION_MAX);
const int ENA_STORAGE_TEMP_BEACONS_START_ADDRESS = (ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS + sizeof(uint32_t));
const int ENA_STORAGE_BEACONS_COUNT_ADDRESS = (ENA_STORAGE_TEMP_BEACONS_START_ADDRESS + sizeof(ena_beacon_t) * ENA_STORAGE_TEMP_BEACONS_MAX);
const int ENA_STORAGE_BEACONS_START_ADDRESS = (ENA_STORAGE_BEACONS_COUNT_ADDRESS + sizeof(uint32_t));

void ena_storage_read(size_t address, void *data, size_t size)
{
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, ENA_STORAGE_PARTITION_NAME);
    assert(partition);
    ESP_ERROR_CHECK(esp_partition_read(partition, address, data, size));
    vTaskDelay(1);
    ESP_LOGD(ENA_STORAGE_LOG, "read data at %u", address);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, data, size, ESP_LOG_DEBUG);
}

void ena_storage_write(size_t address, void *data, size_t size)
{
    const int block_num = address / BLOCK_SIZE;
    // check for overflow
    if (address + size <= (block_num + 1) * BLOCK_SIZE)
    {
        const esp_partition_t *partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, ENA_STORAGE_PARTITION_NAME);
        assert(partition);
        const int block_start = block_num * BLOCK_SIZE;
        const int block_address = address - block_start;
        void *buffer = malloc(BLOCK_SIZE);
        if (buffer == NULL)
        {
            ESP_LOGE(ENA_STORAGE_LOG, "Warning %s malloc low memory", "buffer");
            return;
        }
        ESP_LOGD(ENA_STORAGE_LOG, "read block %d buffer: start %d size %u", block_num, block_start, BLOCK_SIZE);
        ESP_ERROR_CHECK(esp_partition_read(partition, block_start, buffer, BLOCK_SIZE));
        vTaskDelay(1);
        ESP_ERROR_CHECK(esp_partition_erase_range(partition, block_start, BLOCK_SIZE));

        memcpy((buffer + block_address), data, size);

        ESP_ERROR_CHECK(esp_partition_write(partition, block_start, buffer, BLOCK_SIZE));
        free(buffer);
        ESP_LOGD(ENA_STORAGE_LOG, "write data at %u", address);
        ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, data, size, ESP_LOG_DEBUG);
    }
    else
    {
        ESP_LOGD(ENA_STORAGE_LOG, "overflow block at address %u with size %d (block %d)", address, size, block_num);
        const size_t block2_address = (block_num + 1) * BLOCK_SIZE;
        const size_t data2_size = address + size - block2_address;
        const size_t data1_size = size - data2_size;
        ESP_LOGD(ENA_STORAGE_LOG, "block1_address %d, block1_size %d (block %d)", address, data1_size, block_num);
        ESP_LOGD(ENA_STORAGE_LOG, "block2_address %d, block2_size %d (block %d)", block2_address, data2_size, block_num + 1);
        void *data1 = malloc(data1_size);
        memcpy(data1, data, data1_size);
        ena_storage_write(address, data1, data1_size);
        free(data1);

        void *data2 = malloc(data2_size);
        memcpy(data2, (data + data1_size), data2_size);
        ena_storage_write(block2_address, data2, data2_size);
        free(data2);
    }
}

void ena_storage_shift_delete(size_t address, size_t end_address, size_t size)
{
    int block_num_start = address / BLOCK_SIZE;
    // check for overflow
    if (address + size <= (block_num_start + 1) * BLOCK_SIZE)
    {
        const esp_partition_t *partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, ENA_STORAGE_PARTITION_NAME);
        assert(partition);

        int block_num_end = end_address / BLOCK_SIZE;
        size_t block_start = address - block_num_start * BLOCK_SIZE;
        while (block_num_end >= block_num_start)
        {

            void *buffer = malloc(BLOCK_SIZE);
            ESP_ERROR_CHECK(esp_partition_read(partition, block_num_start * BLOCK_SIZE, buffer, BLOCK_SIZE));
            vTaskDelay(1);
            // shift inside buffer
            ESP_LOGD(ENA_STORAGE_LOG, "shift block %d from %u to %u with size %u", block_num_start, (block_start + size), block_start, (BLOCK_SIZE - block_start - size));
            memcpy((buffer + block_start), (buffer + block_start + size), BLOCK_SIZE - block_start - size);
            if (block_num_end > block_num_start)
            {
                void *buffer_next_block = malloc(BLOCK_SIZE);

                ESP_ERROR_CHECK(esp_partition_read(partition, (block_num_start + 1) * BLOCK_SIZE, buffer_next_block, BLOCK_SIZE));
                vTaskDelay(1);
                // shift from next block
                ESP_LOGD(ENA_STORAGE_LOG, "shift next block size %u", size);
                memcpy((buffer + BLOCK_SIZE - size), buffer_next_block, size);
                free(buffer_next_block);
            }

            ESP_ERROR_CHECK(esp_partition_erase_range(partition, block_num_start * BLOCK_SIZE, BLOCK_SIZE));
            ESP_ERROR_CHECK(esp_partition_write(partition, block_num_start * BLOCK_SIZE, buffer, BLOCK_SIZE));
            free(buffer);

            block_num_start++;
            block_start = 0;
        }
    }
    else
    {
        ESP_LOGD(ENA_STORAGE_LOG, "overflow block at address %u with size %d (block %d)", address, size, block_num_start);
        const size_t block1_address = address;
        const size_t block2_address = (block_num_start + 1) * BLOCK_SIZE;
        const size_t data2_size = address + size - block2_address;
        const size_t data1_size = size - data2_size;
        ena_storage_shift_delete(block1_address, block2_address, data1_size);
        ena_storage_shift_delete(block2_address, end_address - data1_size, data2_size);
    }
}

uint32_t ena_storage_read_last_tek(ena_tek_t *tek)
{
    uint32_t tek_count = 0;
    ena_storage_read(ENA_STORAGE_TEK_COUNT_ADDRESS, &tek_count, sizeof(uint32_t));
    if (tek_count < 1)
    {
        return 0;
    }
    uint8_t index = (tek_count % ENA_STORAGE_TEK_MAX) - 1;
    ena_storage_read(ENA_STORAGE_TEK_START_ADDRESS + index * sizeof(ena_tek_t), tek, sizeof(ena_tek_t));

    ESP_LOGD(ENA_STORAGE_LOG, "read last tek %u:", tek->enin);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, tek->key_data, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    return tek_count;
}

void ena_storage_write_tek(ena_tek_t *tek)
{
    uint32_t tek_count = 0;
    ena_storage_read(ENA_STORAGE_TEK_COUNT_ADDRESS, &tek_count, sizeof(uint32_t));
    uint8_t index = (tek_count % ENA_STORAGE_TEK_MAX);
    ena_storage_write(ENA_STORAGE_TEK_START_ADDRESS + index * sizeof(ena_tek_t), tek, sizeof(ena_tek_t));

    tek_count++;
    ena_storage_write(ENA_STORAGE_TEK_COUNT_ADDRESS, &tek_count, sizeof(uint32_t));

    ESP_LOGD(ENA_STORAGE_LOG, "write tek: ENIN %u", tek->enin);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, tek->key_data, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
}

uint32_t ena_storage_exposure_information_count(void)
{
    uint32_t count = 0;
    ena_storage_read(ENA_STORAGE_EXPOSURE_INFORMATION_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read exposure information count: %u", count);
    return count;
}

void ena_storage_get_exposure_information(uint32_t index, ena_exposure_information_t *exposure_info)
{
    ena_storage_read(ENA_STORAGE_EXPOSURE_INFORMATION_START_ADDRESS + index * sizeof(ena_exposure_information_t), exposure_info, sizeof(ena_exposure_information_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read exporuse information: day %u, duration %d", exposure_info->day, exposure_info->duration_minutes);
}

void ena_storage_add_exposure_information(ena_exposure_information_t *exposure_info)
{
    uint32_t count = ena_storage_exposure_information_count();
    ena_storage_write(ENA_STORAGE_EXPOSURE_INFORMATION_START_ADDRESS + count * sizeof(ena_exposure_information_t), exposure_info, sizeof(ena_exposure_information_t));
    count++;
    ena_storage_write(ENA_STORAGE_EXPOSURE_INFORMATION_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "write exposure info:  day %u, duration %d", exposure_info->day, exposure_info->duration_minutes);
}

uint32_t ena_storage_temp_beacons_count(void)
{
    uint32_t count = 0;
    ena_storage_read(ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read temp beacons count: %u", count);
    return count;
}

void ena_storage_get_temp_beacon(uint32_t index, ena_beacon_t *beacon)
{
    ena_storage_read(ENA_STORAGE_TEMP_BEACONS_START_ADDRESS + index * sizeof(ena_beacon_t), beacon, sizeof(ena_beacon_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read temp beacon: first %u, last %u and rssi %d", beacon->timestamp_first, beacon->timestamp_last, beacon->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
}

uint32_t ena_storage_add_temp_beacon(ena_beacon_t *beacon)
{
    uint32_t count = ena_storage_temp_beacons_count();
    // overwrite older temporary beacons?!
    uint8_t index = count % ENA_STORAGE_TEMP_BEACONS_MAX;
    ena_storage_set_temp_beacon(index, beacon);
    ESP_LOGD(ENA_STORAGE_LOG, "add temp beacon at %u: first %u, last %u  and rssi %d", index, beacon->timestamp_first, beacon->timestamp_last, beacon->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
    count++;
    ena_storage_write(ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    return count - 1;
}

void ena_storage_set_temp_beacon(uint32_t index, ena_beacon_t *beacon)
{
    ena_storage_write(ENA_STORAGE_TEMP_BEACONS_START_ADDRESS + index * sizeof(ena_beacon_t), beacon, sizeof(ena_beacon_t));
    ESP_LOGD(ENA_STORAGE_LOG, "set temp beacon at %u: first %u, last %u  and rssi %d", index, beacon->timestamp_first, beacon->timestamp_last, beacon->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
}

void ena_storage_remove_temp_beacon(uint32_t index)
{
    uint32_t count = ena_storage_temp_beacons_count();
    size_t address_from = ENA_STORAGE_TEMP_BEACONS_START_ADDRESS + index * sizeof(ena_beacon_t);
    size_t address_to = ENA_STORAGE_TEMP_BEACONS_START_ADDRESS + count * sizeof(ena_beacon_t);

    ena_storage_shift_delete(address_from, address_to, sizeof(ena_beacon_t));

    count--;
    ena_storage_write(ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "remove temp beacon: %u", index);
}

uint32_t ena_storage_beacons_count(void)
{
    uint32_t count = 0;
    ena_storage_read(ENA_STORAGE_BEACONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read contancts count: %u", count);
    return count;
}

void ena_storage_get_beacon(uint32_t index, ena_beacon_t *beacon)
{
    ena_storage_read(ENA_STORAGE_BEACONS_START_ADDRESS + index * sizeof(ena_beacon_t), beacon, sizeof(ena_beacon_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read beacon: first %u, last %u and rssi %d", beacon->timestamp_first, beacon->timestamp_last, beacon->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
}

void ena_storage_add_beacon(ena_beacon_t *beacon)
{
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    uint32_t count = ena_storage_beacons_count();
    ena_storage_write(ENA_STORAGE_BEACONS_START_ADDRESS + count * sizeof(ena_beacon_t), beacon, sizeof(ena_beacon_t));
    count++;
    ena_storage_write(ENA_STORAGE_BEACONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "write beacon: first %u, last %u  and rssi %d", beacon->timestamp_first, beacon->timestamp_last, beacon->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, beacon->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
}

void ena_storage_remove_beacon(uint32_t index)
{
    uint32_t count = ena_storage_beacons_count();
    size_t address_from = ENA_STORAGE_BEACONS_START_ADDRESS + index * sizeof(ena_beacon_t);
    size_t address_to = ENA_STORAGE_BEACONS_START_ADDRESS + count * sizeof(ena_beacon_t);

    ena_storage_shift_delete(address_from, address_to, sizeof(ena_beacon_t));

    count--;
    ena_storage_write(ENA_STORAGE_BEACONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "remove beacon: %u", index);
}

void ena_storage_erase(void)
{
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, ENA_STORAGE_PARTITION_NAME);
    assert(partition);
    ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, partition->size));
    ESP_LOGI(ENA_STORAGE_LOG, "erased partition %s!", ENA_STORAGE_PARTITION_NAME);

    uint32_t count = 0;
    ena_storage_write(ENA_STORAGE_TEK_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ena_storage_write(ENA_STORAGE_EXPOSURE_INFORMATION_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ena_storage_write(ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ena_storage_write(ENA_STORAGE_BEACONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
}

void ena_storage_erase_tek(void)
{
    uint32_t count = 0;
    ena_storage_read(ENA_STORAGE_TEK_COUNT_ADDRESS, &count, sizeof(uint32_t));
    uint32_t stored = ENA_STORAGE_TEK_MAX;

    if (count < ENA_STORAGE_TEK_MAX)
    {
        stored = count;
    }

    size_t size = sizeof(uint32_t) + stored * sizeof(ena_tek_t);
    uint8_t *zeros = calloc(size, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_TEK_COUNT_ADDRESS, zeros, size);
    free(zeros);
    ESP_LOGI(ENA_STORAGE_LOG, "erased %d teks (size %u at %u)", stored, size, ENA_STORAGE_TEK_COUNT_ADDRESS);
}

void ena_storage_erase_exposure_information(void)
{
    uint32_t count = ena_storage_exposure_information_count();
    uint32_t stored = ENA_STORAGE_EXPOSURE_INFORMATION_MAX;

    if (count < ENA_STORAGE_EXPOSURE_INFORMATION_MAX)
    {
        stored = count;
    }

    size_t size = sizeof(uint32_t) + stored * sizeof(ena_exposure_information_t);
    uint8_t *zeros = calloc(size, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_EXPOSURE_INFORMATION_COUNT_ADDRESS, zeros, size);
    free(zeros);
    ESP_LOGI(ENA_STORAGE_LOG, "erased %d exposure information (size %u at %u)", stored, size, ENA_STORAGE_EXPOSURE_INFORMATION_COUNT_ADDRESS);
}

void ena_storage_erase_temporary_beacon(void)
{
    uint32_t beacon_count = 0;
    ena_storage_read(ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS, &beacon_count, sizeof(uint32_t));
    uint32_t stored = ENA_STORAGE_TEMP_BEACONS_MAX;

    if (beacon_count < ENA_STORAGE_TEMP_BEACONS_MAX)
    {
        stored = beacon_count;
    }

    size_t size = sizeof(uint32_t) + stored * sizeof(ena_beacon_t);
    uint8_t *zeros = calloc(size, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS, zeros, size);
    free(zeros);

    ESP_LOGI(ENA_STORAGE_LOG, "erased %d temporary beacons (size %u at %u)", stored, size, ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS);
}

void ena_storage_erase_beacon(void)
{
    uint32_t beacon_count = 0;
    ena_storage_read(ENA_STORAGE_BEACONS_COUNT_ADDRESS, &beacon_count, sizeof(uint32_t));

    size_t size = sizeof(uint32_t) + beacon_count * sizeof(ena_beacon_t);
    uint8_t *zeros = calloc(size, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_BEACONS_COUNT_ADDRESS, zeros, size);
    free(zeros);

    ESP_LOGI(ENA_STORAGE_LOG, "erased %d beacons (size %u at %u)", beacon_count, size, ENA_STORAGE_BEACONS_COUNT_ADDRESS);
}

void ena_storage_dump_hash_array(uint8_t *data, size_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (i == 0)
        {
            printf("%02x", data[i]);
        }
        else
        {
            printf(" %02x", data[i]);
        }
    }
}

void ena_storage_dump_teks(void)
{
    ena_tek_t tek;
    uint32_t tek_count = 0;
    ena_storage_read(ENA_STORAGE_TEK_COUNT_ADDRESS, &tek_count, sizeof(uint32_t));
    uint32_t stored = ENA_STORAGE_TEK_MAX;

    if (tek_count < ENA_STORAGE_TEK_MAX)
    {
        stored = tek_count;
    }

    ESP_LOGD(ENA_STORAGE_LOG, "%u TEKs (%u stored)\n", tek_count, stored);
    printf("#,enin,tek,rolling_period\n");
    for (int i = 0; i < stored; i++)
    {

        size_t address = ENA_STORAGE_TEK_START_ADDRESS + i * sizeof(ena_tek_t);
        ena_storage_read(address, &tek, sizeof(ena_tek_t));
        printf("%d,%u,", i, tek.enin);
        ena_storage_dump_hash_array(tek.key_data, ENA_KEY_LENGTH);
        printf(",%u\n", tek.rolling_period);
    }
}

void ena_storage_dump_exposure_information(void)
{
    ena_exposure_information_t exposure_info;
    uint32_t exposure_information_count = ena_storage_exposure_information_count();
    uint32_t stored = ENA_STORAGE_EXPOSURE_INFORMATION_MAX;

    if (exposure_information_count < ENA_STORAGE_EXPOSURE_INFORMATION_MAX)
    {
        stored = exposure_information_count;
    }

    ESP_LOGD(ENA_STORAGE_LOG, "%u exposure information (%u stored)\n", exposure_information_count, stored);
    printf("#,day,typical_attenuation,min_attenuation,duration_minutes,report_type\n");
    for (int i = 0; i < stored; i++)
    {

        size_t address = ENA_STORAGE_EXPOSURE_INFORMATION_START_ADDRESS + i * sizeof(ena_exposure_information_t);
        ena_storage_read(address, &exposure_info, sizeof(ena_exposure_information_t));
        printf("%d,%u,%d,%d,%d,%d\n", i, exposure_info.day, exposure_info.typical_attenuation, exposure_info.min_attenuation, exposure_info.duration_minutes, exposure_info.report_type);
    }
}

void ena_storage_dump_temp_beacons(void)
{
    ena_beacon_t beacon;
    uint32_t beacon_count = 0;
    ena_storage_read(ENA_STORAGE_TEMP_BEACONS_COUNT_ADDRESS, &beacon_count, sizeof(uint32_t));
    uint32_t stored = ENA_STORAGE_TEMP_BEACONS_MAX;

    if (beacon_count < ENA_STORAGE_TEMP_BEACONS_MAX)
    {
        stored = beacon_count;
    }
    ESP_LOGD(ENA_STORAGE_LOG, "%u temporary beacons (%u stored)\n", beacon_count, stored);
    printf("#,timestamp_first,timestamp_last,rpi,aem,rssi\n");
    for (int i = 0; i < stored; i++)
    {
        ena_storage_get_temp_beacon(i, &beacon);
        printf("%d,%u,%u,", i, beacon.timestamp_first, beacon.timestamp_last);
        ena_storage_dump_hash_array(beacon.rpi, ENA_KEY_LENGTH);
        printf(",");
        ena_storage_dump_hash_array(beacon.aem, ENA_AEM_METADATA_LENGTH);
        printf(",%d\n", beacon.rssi);
    }
}

void ena_storage_dump_beacons(void)
{

    ena_beacon_t beacon;
    uint32_t beacon_count = 0;
    ena_storage_read(ENA_STORAGE_BEACONS_COUNT_ADDRESS, &beacon_count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "%u beacons\n", beacon_count);
    printf("#,timestamp_first,timestamp_last,rpi,aem,rssi\n");
    for (int i = 0; i < beacon_count; i++)
    {
        ena_storage_get_beacon(i, &beacon);
        printf("%d,%u,%u,", i, beacon.timestamp_first, beacon.timestamp_last);
        ena_storage_dump_hash_array(beacon.rpi, ENA_KEY_LENGTH);
        printf(",");
        ena_storage_dump_hash_array(beacon.aem, ENA_AEM_METADATA_LENGTH);
        printf(",%d\n", beacon.rssi);
    }
}