
#include <stdio.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "nvs_flash.h"

#include "ena-datastructures.h"
#include "ena-crypto.h"
#include "ena-storage.h"
#include "ena-bluetooth-scan.h"
#include "ena-bluetooth-advertise.h"

#include "ena.h"

static ena_tek_t last_tek;                // last ENIN

void ena_run(void *pvParameter)
{
    uint32_t unix_timestamp = 0;
    uint32_t current_enin = 0;
    while (1)
    {
        unix_timestamp = (uint32_t)time(NULL);
        current_enin = ena_crypto_enin(unix_timestamp);
        if (current_enin - last_tek.enin >= ENA_TEK_ROLLING_PERIOD)
        {
            ena_crypto_tek(last_tek.key_data);
            last_tek.enin = current_enin;
            ena_storage_write_tek(&last_tek);
        }

        // change RPI
        if (unix_timestamp % ENA_TIME_WINDOW == 0)
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
        }

        // scan
        if (unix_timestamp % ENA_SCANNING_INTERVAL == 0 && ena_bluetooth_scan_get_status() == ENA_SCAN_STATUS_NOT_SCANNING)
        {
            ena_bluetooth_scan_start(ENA_SCANNING_TIME);
        }

        // one second loop correct?!
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void ena_start(void)
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

    uint32_t current_enin = ena_crypto_enin((uint32_t)time(NULL));

    uint8_t tek_count = ena_storage_read_last_tek(&last_tek);

    // read last TEK or create new
    if (tek_count == 0 || (current_enin - last_tek.enin) >= ENA_TEK_ROLLING_PERIOD)
    {
        ena_crypto_tek(last_tek.key_data);
        last_tek.enin = ena_crypto_enin((uint32_t)time(NULL));
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
    xTaskCreate(&ena_run, "ena_run", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);
}
