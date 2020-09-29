#include <string.h>
#include <time.h>
#include <limits.h>

#include "esp_err.h"
#include "esp_log.h"

#include "ena-exposure.h"

#include "pb_decode.h"
#include "TemporaryExposureKeyExport.pb.h"

static const char kFileHeader[] = "EK Export v1    ";
static size_t kFileHeaderSize = sizeof(kFileHeader) - 1;

bool ena_binary_export_decode_key_data(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    uint8_t *key_data = (uint8_t *)*arg;

    if (!pb_read(stream, key_data, stream->bytes_left))
    {
        ESP_LOGW(ENA_EXPOSURE_LOG, "Decoding failed: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    return true;
}

bool ena_binary_export_decode_key(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    uint8_t key_data[ENA_KEY_LENGTH] = {0};
    TemporaryExposureKey tek = TemporaryExposureKey_init_zero;

    tek.key_data = (pb_callback_t){
        .funcs.decode = ena_binary_export_decode_key_data,
        .arg = &key_data,
    };

    if (!pb_decode(stream, TemporaryExposureKey_fields, &tek))
    {
        ESP_LOGW(ENA_EXPOSURE_LOG, "Decoding failed: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    ESP_LOGD(ENA_EXPOSURE_LOG,
             "check reported tek: rolling_start_interval_number %d, rolling_period %d, days_since_last_exposure %d, report_type %d, transmission_risk_values %d",
             tek.rolling_start_interval_number, tek.rolling_period, tek.days_since_onset_of_symptoms, tek.report_type, tek.transmission_risk_level);

    ESP_LOG_BUFFER_HEXDUMP(ENA_EXPOSURE_LOG, &key_data, ENA_KEY_LENGTH, ESP_LOG_DEBUG);

    ena_temporary_exposure_key_t temporary_exposure_key = {
        .transmission_risk_level = tek.transmission_risk_level,
        .rolling_start_interval_number = tek.rolling_start_interval_number,
        .rolling_period = tek.rolling_period,
        .report_type = tek.report_type,
        .days_since_onset_of_symptoms = tek.days_since_onset_of_symptoms,
    };

    memcpy(temporary_exposure_key.key_data, key_data, ENA_KEY_LENGTH);

    ena_exposure_check_temporary_exposure_key(temporary_exposure_key);

    return true;
}

esp_err_t ena_binary_export_check_export(uint8_t *buf, size_t size)
{
    // validate header
    if (memcmp(kFileHeader, buf, kFileHeaderSize) != 0)
    {
        ESP_LOGW(ENA_EXPOSURE_LOG, "Wrong or missing header!");
        return ESP_FAIL;
    }

    TemporaryExposureKeyExport tek_export = TemporaryExposureKeyExport_init_zero;

    tek_export.keys = (pb_callback_t){
        .funcs.decode = ena_binary_export_decode_key,
    };

    pb_istream_t stream = pb_istream_from_buffer(&buf[kFileHeaderSize], (size - kFileHeaderSize));

    if (!pb_decode(&stream, TemporaryExposureKeyExport_fields, &tek_export))
    {
        ESP_LOGW(ENA_EXPOSURE_LOG, "Decoding failed: %s\n", PB_GET_ERROR(&stream));
        return ESP_FAIL;
    }

    return ESP_OK;
}