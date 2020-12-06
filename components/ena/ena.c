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
#include <stdio.h>
#include <time.h>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "nvs_flash.h"

#include "ena-crypto.h"
#include "ena-storage.h"
#include "ena-bluetooth-scan.h"
#include "ena-bluetooth-advertise.h"
#include "ena-beacons.h"

#include "ena.h"

static ena_tek_t last_tek;          // last ENIN
static uint32_t next_rpi_timestamp; // next rpi

void ena_next_rpi_timestamp(uint32_t timestamp)
{
    int random_interval = esp_random() % (2 * ENA_BT_RANDOMIZE_ROTATION_TIMEOUT_INTERVAL);
    if (random_interval > ENA_BT_RANDOMIZE_ROTATION_TIMEOUT_INTERVAL)
    {
        random_interval = ENA_BT_RANDOMIZE_ROTATION_TIMEOUT_INTERVAL - random_interval;
    }
    next_rpi_timestamp = timestamp + ENA_BT_ROTATION_TIMEOUT_INTERVAL + random_interval;
    ESP_LOGD(ENA_LOG, "next rpi at %u (%u from %u)", next_rpi_timestamp, (ENA_BT_ROTATION_TIMEOUT_INTERVAL + random_interval), timestamp);
}

void ena_run(void)
{
    static uint32_t unix_timestamp = 0;
    static uint32_t current_enin = 0;
    unix_timestamp = (uint32_t)time(NULL);
    current_enin = ena_crypto_enin(unix_timestamp);
    if (current_enin - last_tek.enin >= last_tek.rolling_period)
    {
        ena_crypto_tek(last_tek.key_data);
        last_tek.enin = current_enin;
        // validity only to next day 00:00
        last_tek.rolling_period = ENA_TEK_ROLLING_PERIOD - (last_tek.enin % ENA_TEK_ROLLING_PERIOD);
        ena_storage_write_tek(&last_tek);
        // clean up old beacons
        ena_beacons_cleanup(unix_timestamp);
    }

    // change RPI
    if (unix_timestamp >= next_rpi_timestamp)
    {
        if (ena_bluetooth_scan_get_status() == ENA_SCAN_STATUS_SCANNING)
        {
            ena_bluetooth_scan_stop();
        }
        ena_bluetooth_advertise_stop();
        ena_bluetooth_advertise_set_payload(current_enin, last_tek.key_data);
        ena_bluetooth_advertise_start();
        if (ena_bluetooth_scan_get_status() == ENA_SCAN_STATUS_WAITING)
        {
            ena_bluetooth_scan_start(ENA_SCANNING_TIME);
        }
        ena_next_rpi_timestamp(unix_timestamp);
    }

    // scan
    if (unix_timestamp % ENA_SCANNING_INTERVAL == 0 && ena_bluetooth_scan_get_status() == ENA_SCAN_STATUS_NOT_SCANNING)
    {
        ena_bluetooth_scan_start(ENA_SCANNING_TIME);
    }
}

void ena_start(void)
{
#if (CONFIG_ENA_STORAGE_ERASE)
    ena_storage_erase_all();
#endif

#ifdef CONFIG_ENA_RESET_LAST_CHECK
    ena_storage_write_last_exposure_date(0);
#endif

    if (ena_storage_read_last_exposure_date() == 0xFFFFFFFF)
    {
        ena_storage_erase_all();
    }

    // init NVS for BLE
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // init BLE
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE)
    {
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
        while (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE)
        {
        }
    }

    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED)
    {
        ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    }

    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_UNINITIALIZED)
    {
        ESP_ERROR_CHECK(esp_bluedroid_init());
    }
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_INITIALIZED)
    {
        ESP_ERROR_CHECK(esp_bluedroid_enable());
    }

    // new bluetooth address nesseccary?
    uint8_t bt_address[ESP_BD_ADDR_LEN];
    esp_fill_random(bt_address, ESP_BD_ADDR_LEN);
    bt_address[0] |= 0xC0;

    ESP_ERROR_CHECK(esp_ble_gap_set_rand_addr(bt_address));

    ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9));
    ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9));
    ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9));
    ESP_ERROR_CHECK(esp_ble_gap_config_local_privacy(true));

    // init ENA
    ena_crypto_init();

    uint32_t unix_timestamp = (uint32_t)time(NULL);

    uint32_t current_enin = ena_crypto_enin(unix_timestamp);
    uint32_t tek_count = ena_storage_read_last_tek(&last_tek);

    ena_next_rpi_timestamp(unix_timestamp);

    // read last TEK or create new
    if (tek_count == 0 || (current_enin - last_tek.enin) >= last_tek.rolling_period)
    {
        ena_crypto_tek(last_tek.key_data);
        last_tek.enin = ena_crypto_enin(unix_timestamp);
        // validity only to next day 00:00
        last_tek.rolling_period = ENA_TEK_ROLLING_PERIOD - (last_tek.enin % ENA_TEK_ROLLING_PERIOD);
        ena_storage_write_tek(&last_tek);
    }

    // init scan
    ena_bluetooth_scan_init();

    // init and start advertising
    ena_bluetooth_advertise_set_payload(current_enin, last_tek.key_data);
    ena_bluetooth_advertise_start();
    // initial scan on every start
    ena_bluetooth_scan_start(ENA_SCANNING_TIME);

    // what is a good stack size here?
    // xTaskCreate(&ena_run, "ena_run", ENA_RAM, NULL, 5, NULL);
}

void ena_stop(void)
{
    ena_bluetooth_advertise_stop();
    ena_bluetooth_scan_stop();
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
}