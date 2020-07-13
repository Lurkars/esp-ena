#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_partition.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "ena-storage.h"
#include "ena-crypto.h"

#define BLOCK_SIZE (4096)

const int ENA_STORAGE_TEK_COUNT_ADDRESS = (0); // starting address for TEK COUNT
const int ENA_STORAGE_TEK_START_ADDRESS = (ENA_STORAGE_TEK_COUNT_ADDRESS + sizeof(uint8_t));
const int ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS = (ENA_STORAGE_TEK_START_ADDRESS + sizeof(ena_tek_t) * ENA_STOARGE_TEK_STORE_PERIOD);                      // starting address for temporary detections COUNT (offset from max. stored TEKs)
const int ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS = (ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS + sizeof(uint32_t));                                          // starting address for temporary detections
const int ENA_STORAGE_DETECTIONS_COUNT_ADDRESS = (ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + sizeof(ena_temp_detection_t) * ENA_STOARGE_TEMP_DETECTIONS_MAX); // starting address for detections COUNT (offset from max. stored temporary detections)
const int ENA_STORAGE_DETECTIONS_START_ADDRESS = (ENA_STORAGE_DETECTIONS_COUNT_ADDRESS + sizeof(uint32_t));

void ena_storage_read(size_t address, void *data, size_t size)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_read");
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, PARTITION_NAME);
    assert(partition);
    ESP_ERROR_CHECK(esp_partition_read(partition, address, data, size));
    vTaskDelay(1);
    ESP_LOGD(ENA_STORAGE_LOG, "read data at %u", address);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, data, size, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_read");
}

void ena_storage_write(size_t address, void *data, size_t size)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_write");
    const int block_num = address / BLOCK_SIZE;
    // check for overflow
    if (address + size <= (block_num + 1) * BLOCK_SIZE)
    {
        const esp_partition_t *partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, PARTITION_NAME);
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
        const size_t block1_address = address;
        const size_t block2_address = (block_num + 1) * BLOCK_SIZE;
        const size_t data2_size = address + size - block2_address;
        const size_t data1_size = size - data2_size;
        ESP_LOGD(ENA_STORAGE_LOG, "block1_address %d, block1_size %d (block %d)", block1_address, data1_size, block_num);
        ESP_LOGD(ENA_STORAGE_LOG, "block2_address %d, block2_size %d (block %d)", block2_address, data2_size, block_num + 1);
        void *data1 = malloc(data1_size);
        memcpy(data1, data, data1_size);
        ena_storage_write(block1_address, data1, data1_size);
        free(data1);

        void *data2 = malloc(data2_size);
        memcpy(data2, (data + data1_size), data2_size);
        ena_storage_write(block2_address, data2, data2_size);
        free(data2);
    }
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_write");
}

void ena_storage_shift_delete(size_t address, size_t end_address, size_t size)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_shift_delete");

    int block_num_start = address / BLOCK_SIZE;
    // check for overflow
    if (address + size < (block_num_start + 1) * BLOCK_SIZE)
    {
        const esp_partition_t *partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, PARTITION_NAME);
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
            else
            {
                // fill end with zeros
                ESP_LOGD(ENA_STORAGE_LOG, "fill with zeros %u", size);
                memset((buffer + BLOCK_SIZE - size), 0, size);
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
        ena_storage_shift_delete(block1_address, end_address, data1_size);
        ena_storage_shift_delete(block2_address, end_address - data1_size, data2_size);
    }
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_shift_delete");
}

uint8_t ena_storage_read_last_tek(ena_tek_t *tek)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_read_tek");
    uint8_t tek_count = 0;
    ena_storage_read(ENA_STORAGE_TEK_COUNT_ADDRESS, &tek_count, sizeof(uint8_t));
    if (tek_count < 1)
    {
        return 0;
    }
    size_t address = ENA_STORAGE_TEK_START_ADDRESS + (tek_count - 1) * sizeof(ena_tek_t);

    ena_storage_read(address, tek, sizeof(ena_tek_t));

    ESP_LOGD(ENA_STORAGE_LOG, "read last tek %u:", tek->enin);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, tek->key_data, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_read_tek");
    return tek_count;
}

void ena_storage_write_tek(ena_tek_t *tek)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_write_tek");

    uint8_t tek_count = 0;
    ena_storage_read(ENA_STORAGE_TEK_COUNT_ADDRESS, &tek_count, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_TEK_START_ADDRESS + tek_count * sizeof(ena_tek_t), tek, sizeof(ena_tek_t));

    tek_count++;
    if (tek_count > ENA_STOARGE_TEK_STORE_PERIOD)
    {
        tek_count = 1;
    }
    ena_storage_write(ENA_STORAGE_TEK_COUNT_ADDRESS, &tek_count, sizeof(uint8_t));

    ESP_LOGD(ENA_STORAGE_LOG, "write tek: ENIN %u", tek->enin);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, tek->key_data, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_write_tek");
}

uint32_t ena_storage_temp_detections_count(void)
{

    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_temp_detections_count");
    uint32_t count = 0;
    ena_storage_read(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read temp contancts count: %u", count);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_temp_detections_count");
    return count;
}

void ena_storage_read_temp_detection(uint32_t index, ena_temp_detection_t *detection)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_read_temp_detection");
    ena_storage_read(ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + index * sizeof(ena_temp_detection_t), detection, sizeof(ena_temp_detection_t));

    ESP_LOGD(ENA_STORAGE_LOG, "read temp detection: first %u, last %u and rssi %d", detection->timestamp_first, detection->timestamp_last, detection->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_read_temp_detection");
}

uint32_t ena_storage_write_temp_detection(ena_temp_detection_t *detection)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_write_temp_detection");
    uint32_t count = ena_storage_temp_detections_count();

    // start overwriting temporay detections?!
    if (count > ENA_STOARGE_TEMP_DETECTIONS_MAX)
    {
        count = 0;
    }
    ena_storage_write(ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + count * sizeof(ena_temp_detection_t), detection, sizeof(ena_temp_detection_t));
    count++;
    ena_storage_write(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "write temp detection: first %u, last %u  and rssi %d", detection->timestamp_first, detection->timestamp_last, detection->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);

    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_write_temp_detection");
    return count - 1;
}

void ena_storage_remove_temp_detection(uint32_t index)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_remove_temp_detection");
    uint32_t count = ena_storage_temp_detections_count();
    size_t address_from = ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + index * sizeof(ena_detection_t);
    size_t address_to = ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + count * sizeof(ena_detection_t);

    ena_storage_shift_delete(address_from, address_to, sizeof(ena_detection_t));

    count--;
    ena_storage_write(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "remove temp detection: %u", index);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_remove_temp_detection");
}

uint32_t ena_storage_detections_count(void)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_detections_count");
    uint32_t count = 0;
    ena_storage_read(ENA_STORAGE_DETECTIONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read contancts count: %u", count);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_detections_count");
    return count;
}

void ena_storage_read_detection(uint32_t index, ena_detection_t *detection)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_read_detection");
    ena_storage_read(ENA_STORAGE_DETECTIONS_START_ADDRESS + index * sizeof(ena_detection_t), detection, sizeof(ena_detection_t));
    ESP_LOGD(ENA_STORAGE_LOG, "read detection: timestamp %u and rssi %d", detection->timestamp, detection->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_read_detection");
}

void ena_storage_write_detection(ena_detection_t *detection)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_write_detection");
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    uint32_t count = ena_storage_detections_count();
    ena_storage_write(ENA_STORAGE_DETECTIONS_START_ADDRESS + count * sizeof(ena_detection_t), detection, sizeof(ena_detection_t));
    count++;
    ena_storage_write(ENA_STORAGE_DETECTIONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "write detection: timestamp %u and rssi %d", detection->timestamp, detection->rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, detection->aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_write_detection");
}

void ena_storage_erase(void)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_erase");
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, PARTITION_NAME);
    assert(partition);
    ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, partition->size));
    ESP_LOGI(PARTITION_NAME, "erase partition!");

    uint32_t count = 0;
    ena_storage_write(ENA_STORAGE_TEK_COUNT_ADDRESS, &count, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS, &count, sizeof(uint32_t));
    ena_storage_write(ENA_STORAGE_DETECTIONS_COUNT_ADDRESS, &count, sizeof(uint32_t));

    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_erase");
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

void ena_storage_dump_tek(void)
{
    ena_tek_t tek;
    uint8_t tek_count = 0;
    ena_storage_read(ENA_STORAGE_TEK_COUNT_ADDRESS, &tek_count, sizeof(uint8_t));
    ESP_LOGD(ENA_STORAGE_LOG, "%u TEKs\n", tek_count);
    printf("#,enin,tek\n");
    for (int i = 0; i < tek_count; i++)
    {

        size_t address = ENA_STORAGE_TEK_START_ADDRESS + i * sizeof(ena_tek_t);
        ena_storage_read(address, &tek, sizeof(ena_tek_t));
        printf("%d,%u,", i, tek.enin);
        ena_storage_dump_hash_array(tek.key_data, ENA_KEY_LENGTH);
        printf("\n");
    }
}

void ena_storage_dump_temp_detections(void)
{
    ena_temp_detection_t detection;
    uint32_t detection_count = 0;
    ena_storage_read(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS, &detection_count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "%u temporary detections\n", detection_count);
    printf("#,timestamp_first,timestamp_last,rpi,aem,rssi\n");
    for (int i = 0; i < detection_count; i++)
    {
        ena_storage_read_temp_detection(i, &detection);
        printf("%d,%u,%u,", i, detection.timestamp_first, detection.timestamp_last);
        ena_storage_dump_hash_array(detection.rpi, ENA_KEY_LENGTH);
        printf(",");
        ena_storage_dump_hash_array(detection.aem, ENA_AEM_METADATA_LENGTH);
        printf(",%d\n", detection.rssi);
    }
}

void ena_storage_dump_detections(void)
{

    ena_detection_t detection;
    uint32_t detection_count = 0;
    ena_storage_read(ENA_STORAGE_DETECTIONS_COUNT_ADDRESS, &detection_count, sizeof(uint32_t));
    ESP_LOGD(ENA_STORAGE_LOG, "%u detections\n", detection_count);
    printf("#,timestamp,rpi,aem,rssi\n");
    for (int i = 0; i < detection_count; i++)
    {
        ena_storage_read_detection(i, &detection);
        printf("%d,%u,", i, detection.timestamp);
        ena_storage_dump_hash_array(detection.rpi, ENA_KEY_LENGTH);
        printf(",");
        ena_storage_dump_hash_array(detection.aem, ENA_AEM_METADATA_LENGTH);
        printf(",%d\n", detection.rssi);
    }
}