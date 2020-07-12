#include "esp_log.h"
#include "esp_bt.h"

#include "ena-crypto.h"

#include "ena-bluetooth-advertise.h"

static esp_ble_adv_params_t ena_adv_params = {
    .adv_int_min = 0x140, // 200 ms
    .adv_int_max = 0x190, // 250 ms
    .adv_type = ADV_TYPE_NONCONN_IND,
    .own_addr_type = BLE_ADDR_TYPE_RANDOM,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

void ena_bluetooth_advertise_start(void)
{
    ESP_ERROR_CHECK(esp_ble_gap_start_advertising(&ena_adv_params));
}

void ena_bluetooth_advertise_set_payload(uint32_t enin, uint8_t *tek)
{
    uint8_t rpik[ENA_KEY_LENGTH] = {0};
    uint8_t rpi[ENA_KEY_LENGTH] = {0};
    uint8_t aemk[ENA_KEY_LENGTH] = {0};
    uint8_t aem[ENA_AEM_METADATA_LENGTH] = {0};

    ena_crypto_rpik(rpik, tek);

    ena_crypto_rpi(rpi, rpik, enin);

    ena_crypto_aemk(aemk, tek);

    ena_crypto_aem(aem, aemk, rpi, esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_ADV));

    uint8_t adv_raw_data[31];
    // FLAG??? skipped on sniffed android packages!?
    adv_raw_data[0] = 0x02;
    adv_raw_data[1] = 0x01;
    adv_raw_data[2] = ENA_BLUETOOTH_TAG_DATA;

    // SERVICE UUID
    adv_raw_data[3] = 0x03;
    adv_raw_data[4] = 0x03;
    adv_raw_data[5] = 0x6F;
    adv_raw_data[6] = 0xFD;

    // SERVICE DATA
    adv_raw_data[7] = 0x17;
    adv_raw_data[8] = 0x16;

    adv_raw_data[9] = 0x6F;
    adv_raw_data[10] = 0xFD;

    for (int i = 0; i < ENA_KEY_LENGTH; i++)
    {
        adv_raw_data[i + 11] = rpi[i];
    }
    for (int i = 0; i < ENA_AEM_METADATA_LENGTH; i++)
    {
        adv_raw_data[i + ENA_KEY_LENGTH + 11] = aem[i];
    }

    esp_ble_gap_config_adv_data_raw(adv_raw_data, sizeof(adv_raw_data));

    ESP_LOGI(ENA_ADVERTISE_LOG, "payload for ENIN %u", enin);
    ESP_LOG_BUFFER_HEXDUMP(ENA_ADVERTISE_LOG, adv_raw_data, sizeof(adv_raw_data), ESP_LOG_DEBUG);
}

void ena_bluetooth_advertise_stop(void)
{
    ESP_ERROR_CHECK(esp_ble_gap_stop_advertising());
}