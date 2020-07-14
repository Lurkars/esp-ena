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

#include "esp_log.h"

#include "ena-datastructures.h"
#include "ena-crypto.h"
#include "ena-storage.h"

#include "ena-detection.h"

static uint32_t temp_detections_count = 0;
static ena_temp_detection_t temp_detections[ENA_STORAGE_TEMP_DETECTIONS_MAX];

ena_detection_t ena_detections_convert(ena_temp_detection_t temp_detection)
{
    ena_detection_t detection;
    memcpy(detection.rpi, temp_detection.rpi, ENA_KEY_LENGTH);
    memcpy(detection.aem, temp_detection.aem, ENA_AEM_METADATA_LENGTH);
    detection.timestamp = temp_detection.timestamp_last;
    detection.rssi = temp_detection.rssi;
    return detection;
}

int ena_get_temp_detection_index(uint8_t *rpi, uint8_t *aem)
{
    for (int i = 0; i < temp_detections_count; i++)
    {
        if (memcmp(temp_detections[i].rpi, rpi, sizeof(ENA_KEY_LENGTH)) == 0 &&
            memcmp(temp_detections[i].aem, aem, sizeof(ENA_AEM_METADATA_LENGTH)) == 0)
        {
            return i;
        }
    }
    return -1;
}

void ena_detections_temp_refresh(uint32_t unix_timestamp)
{
    for (int i = temp_detections_count - 1; i >= 0; i--)
    {
        // check for treshold and add permanent detection
        if (temp_detections[i].timestamp_last - temp_detections[i].timestamp_first >= ENA_DETECTION_TRESHOLD)
        {
            ESP_LOGD(ENA_DETECTION_LOG, "create detection after treshold");
            ESP_LOG_BUFFER_HEXDUMP(ENA_DETECTION_LOG, temp_detections[i].rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
            ena_detection_t detection = ena_detections_convert(temp_detections[i]);
            ena_storage_add_detection(&detection);
            ena_storage_remove_temp_detection(i);
        }
        else
            // delete temp detections older than two times time window (two times to be safe, one times time window enough?!)
            if (unix_timestamp - temp_detections[i].timestamp_last > (ENA_TIME_WINDOW * 2))
        {
            ESP_LOGD(ENA_DETECTION_LOG, "remove old temporary detection %u", i);
            ena_storage_remove_temp_detection(i);
        }
    }

    // update detections
    temp_detections_count = ena_storage_temp_detections_count();
    for (int i = 0; i < temp_detections_count; i++)
    {
        ena_storage_get_temp_detection(i, &temp_detections[i]);
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
        temp_detections[temp_detections_count].timestamp_first = unix_timestamp;
        memcpy(temp_detections[temp_detections_count].rpi, rpi, ENA_KEY_LENGTH);
        memcpy(temp_detections[temp_detections_count].aem, aem, ENA_AEM_METADATA_LENGTH);
        temp_detections[temp_detections_count].rssi = rssi;
        temp_detections[temp_detections_count].timestamp_last = unix_timestamp;
        detection_index = ena_storage_add_temp_detection(&temp_detections[temp_detections_count]);

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
        temp_detections[detection_index].rssi = rssi;
        temp_detections[detection_index].timestamp_last = unix_timestamp;
        ESP_LOGD(ENA_DETECTION_LOG, "New Timestamp for temporary detection %d: %u", detection_index, unix_timestamp);
        ena_storage_set_temp_detection(temp_detections_count, &temp_detections[temp_detections_count]);
    }
}