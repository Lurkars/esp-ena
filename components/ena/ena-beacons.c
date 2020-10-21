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
#include <time.h>

#include "esp_log.h"

#include "ena-crypto.h"
#include "ena-storage.h"

#include "ena-beacons.h"

static uint32_t temp_beacons_count = 0;
static ena_beacon_t temp_beacons[ENA_STORAGE_TEMP_BEACONS_MAX];

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
            ena_storage_add_beacon(&temp_beacons[i]);
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
    ena_storage_dump_teks();
    ena_storage_dump_exposure_information();
    ena_storage_dump_temp_beacons();
    ena_storage_dump_beacons();
#endif
}

void ena_beacons_cleanup(uint32_t unix_timestamp)
{
    uint32_t count = ena_storage_beacons_count();
    ena_beacon_t beacon;
    for (int i = count - 1; i >= 0; i--)
    {
        ena_storage_get_beacon(i, &beacon);
        if (((unix_timestamp - beacon.timestamp_last) / (60 * 60 * 24)) > ENA_BEACON_CLEANUP_TRESHOLD)
        {
            ena_storage_remove_beacon(i);
        }
    }
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
        ESP_LOGD(ENA_BEACON_LOG, "new temporary beacon %d at %u", temp_beacons_count, unix_timestamp);
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
        ESP_LOGD(ENA_BEACON_LOG, "update temporary beacon %d at %u", beacon_index, unix_timestamp);
        ESP_LOG_BUFFER_HEX_LEVEL(ENA_BEACON_LOG, temp_beacons[beacon_index].rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
        ESP_LOG_BUFFER_HEX_LEVEL(ENA_BEACON_LOG, temp_beacons[beacon_index].aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);
        ESP_LOGD(ENA_BEACON_LOG, "RSSI %d", temp_beacons[beacon_index].rssi);
        ena_storage_set_temp_beacon(beacon_index, &temp_beacons[beacon_index]);
    }
}