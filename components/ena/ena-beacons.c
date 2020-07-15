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

#include "ena-crypto.h"
#include "ena-storage.h"

#include "ena-beacons.h"

static uint32_t temp_beacons_count = 0;
static ena_temp_beacon_t temp_beacons[ENA_STORAGE_TEMP_BEACONS_MAX];

ena_beacon_t ena_beacons_convert(ena_temp_beacon_t temp_beacon)
{
    ena_beacon_t beacon;
    memcpy(beacon.rpi, temp_beacon.rpi, ENA_KEY_LENGTH);
    memcpy(beacon.aem, temp_beacon.aem, ENA_AEM_METADATA_LENGTH);
    beacon.timestamp = temp_beacon.timestamp_last;
    beacon.rssi = temp_beacon.rssi;
    return beacon;
}

int ena_get_temp_beacon_index(uint8_t *rpi, uint8_t *aem)
{
    for (int i = 0; i < temp_beacons_count; i++)
    {
        if (memcmp(temp_beacons[i].rpi, rpi, sizeof(ENA_KEY_LENGTH)) == 0 &&
            memcmp(temp_beacons[i].aem, aem, sizeof(ENA_AEM_METADATA_LENGTH)) == 0)
        {
            return i;
        }
    }
    return -1;
}

void ena_beacons_temp_refresh(uint32_t unix_timestamp)
{
    for (int i = temp_beacons_count - 1; i >= 0; i--)
    {
        // check for treshold and add permanent beacon
        if (temp_beacons[i].timestamp_last - temp_beacons[i].timestamp_first >= ENA_BEACON_TRESHOLD)
        {
            ESP_LOGD(ENA_BEACON_LOG, "create beacon after treshold");
            ESP_LOG_BUFFER_HEXDUMP(ENA_BEACON_LOG, temp_beacons[i].rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
            ena_beacon_t beacon = ena_beacons_convert(temp_beacons[i]);
            ena_storage_add_beacon(&beacon);
            ena_storage_remove_temp_beacon(i);
        }
        else
            // delete temp beacons older than two times time window (two times to be safe, one times time window enough?!)
            if (unix_timestamp - temp_beacons[i].timestamp_last > (ENA_TIME_WINDOW * 2))
        {
            ESP_LOGD(ENA_BEACON_LOG, "remove old temporary beacon %u", i);
            ena_storage_remove_temp_beacon(i);
        }
    }

    // update beacons
    temp_beacons_count = ena_storage_temp_beacons_count();
    for (int i = 0; i < temp_beacons_count; i++)
    {
        ena_storage_get_temp_beacon(i, &temp_beacons[i]);
    }

#if (CONFIG_ENA_STORAGE_DUMP)
    // DEBUG dump
    ena_storage_dump_tek();
    ena_storage_dump_temp_beacons();
    ena_storage_dump_beacons();
#endif
}

void ena_beacon(uint32_t unix_timestamp, uint8_t *rpi, uint8_t *aem, int rssi)
{
    uint32_t beacon_index = ena_get_temp_beacon_index(rpi, aem);

    if (beacon_index == -1)
    {
        temp_beacons[temp_beacons_count].timestamp_first = unix_timestamp;
        memcpy(temp_beacons[temp_beacons_count].rpi, rpi, ENA_KEY_LENGTH);
        memcpy(temp_beacons[temp_beacons_count].aem, aem, ENA_AEM_METADATA_LENGTH);
        temp_beacons[temp_beacons_count].rssi = rssi;
        temp_beacons[temp_beacons_count].timestamp_last = unix_timestamp;
        beacon_index = ena_storage_add_temp_beacon(&temp_beacons[temp_beacons_count]);

        ESP_LOGD(ENA_BEACON_LOG, "New temporary beacon at %d with timestamp %u", temp_beacons_count, unix_timestamp);
        ESP_LOG_BUFFER_HEX_LEVEL(ENA_BEACON_LOG, rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
        ESP_LOG_BUFFER_HEX_LEVEL(ENA_BEACON_LOG, aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
        ESP_LOGD(ENA_BEACON_LOG, "RSSI %d", rssi);

        if (beacon_index != temp_beacons_count)
        {
            ESP_LOGW(ENA_BEACON_LOG, "last temporary beacon index does not match array index!");
        }
        temp_beacons_count++;
    }
    else
    {
        temp_beacons[beacon_index].rssi = (temp_beacons[beacon_index].rssi + rssi) / 2;
        temp_beacons[beacon_index].timestamp_last = unix_timestamp;
        ESP_LOGD(ENA_BEACON_LOG, "New Timestamp for temporary beacon %d: %u", beacon_index, unix_timestamp);
        ESP_LOG_BUFFER_HEX_LEVEL(ENA_BEACON_LOG, rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
        ESP_LOG_BUFFER_HEX_LEVEL(ENA_BEACON_LOG, aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
        ena_storage_set_temp_beacon(temp_beacons_count, &temp_beacons[temp_beacons_count]);
    }
}