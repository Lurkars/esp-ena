#include <string.h>

#include "esp_log.h"
#include "ena-crypto.h"
#include "ena-storage.h"

#include "ena-detection.h"

static uint32_t temp_detections_count = 0;
static uint8_t temp_detection_rpi[ENA_STOARGE_TEMP_DETECTIONS_MAX][ENA_KEY_LENGTH] = {{0}};
static uint8_t temp_detection_aem[ENA_STOARGE_TEMP_DETECTIONS_MAX][ENA_AEM_METADATA_LENGTH] = {{0}};
static uint32_t temp_detection_timestamp_first[ENA_STOARGE_TEMP_DETECTIONS_MAX] = {0};
static uint32_t temp_detection_timestamp_last[ENA_STOARGE_TEMP_DETECTIONS_MAX] = {0};
static int temp_detection_rssi_last[ENA_STOARGE_TEMP_DETECTIONS_MAX] = {0};

int ena_get_temp_detection_index(uint8_t *rpi, uint8_t *aem)
{
    for (int i = 0; i < temp_detections_count; i++)
    {
        if (memcmp(temp_detection_rpi[i], rpi, sizeof(ENA_KEY_LENGTH)) == 0 &&
            memcmp(temp_detection_aem[i], aem, sizeof(ENA_AEM_METADATA_LENGTH)) == 0)
        {
            return i;
        }
    }
    return -1;
}

void ena_detections_temp_refresh(uint32_t unix_timestamp)
{
    for (int i = 0; i < temp_detections_count; i++)
    {
        // check for treshold and add permanent detection
        if (temp_detection_timestamp_last[i] - temp_detection_timestamp_first[i] >= ENA_DETECTION_TRESHOLD)
        {
            ESP_LOGD(ENA_DETECTION_LOG, "create detection after treshold");
            ESP_LOG_BUFFER_HEXDUMP(ENA_STORAGE_LOG, temp_detection_rpi[i], ENA_KEY_LENGTH, ESP_LOG_DEBUG);
            ena_storage_write_detection(ena_crypto_enin(temp_detection_timestamp_first[i]), temp_detection_rpi[i],
                                        temp_detection_aem[i], temp_detection_rssi_last[i]);
            ena_storage_remove_temp_detection(i);
        }
        else
            // delete temp detections older than two times time window (two times to be safe, one times time window enough?!)
            if (unix_timestamp - temp_detection_timestamp_first[i] > (ENA_TIME_WINDOW * 2))
        {
            ESP_LOGD(ENA_DETECTION_LOG, "remove old temporary detection %u", i);
            ena_storage_remove_temp_detection(i);
        }
    }

    // update detections
    temp_detections_count = ena_storage_temp_detections_count();
    for (int i = 0; i < temp_detections_count; i++)
    {
        ena_storage_read_temp_detection(i, &temp_detection_timestamp_first[i], temp_detection_rpi[i], temp_detection_aem[i], &temp_detection_rssi_last[i]);
        temp_detection_timestamp_last[i] = temp_detection_timestamp_first[i];
    }

    // DEBUG dump
    ena_storage_dump_tek();
    ena_storage_dump_temp_detections();
    ena_storage_dump_detections();
}

void ena_detection(uint32_t unix_timestamp, uint8_t *rpi, uint8_t *aem, int rssi)
{
    uint32_t detection_index = ena_get_temp_detection_index(rpi, aem);

    if (detection_index == -1)
    {
        temp_detection_timestamp_first[temp_detections_count] = unix_timestamp;
        memcpy(temp_detection_rpi[temp_detections_count], rpi, ENA_KEY_LENGTH);
        memcpy(temp_detection_aem[temp_detections_count], aem, ENA_AEM_METADATA_LENGTH);
        temp_detection_rssi_last[temp_detections_count] = rssi;
        temp_detection_timestamp_last[temp_detections_count] = unix_timestamp;
        detection_index = ena_storage_write_temp_detection(unix_timestamp, rpi, aem, rssi);

        ESP_LOGD(ENA_DETECTION_LOG, "New temporary detection at %d with timestamp %u", temp_detections_count, unix_timestamp);
        ESP_LOG_BUFFER_HEX_LEVEL(ENA_DETECTION_LOG, rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
        ESP_LOG_BUFFER_HEX_LEVEL(ENA_DETECTION_LOG, aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
        ESP_LOGD(ENA_DETECTION_LOG, "RSSI %d", rssi);

        if (detection_index != temp_detections_count)
        {
            ESP_LOGW(ENA_DETECTION_LOG, "last temporary detection index does not match array index!");
        }
        temp_detections_count++;
    }
    else
    {
        temp_detection_rssi_last[detection_index] = rssi;
        temp_detection_timestamp_last[detection_index] = unix_timestamp;
        ESP_LOGD(ENA_DETECTION_LOG, "New Timestamp for temporary detection %d: %u", detection_index, unix_timestamp);
    }
}