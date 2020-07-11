
#include <stdio.h>
#include <time.h>

#include "esp_log.h"

#include "ena-detection.h"

#include "ena-bluetooth-scan.h"

static int scan_status = ENA_SCAN_STATUS_NOT_SCANNING;

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
    esp_ble_gap_cb_param_t *p = (esp_ble_gap_cb_param_t *)param;
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        ESP_LOGI(ENA_SCAN_LOG, "start scanning...");
        break;
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        ESP_LOGI(ENA_SCAN_LOG, "stopped scanning...");
        ena_detections_temp_refresh((uint32_t)time(NULL));
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
        if (p->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT)
        {
            if (p->scan_rst.adv_data_len < 28)
            {
                // adv_data too short for exposure notification
                break;
            }

            int flag_offset = 0;
            // received payload from Google does not contain flag specified in Bluetooth Specification!? So check for length and then add offset
            if (p->scan_rst.adv_data_len == 31)
            {
                // data contains flag
                flag_offset = 3;
            }

            // check for ENA Service UUID: (after flag 0x03 0x03 0x6f 0xfd for ENA service)
            if (p->scan_rst.ble_adv[0 + flag_offset] == 0x3 && p->scan_rst.ble_adv[1 + flag_offset] == 0x3 && p->scan_rst.ble_adv[2 + flag_offset] == 0x6f && p->scan_rst.ble_adv[3 + flag_offset] == 0xfd)
            {
                uint8_t rpi[ENA_KEY_LENGTH] = {0};
                for (int i = 0; i < ENA_KEY_LENGTH; i++)
                {
                    rpi[i] = p->scan_rst.ble_adv[i + 8 + flag_offset];
                }

                uint8_t aem[ENA_AEM_METADATA_LENGTH] = {0};
                for (int i = 0; i < ENA_AEM_METADATA_LENGTH; i++)
                {
                    aem[i] = p->scan_rst.ble_adv[i + ENA_KEY_LENGTH + 8 + flag_offset];
                }

                ena_detection((uint32_t)time(NULL), rpi, aem, p->scan_rst.rssi);
            }
        }
        else if (p->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT)
        {
            scan_status = ENA_SCAN_STATUS_NOT_SCANNING;
            ena_detections_temp_refresh((uint32_t)time(NULL));
            ESP_LOGI(ENA_SCAN_LOG, "finished scanning...");
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
    // init temporary detections
    ena_detections_temp_refresh((uint32_t)time(NULL));
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
