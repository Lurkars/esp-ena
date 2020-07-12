#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_partition.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "ena-storage.h"
#include "ena-crypto.h"

#define BLOCK_SIZE 4096

void ena_storage_read(size_t address, uint8_t *data, size_t size)
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

void ena_storage_write(size_t address, uint8_t *data, size_t size)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_write");
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, PARTITION_NAME);
    assert(partition);

    const int block_num = address / BLOCK_SIZE;

    // split if size extends block
    if (address + size > (block_num + 1) * BLOCK_SIZE)
    {

        ESP_LOGD(ENA_STORAGE_LOG, "overflow block at address %u with size %d (block %d)", address, size, block_num);
        const size_t block1_address = address;
        const size_t block2_address = (block_num + 1) * BLOCK_SIZE;
        const size_t data2_size = address + size - block2_address;
        const size_t data1_size = size - data2_size;
        ESP_LOGD(ENA_STORAGE_LOG, "block1_address %d, block1_size %d (block %d)", block1_address, data1_size, block_num);
        ESP_LOGD(ENA_STORAGE_LOG, "block2_address %d, block2_size %d (block %d)", block2_address, data2_size, block_num + 1);
        uint8_t *data1 = malloc(data1_size);
        memcpy(data1, data, data1_size);
        ena_storage_write(block1_address, data1, data1_size);
        free(data1);

        uint8_t *data2 = malloc(data2_size);
        memcpy(data2, &data[data1_size], data2_size);
        ena_storage_write(block2_address, data2, data2_size);
        free(data2);
    }
    else
    {
        const int block_start = block_num * BLOCK_SIZE;
        const int block_address = address - block_start;
        uint8_t *buffer = malloc(BLOCK_SIZE);
        if (buffer == NULL)
        {
            ESP_LOGE(ENA_STORAGE_LOG, "Warning %s malloc low memory", "buffer");
            return;
        }
        ESP_LOGD(ENA_STORAGE_LOG, "read block %d buffer: start %d size %u", block_num, block_start, BLOCK_SIZE);
        ESP_ERROR_CHECK(esp_partition_read(partition, block_start, buffer, BLOCK_SIZE));
        vTaskDelay(1);
        ESP_ERROR_CHECK(esp_partition_erase_range(partition, block_start, BLOCK_SIZE));

        memcpy(&buffer[block_address], data, size);

        ESP_ERROR_CHECK(esp_partition_write(partition, block_start, buffer, BLOCK_SIZE));
        free(buffer);
        ESP_LOGD(ENA_STORAGE_LOG, "write data at %u", address);
        ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, data, size, ESP_LOG_DEBUG);
    }
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_write");
}

void ena_storage_shift_delete(size_t address, size_t end_address, size_t size)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_shift_delete");

    int block_num_start = address / BLOCK_SIZE;
    // split if size extends block
    if (address + size > (block_num_start + 1) * BLOCK_SIZE)
    {
        ESP_LOGD(ENA_STORAGE_LOG, "overflow block at address %u with size %d (block %d)", address, size, block_num_start);
        const size_t block1_address = address;
        const size_t block2_address = (block_num_start + 1) * BLOCK_SIZE;
        const size_t data2_size = address + size - block2_address;
        const size_t data1_size = size - data2_size;
        ena_storage_shift_delete(block1_address, end_address, data1_size);
        ena_storage_shift_delete(block2_address, end_address - data1_size, data2_size);
    }
    else
    {
        const esp_partition_t *partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, PARTITION_NAME);
        assert(partition);

        int block_num_end = end_address / BLOCK_SIZE;
        size_t block_start = address - block_num_start * BLOCK_SIZE;
        while (block_num_end >= block_num_start)
        {

            uint8_t *buffer = malloc(BLOCK_SIZE);
            ESP_ERROR_CHECK(esp_partition_read(partition, block_num_start * BLOCK_SIZE, buffer, BLOCK_SIZE));
            vTaskDelay(1);
            // shift inside buffer
            ESP_LOGD(ENA_STORAGE_LOG, "shift block %d from %u to %u with size %u", block_num_start, (block_start + size), block_start, (BLOCK_SIZE - block_start - size));
            memcpy(&buffer[block_start], &buffer[block_start + size], BLOCK_SIZE - block_start - size);
            if (block_num_end > block_num_start)
            {
                uint8_t *buffer_next_block = malloc(BLOCK_SIZE);

                ESP_ERROR_CHECK(esp_partition_read(partition, (block_num_start + 1) * BLOCK_SIZE, buffer_next_block, BLOCK_SIZE));
                vTaskDelay(1);
                // shift from next block
                ESP_LOGD(ENA_STORAGE_LOG, "shift next block size %u", size);
                memcpy(&buffer[BLOCK_SIZE - size], &buffer_next_block[0], size);
                free(buffer_next_block);
            }
            else
            {
                // fill end with zeros
                ESP_LOGD(ENA_STORAGE_LOG, "fill with zeros %u", size);
                memset(&buffer[BLOCK_SIZE - size], 0, size);
            }

            ESP_ERROR_CHECK(esp_partition_erase_range(partition, block_num_start * BLOCK_SIZE, BLOCK_SIZE));
            ESP_ERROR_CHECK(esp_partition_write(partition, block_num_start * BLOCK_SIZE, buffer, BLOCK_SIZE));
            free(buffer);

            block_num_start++;
            block_start = 0;
        }
    }
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_shift_delete");
}

void ena_storage_write_tek(uint32_t enin, uint8_t *tek)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_write_tek");
    uint8_t tek_count = ena_storage_read_u8(ENA_STORAGE_TEK_COUNT_ADDRESS);
    size_t address = ENA_STORAGE_TEK_START_ADDRESS + (tek_count * ENA_STORAGE_TEK_LENGTH);
    ena_storage_write_u32(address, enin);
    ena_storage_write(address + 4, tek, ENA_KEY_LENGTH);

    tek_count++;
    if (tek_count > ENA_STOARGE_TEK_STORE_PERIOD)
    {
        tek_count = 0;
    }
    ena_storage_write_u8(ENA_STORAGE_TEK_COUNT_ADDRESS, tek_count);

    ESP_LOGD(ENA_STORAGE_LOG, "write tek: ENIN %u", enin);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, tek, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_write_tek");
}

uint32_t ena_storage_read_enin(void)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_read_enin");
    uint8_t tek_count = ena_storage_read_u8(ENA_STORAGE_TEK_COUNT_ADDRESS);
    if (tek_count < 1)
    {
        return 0;
    }
    size_t address = ENA_STORAGE_TEK_START_ADDRESS + (tek_count - 1) * ENA_STORAGE_TEK_LENGTH;
    uint32_t result = ena_storage_read_u32(address);

    ESP_LOGD(ENA_STORAGE_LOG, "read last ENIN: %u", result);

    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_read_enin");
    return result;
}

void ena_storage_read_tek(uint8_t *tek)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_read_tek");
    uint8_t tek_count = ena_storage_read_u8(ENA_STORAGE_TEK_COUNT_ADDRESS);
    if (tek_count < 1)
    {
        return;
    }
    size_t address = ENA_STORAGE_TEK_START_ADDRESS + (tek_count - 1) * ENA_STORAGE_TEK_LENGTH + 4;
    ena_storage_read(address, tek, ENA_KEY_LENGTH);

    ESP_LOGD(ENA_STORAGE_LOG, "read last tek:");
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, tek, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_read_tek");
}

uint32_t ena_storage_temp_detections_count(void)
{

    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_temp_detections_count");
    uint32_t count = ena_storage_read_u32(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS);
    ESP_LOGD(ENA_STORAGE_LOG, "read temp contancts count: %u", count);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_temp_detections_count");
    return count;
}

uint32_t ena_storage_write_temp_detection(uint32_t timestamp, uint8_t *rpi, uint8_t *aem, int rssi)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_write_temp_detection");
    uint32_t count = ena_storage_temp_detections_count() + 1;

    // start overwriting temporay detections?!
    if (count > ENA_STOARGE_TEMP_DETECTIONS_MAX)
    {
        count = 1;
    }

    size_t address = ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + (count - 1) * ENA_STORAGE_DETECTION_LENGTH;
    ena_storage_write_u32(address, timestamp);
    address += 4;
    ena_storage_write(address, rpi, ENA_KEY_LENGTH);
    address += ENA_KEY_LENGTH;
    ena_storage_write(address, aem, ENA_AEM_METADATA_LENGTH);
    address += ENA_AEM_METADATA_LENGTH;
    ena_storage_write_int(address, rssi);
    ena_storage_write_u32(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS, count);

    ESP_LOGD(ENA_STORAGE_LOG, "write temp detection: timestamp %u and rssi %d", timestamp, rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);

    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_write_temp_detection");
    return count - 1;
}

void ena_storage_read_temp_detection(uint32_t index, uint32_t *timestamp, uint8_t *rpi, uint8_t *aem, int *rssi)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_read_temp_detection");
    size_t address = ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + index * ENA_STORAGE_DETECTION_LENGTH;
    *timestamp = ena_storage_read_u32(address);
    address += 4;
    ena_storage_read(address, rpi, ENA_KEY_LENGTH);
    address += ENA_KEY_LENGTH;
    ena_storage_read(address, aem, ENA_AEM_METADATA_LENGTH);
    address += 4;
    *rssi = ena_storage_read_int(address);

    ESP_LOGD(ENA_STORAGE_LOG, "read temp detection: timestamp %u and rssi %d", *timestamp, *rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_read_temp_detection");
}

void ena_storage_remove_temp_detection(uint32_t index)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_remove_temp_detection");
    size_t address = ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + index * ENA_STORAGE_DETECTION_LENGTH;

    uint32_t count = ena_storage_temp_detections_count();
    ena_storage_shift_delete(address, ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS + count * ENA_STORAGE_DETECTION_LENGTH, ENA_STORAGE_DETECTION_LENGTH);

    count--;
    ena_storage_write_u32(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS, count);
    ESP_LOGD(ENA_STORAGE_LOG, "remove temp detection: %u", index);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_remove_temp_detection");
}

uint32_t ena_storage_detections_count(void)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_detections_count");
    uint32_t count = ena_storage_read_u32(ENA_STORAGE_DETECTIONS_COUNT_ADDRESS);
    ESP_LOGD(ENA_STORAGE_LOG, "read contancts count: %u", count);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_detections_count");
    return count;
}

void ena_storage_write_detection(uint32_t timestamp, uint8_t *rpi, uint8_t *aem, int rssi)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_write_detection");
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    uint32_t count = ena_storage_detections_count() + 1;
    size_t address = ENA_STORAGE_DETECTIONS_START_ADDRESS + (count - 1) * ENA_STORAGE_DETECTION_LENGTH;
    ena_storage_write_u32(address, timestamp);
    address += 4;
    ena_storage_write(address, rpi, ENA_KEY_LENGTH);
    address += ENA_KEY_LENGTH;
    ena_storage_write(address, aem, ENA_AEM_METADATA_LENGTH);
    address += ENA_AEM_METADATA_LENGTH;
    ena_storage_write_int(address, rssi);
    ena_storage_write_u32(ENA_STORAGE_DETECTIONS_COUNT_ADDRESS, count);

    ESP_LOGD(ENA_STORAGE_LOG, "write detection: timestamp %u and rssi %d", timestamp, rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_write_detection");
}

void ena_storage_read_detection(uint32_t index, uint32_t *timestamp, uint8_t *rpi, uint8_t *aem, int *rssi)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_read_detection");
    size_t address = ENA_STORAGE_DETECTIONS_START_ADDRESS + index * ENA_STORAGE_DETECTION_LENGTH;
    *timestamp = ena_storage_read_u32(address);
    address += 4;
    ena_storage_read(address, rpi, ENA_KEY_LENGTH);
    address += ENA_KEY_LENGTH;
    ena_storage_read(address, aem, ENA_AEM_METADATA_LENGTH);
    address += 4;
    *rssi = ena_storage_read_int(address);

    ESP_LOGD(ENA_STORAGE_LOG, "read detection: timestamp %u and rssi %d", *timestamp, *rssi);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
    ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_read_detection");
}

uint8_t ena_storage_read_u8(size_t address)
{
    uint8_t data[1] = {0};
    ena_storage_read(address, (uint8_t *)&data, 1);
    return data[0];
}

uint32_t ena_storage_read_u32(size_t address)
{
    uint8_t data[4] = {0};
    ena_storage_read(address, (uint8_t *)&data, 4);

    uint32_t result = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    return result;
}

void ena_storage_write_u8(size_t address, uint8_t byte)
{
    uint8_t data[1] = {byte};
    ena_storage_write(address, data, 1);
}

void ena_storage_write_u32(size_t address, uint32_t value)
{
    uint8_t *data = malloc(4);
    data[0] = (value & 0x000000ff);
    data[1] = (value & 0x0000ff00) >> 8;
    data[2] = (value & 0x00ff0000) >> 16;
    data[3] = (value & 0xff000000) >> 24;
    ena_storage_write(address, data, 4);
    free(data);
}

int ena_storage_read_int(size_t address)
{
    uint8_t data[sizeof(int)] = {0};
    ena_storage_read(address, (uint8_t *)&data, sizeof(int));
    int result = 0;
    memcpy((int *)&result, (int *)&data, sizeof(int));
    return result;
}

void ena_storage_write_int(size_t address, int value)
{

    uint8_t data[sizeof(int)] = {0};
    memcpy((int *)&data, (int *)&value, sizeof(int));
    ena_storage_write(address, data, sizeof(int));
}

void ena_storage_erase(void)
{
    ESP_LOGD(ENA_STORAGE_LOG, "START ena_storage_erase");
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, PARTITION_NAME);
    assert(partition);
    ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, partition->size));
    ESP_LOGI(PARTITION_NAME, "erase partition!");

    uint8_t *tek_zeros = calloc(ENA_STORAGE_TEK_LENGTH + 1, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_TEK_COUNT_ADDRESS, tek_zeros, ENA_STORAGE_TEK_LENGTH + 1);

    uint8_t *temp_detection_zeros = calloc(ENA_STORAGE_DETECTION_LENGTH + 4, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS, temp_detection_zeros, ENA_STORAGE_DETECTION_LENGTH + 4);

    uint8_t *detection_zeros = calloc(ENA_STORAGE_DETECTION_LENGTH + 4, sizeof(uint8_t));
    ena_storage_write(ENA_STORAGE_DETECTIONS_COUNT_ADDRESS, detection_zeros, ENA_STORAGE_DETECTION_LENGTH + 4);
    ESP_LOGD(ENA_STORAGE_LOG, "END ena_storage_erase");
}

void ena_storage_dump_hash_array(uint8_t *data, size_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (i == 0)
        {
            printf("%0x", data[i]);
        }
        else
        {
            printf(" %0x", data[i]);
        }
    }
}

void ena_storage_dump_tek(void)
{
    uint32_t timestamp;
    uint8_t tek[ENA_KEY_LENGTH] = {0};
    uint8_t tek_count = ena_storage_read_u8(ENA_STORAGE_TEK_COUNT_ADDRESS);
    ESP_LOGD(ENA_STORAGE_LOG, "%u TEKs\n", tek_count);
    printf("#,enin,tek\n");
    for (int i = 0; i < tek_count; i++)
    {

        size_t address = ENA_STORAGE_TEK_START_ADDRESS + i * ENA_STORAGE_TEK_LENGTH;
        timestamp = ena_storage_read_u32(address);
        ena_storage_read(address + 4, tek, ENA_KEY_LENGTH);
        printf("%d,%u,", i, timestamp);
        ena_storage_dump_hash_array(tek, ENA_KEY_LENGTH);
        printf("\n");
    }
}

void ena_storage_dump_temp_detections(void)
{
    uint32_t timestamp;
    uint8_t rpi[ENA_KEY_LENGTH] = {0};
    uint8_t aem[ENA_AEM_METADATA_LENGTH] = {0};
    int rssi;
    uint32_t detection_count = ena_storage_read_u32(ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS);
    ESP_LOGD(ENA_STORAGE_LOG, "%u temporary detections\n", detection_count);
    printf("#,timestamp,rpi,aem,rssi\n");
    for (int i = 0; i < detection_count; i++)
    {
        ena_storage_read_temp_detection(i, &timestamp, rpi, aem, &rssi);
        printf("%d,%u,", i, timestamp);
        ena_storage_dump_hash_array(rpi, ENA_KEY_LENGTH);
        printf(",");
        ena_storage_dump_hash_array(aem, ENA_AEM_METADATA_LENGTH);
        printf(",%d\n", rssi);
    }
}

void ena_storage_dump_detections(void)
{
    uint32_t enin;
    uint8_t rpi[ENA_KEY_LENGTH] = {0};
    uint8_t aem[ENA_AEM_METADATA_LENGTH] = {0};
    int rssi;
    uint32_t detection_count = ena_storage_read_u32(ENA_STORAGE_DETECTIONS_COUNT_ADDRESS);
    ESP_LOGD(ENA_STORAGE_LOG, "%u detections\n", detection_count);
    printf("#,enin,rpi,aem,rssi\n");
    for (int i = 0; i < detection_count; i++)
    {
        ena_storage_read_detection(i, &enin, rpi, aem, &rssi);
        printf("%d,%u,", i, enin);
        ena_storage_dump_hash_array(rpi, ENA_KEY_LENGTH);
        printf(",");
        ena_storage_dump_hash_array(aem, ENA_AEM_METADATA_LENGTH);
        printf(",%d\n", rssi);
    }
}