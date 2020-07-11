
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

#include "ena.h"

static uint32_t last_enin;                // last ENIN
static uint8_t tek[ENA_KEY_LENGTH] = {0}; // current TEK

void ena_init(void)
{
    // init NVS for BLE
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // init BLE
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9));
    ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9));
    ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9));
    ESP_ERROR_CHECK(esp_ble_gap_config_local_privacy(true));

    /* nesseccary?
    // new bluetooth address
    uint8_t bt_address[ESP_BD_ADDR_LEN];
    esp_fill_random(bt_address, ESP_BD_ADDR_LEN);
    bt_address[0] |= 0xC0;

    ESP_ERROR_CHECK(esp_ble_gap_set_rand_addr(bt_address));
*/
    // init ENA
    ena_crypto_init();

    uint32_t current_enin = ena_crypto_enin((uint32_t)time(NULL));

    last_enin = ena_storage_read_enin();

    // read last TEK or create new
    if (ena_storage_read_u8(ENA_STORAGE_TEK_COUNT_ADDRESS) > 0 && (current_enin - last_enin) < ENA_TEK_ROLLING_PERIOD)
    {
        ena_storage_read_tek(tek);
    }
    else
    {
        ena_crypto_tek(tek);
        ena_storage_write_tek(ena_crypto_enin((uint32_t)time(NULL)), tek);
        last_enin = ena_storage_read_enin();
    }

    // init scan
    ena_bluetooth_scan_init();

    // init and start advertising
    ena_bluetooth_advertise_set_payload(current_enin, tek);
    ena_bluetooth_advertise_start();
    // initial scan on every start
    ena_bluetooth_scan_start(ENA_SCANNING_TIME);
}

void ena_run(void)
{
    uint32_t unix_timestamp = (uint32_t)time(NULL);
    uint32_t current_enin = ena_crypto_enin(unix_timestamp);
    if (current_enin - last_enin >= ENA_TEK_ROLLING_PERIOD)
    {
        ena_crypto_tek(tek);
        ena_storage_write_tek(current_enin, tek);
        last_enin = current_enin;
    }

    // change RPI
    if (unix_timestamp % ENA_TIME_WINDOW == 0)
    {

        //

        if (ena_bluetooth_scan_get_status() == ENA_SCAN_STATUS_SCANNING)
        {
            ena_bluetooth_scan_stop();
        }
        ena_bluetooth_advertise_stop();
        ena_bluetooth_advertise_set_payload(current_enin, tek);
        ena_bluetooth_advertise_start();
        if (ena_bluetooth_scan_get_status() == ENA_SCAN_STATUS_WAITING)
        {
            ena_bluetooth_scan_start(ENA_SCANNING_TIME);
        }
    }

    // scan
    if (unix_timestamp % ENA_SCANNING_INTERVAL == 0 && ena_bluetooth_scan_get_status() == ENA_SCAN_STATUS_NOT_SCANNING)
    {
        ena_bluetooth_scan_start(ENA_SCANNING_TIME);
    }
}
