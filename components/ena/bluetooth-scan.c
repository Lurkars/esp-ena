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
#include "esp_gap_ble_api.h"

#include "ena-crypto.h"
#include "ena-beacons.h"

#include "ena-bluetooth-scan.h"

static int scan_status = ENA_SCAN_STATUS_NOT_SCANNING;

static const uint16_t ENA_SERVICE_UUID = 0xFD6F;

static esp_ble_scan_params_t ena_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_RANDOM,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50, // don't know good parameters, just copied
    .scan_window = 0x30,   // don't know good parameters, just copied
    .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE,
};

void ena_bluetooth_scan_event_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{

    uint32_t unix_timestamp = (uint32_t)time(NULL);
    esp_ble_gap_cb_param_t *p = (esp_ble_gap_cb_param_t *)param;
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        ESP_LOGD(ENA_SCAN_LOG, "start scanning...");
        break;
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        ESP_LOGD(ENA_SCAN_LOG, "stopped scanning...");
        ena_beacons_temp_refresh(unix_timestamp);
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
        if (p->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT)
        {
            uint8_t service_uuid_length = 0;
            uint8_t *service_uuid_data = esp_ble_resolve_adv_data(p->scan_rst.ble_adv, 0x03, &service_uuid_length);
            // check for ENA Service UUID
            if (service_uuid_length == sizeof(ENA_SERVICE_UUID) && memcmp(service_uuid_data, &ENA_SERVICE_UUID, service_uuid_length) == 0)
            {
                uint8_t service_data_length = 0;
                uint8_t *service_data = esp_ble_resolve_adv_data(p->scan_rst.ble_adv, 0x16, &service_data_length);
                if (service_data_length != (sizeof(ENA_SERVICE_UUID) + ENA_KEY_LENGTH + ENA_AEM_METADATA_LENGTH))
                {
                    ESP_LOGW(ENA_SCAN_LOG, "received ENA Service with invalid payload");
                    break;
                }

                uint8_t *rpi = malloc(ENA_KEY_LENGTH);
                memcpy(rpi, &service_data[sizeof(ENA_SERVICE_UUID)], ENA_KEY_LENGTH);
                uint8_t *aem = malloc(ENA_AEM_METADATA_LENGTH);
                memcpy(aem, &service_data[sizeof(ENA_SERVICE_UUID) + ENA_KEY_LENGTH], ENA_AEM_METADATA_LENGTH);
                ena_beacon(unix_timestamp, rpi, aem, p->scan_rst.rssi);
                free(rpi);
                free(aem);
            }
        }
        else if (p->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT)
        {
            scan_status = ENA_SCAN_STATUS_NOT_SCANNING;
            ena_beacons_temp_refresh(unix_timestamp);
            ESP_LOGD(ENA_SCAN_LOG, "finished scanning...");
        }
        break;
    default:
        // nothing
        break;
    }
}

void ena_bluetooth_scan_init(void)
{
    ESP_ERROR_CHECK(esp_ble_gap_set_scan_params(&ena_scan_params));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(ena_bluetooth_scan_event_callback));
    // init temporary beacons
    ena_beacons_temp_refresh((uint32_t)time(NULL));
}

void ena_bluetooth_scan_start(uint32_t duration)
{
    scan_status = ENA_SCAN_STATUS_SCANNING;
    ESP_ERROR_CHECK(esp_ble_gap_start_scanning(duration));
}

void ena_bluetooth_scan_stop(void)
{
    scan_status = ENA_SCAN_STATUS_WAITING;
    ESP_ERROR_CHECK(esp_ble_gap_stop_scanning());
}

int ena_bluetooth_scan_get_status(void)
{
    return scan_status;
}
