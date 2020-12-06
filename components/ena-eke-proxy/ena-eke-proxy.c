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
#include "time.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ena-crypto.h"
#include "ena-storage.h"
#include "ena-exposure.h"
#include "wifi-controller.h"

#include "ena-eke-proxy.h"

#define HOUR_IN_SECONDS (60 * 60)
#define DAY_IN_SECONDS (HOUR_IN_SECONDS * 24)

extern const uint8_t cert_pem_start[] asm("_binary_cert_pem_start");
extern const uint8_t cert_pem_end[] asm("_binary_cert_pem_end");

static size_t current_page = 0;
static time_t request_sleep = 0;
static uint32_t request_sleep_waiting = 30;
static time_t last_check = 0;
static bool wait_for_request = false;

esp_err_t ena_eke_proxy_fetch_event_handler(esp_http_client_event_t *evt)
{
    static uint8_t *output_buffer;
    static int output_len;
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            if (output_buffer == NULL)
            {
                output_buffer = malloc(esp_http_client_get_content_length(evt->client));
                output_len = 0;
                if (output_buffer == NULL)
                {
                    ESP_LOGE(ENA_EKE_PROXY_LOG, "Failed to allocate memory for output buffer, memory: %d kB", (xPortGetFreeHeapSize() / 1024));
                    return ESP_FAIL;
                }
            }
            memcpy(output_buffer + output_len, evt->data, evt->data_len);
            output_len += evt->data_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        if (output_buffer != NULL && esp_http_client_get_status_code(evt->client) == 200)
        {

            if (output_len % 28 != 0)
            {
                ESP_LOGW(ENA_EKE_PROXY_LOG, "Response length does not match key size! %d", output_len);
            }

            size_t temporary_exposure_keys = (output_len / 28);

            if (temporary_exposure_keys > 0)
            {
                uint32_t start_time = (uint32_t)time(NULL);
                ena_temporary_exposure_key_t temporary_exposure_key;
                int tek_index = 0;

                memcpy(&(temporary_exposure_key.key_data), &output_buffer[tek_index * 28], ENA_KEY_LENGTH);
                memcpy(&(temporary_exposure_key.rolling_start_interval_number), &output_buffer[tek_index * 28 + ENA_KEY_LENGTH], 4);
                memcpy(&(temporary_exposure_key.rolling_period), &output_buffer[tek_index * 28 + ENA_KEY_LENGTH + 4], 4);
                memcpy(&(temporary_exposure_key.days_since_onset_of_symptoms), &output_buffer[tek_index * 28 + ENA_KEY_LENGTH + 8], 4);

                uint32_t timestamp_start = temporary_exposure_key.rolling_start_interval_number * ENA_TIME_WINDOW;
                int min = ena_expore_check_find_min(timestamp_start);

                tek_index = temporary_exposure_keys - 1;
                memcpy(&(temporary_exposure_key.key_data), &output_buffer[tek_index * 28], ENA_KEY_LENGTH);
                memcpy(&(temporary_exposure_key.rolling_start_interval_number), &output_buffer[tek_index * 28 + ENA_KEY_LENGTH], 4);
                memcpy(&(temporary_exposure_key.rolling_period), &output_buffer[tek_index * 28 + ENA_KEY_LENGTH + 4], 4);
                memcpy(&(temporary_exposure_key.days_since_onset_of_symptoms), &output_buffer[tek_index * 28 + ENA_KEY_LENGTH + 8], 4);

                uint32_t timestamp_end = (temporary_exposure_key.rolling_start_interval_number + temporary_exposure_key.rolling_period) * ENA_TIME_WINDOW;
                int max = ena_expore_check_find_max(timestamp_end);

                if (min >= 0 && max >= 0 && min <= max)
                {
                    ESP_LOGI(ENA_EKE_PROXY_LOG, "start check with beacons [%d,%d] for [%u,%u]", min, max, timestamp_start, timestamp_end);
                    ena_beacon_t beacon;
                    for (int y = min; y <= max; y++)
                    {
                        ena_storage_get_beacon(y, &beacon);
                        for (int i = 0; i < temporary_exposure_keys; i++)
                        {
                            memcpy(&(temporary_exposure_key.key_data), &output_buffer[i * 28], ENA_KEY_LENGTH);
                            memcpy(&(temporary_exposure_key.rolling_start_interval_number), &output_buffer[i * 28 + ENA_KEY_LENGTH], 4);
                            memcpy(&(temporary_exposure_key.rolling_period), &output_buffer[i * 28 + ENA_KEY_LENGTH + 4], 4);
                            memcpy(&(temporary_exposure_key.days_since_onset_of_symptoms), &output_buffer[i * 28 + ENA_KEY_LENGTH + 8], 4);
#ifdef DEBUG_ENA_EKE_PROXY
                            ESP_LOGD(ENA_EKE_PROXY_LOG, "key payload: ");
                            ESP_LOG_BUFFER_HEXDUMP(ENA_EKE_PROXY_LOG, &output_buffer[i * 28], 28, ESP_LOG_DEBUG);
                            ESP_LOGD(ENA_EKE_PROXY_LOG, "received key: ");
                            ESP_LOG_BUFFER_HEXDUMP(ENA_EKE_PROXY_LOG, &(temporary_exposure_key.key_data), ENA_KEY_LENGTH, ESP_LOG_DEBUG);
                            ESP_LOG_BUFFER_HEXDUMP(ENA_EKE_PROXY_LOG, &output_buffer[i * 28], ENA_KEY_LENGTH, ESP_LOG_DEBUG);
                            ESP_LOGD(ENA_EKE_PROXY_LOG, "rolling_start_interval_number %u", temporary_exposure_key.rolling_start_interval_number);
                            ESP_LOG_BUFFER_HEXDUMP(ENA_EKE_PROXY_LOG, &output_buffer[i * 28 + ENA_KEY_LENGTH], 4, ESP_LOG_DEBUG);
                            ESP_LOGD(ENA_EKE_PROXY_LOG, "rolling_period %u", temporary_exposure_key.rolling_period);
                            ESP_LOG_BUFFER_HEXDUMP(ENA_EKE_PROXY_LOG, &output_buffer[i * 28 + ENA_KEY_LENGTH + 4], 4, ESP_LOG_DEBUG);
                            ESP_LOGD(ENA_EKE_PROXY_LOG, "days_since_onset_of_symptoms %u", temporary_exposure_key.days_since_onset_of_symptoms);
                            ESP_LOG_BUFFER_HEXDUMP(ENA_EKE_PROXY_LOG, &output_buffer[i * 28 + ENA_KEY_LENGTH + 8], 4, ESP_LOG_DEBUG);
#endif
                            ena_exposure_check(beacon, temporary_exposure_key);
                        }
                    }
                    uint32_t end_time = (uint32_t)time(NULL);
                    ESP_LOGI(ENA_EKE_PROXY_LOG, "check took %u seconds", (end_time - start_time));
                }
                else
                {
                    ESP_LOGD(ENA_EKE_PROXY_LOG, "no matching beacons for [%u,%u]", timestamp_start, timestamp_end);
                }
            }
            else
            {
                ESP_LOGW(ENA_EKE_PROXY_LOG, "no keys in request, should not happen on 200 status!");
            }

            current_page = current_page + 1;
            free(output_buffer);
        }
        else if (esp_http_client_get_status_code(evt->client) == 204)
        {
            // finished!
            if (difftime(time(NULL), last_check) >= DAY_IN_SECONDS)
            {
                last_check = last_check + DAY_IN_SECONDS;
            }
            else
            {
                last_check = last_check + HOUR_IN_SECONDS;
            }
            ena_storage_write_last_exposure_date(last_check);
            current_page = 0;
            request_sleep = 0;
            request_sleep_waiting = 30;
            ena_exposure_summary(ena_exposure_default_config());

            ena_exposure_summary_t *current_summary = ena_exposure_current_summary();
            ESP_LOGD(ENA_EKE_PROXY_LOG, "current summary\nlast update: %u\ndays_since_last_exposure: %d\nnum_exposures: %d\nmax_risk_score: %d\nrisk_score_sum: %d",
                     current_summary->last_update,
                     current_summary->days_since_last_exposure,
                     current_summary->num_exposures,
                     current_summary->max_risk_score,
                     current_summary->risk_score_sum);
        }
        else
        {
            current_page = 0;
            request_sleep = time(NULL) + request_sleep_waiting;
            if (request_sleep_waiting < HOUR_IN_SECONDS)
            {
                request_sleep_waiting = request_sleep_waiting * 3;
            }
        }

        output_buffer = NULL;
        output_len = 0;
        wait_for_request = false;

        break;
    default:
        break;
    }
    return ESP_OK;
}

esp_err_t ena_eke_proxy_receive_keys(char *url)
{
    static int retries = 0;
    wait_for_request = true;
    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 30000,
        .event_handler = ena_eke_proxy_fetch_event_handler,
    };

    if (memcmp(url, "https", 5) == 0)
    {
        config.cert_pem = (char *)cert_pem_start;
    }

    ESP_LOGD(ENA_EKE_PROXY_LOG, "start request: url = %s | memory: %d kB", url, (xPortGetFreeHeapSize() / 1024));
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int content_length = esp_http_client_get_content_length(client);
        ESP_LOGD(ENA_EKE_PROXY_LOG, "finished request: url = %s, status = %d, content_length = %d | memory: %d kB", url,
                 esp_http_client_get_status_code(client),
                 content_length, (xPortGetFreeHeapSize() / 1024));
        retries = 0;
    }

    free(url);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (err != ESP_OK && retries < 6)
    {
        retries = retries + 1;
        ESP_LOGD(ENA_EKE_PROXY_LOG, "retry %d for url = %s", retries, url);
        return ena_eke_proxy_receive_keys(url);
    }

    return err;
}

esp_err_t ena_eke_proxy_receive_daily_keys(char *date_string, size_t page, size_t size)
{
    char *url = malloc(strlen(ENA_EKE_PROXY_KEYFILES_DAILY_URL) + strlen(date_string) + 16);
    sprintf(url, ENA_EKE_PROXY_KEYFILES_DAILY_URL, date_string, page, size);
    return ena_eke_proxy_receive_keys(url);
}

esp_err_t ena_eke_proxy_receive_hourly_keys(char *date_string, uint8_t hour, size_t page, size_t size)
{
    char *url = malloc(strlen(ENA_EKE_PROXY_KEYFILES_HOURLY_URL) + strlen(date_string) + 24);
    sprintf(url, ENA_EKE_PROXY_KEYFILES_HOURLY_URL, date_string, hour, page, size);
    return ena_eke_proxy_receive_keys(url);
}

void ena_eke_proxy_run(void)
{
    static time_t current_time = 0;
    static struct tm current_tm;
    static struct tm last_check_tm;
    static double check_diff = 0;
    static time_t wifi_reconnect = 0;
    static uint32_t wifi_reconnect_waiting = 15;
    current_time = time(NULL);
    last_check = (time_t)ena_storage_read_last_exposure_date();
    check_diff = difftime(current_time, last_check);

    if (check_diff > HOUR_IN_SECONDS && !wait_for_request && current_time > request_sleep)
    {
        if (wifi_controller_connection() == NULL && current_time > wifi_reconnect && wifi_reconnect_waiting < 86400)
        {
            wifi_controller_reconnect(NULL);
            wifi_reconnect = current_time + wifi_reconnect_waiting;
            wifi_reconnect_waiting = wifi_reconnect_waiting * 4;
        }
        else if (wifi_controller_connection() != NULL)
        {
            wifi_reconnect = 0;
            wifi_reconnect_waiting = 15;
            int current_day_offset = check_diff / DAY_IN_SECONDS;

            if (current_day_offset > ENA_EKE_PROXY_MAX_PAST_DAYS)
            {
                current_day_offset = ENA_EKE_PROXY_MAX_PAST_DAYS;
                last_check = (current_time - (DAY_IN_SECONDS * current_day_offset));
            }

            memcpy(&current_tm, gmtime(&current_time), sizeof current_tm);
            memcpy(&last_check_tm, gmtime(&last_check), sizeof last_check_tm);

            if (current_day_offset > 0 || current_tm.tm_mday > last_check_tm.tm_mday || current_tm.tm_mon > last_check_tm.tm_mon)
            {
                last_check_tm.tm_hour = 0;
                if (current_day_offset <= 0)
                {
                    current_day_offset = 1;
                }
            }

            last_check_tm.tm_min = 0;
            last_check_tm.tm_sec = 0;
            last_check = mktime(&last_check_tm);

            esp_err_t err;

            char date_string[11];
            strftime(date_string, 11, ENA_EKE_PROXY_KEYFILES_DAILY_FORMAT, &last_check_tm);

            if (current_day_offset == 0 && ENA_EKE_PROXY_KEYFILES_HOURLY)
            {
                ESP_LOGD(ENA_EKE_PROXY_LOG, "eke-proxy request for /%s/hour/%d?page=%d&size=%d : %d kB, ", date_string, last_check_tm.tm_hour, current_page, ENA_EKE_PROXY_DEFAULT_LIMIT, (xPortGetFreeHeapSize() / 1024));
                err = ena_eke_proxy_receive_hourly_keys(date_string, last_check_tm.tm_hour, current_page, ENA_EKE_PROXY_DEFAULT_LIMIT);
            }
            else
            {
                ESP_LOGD(ENA_EKE_PROXY_LOG, "eke-proxy request for /%s?page=%d&size=%d : %d kB, ", date_string, current_page, ENA_EKE_PROXY_DEFAULT_LIMIT, (xPortGetFreeHeapSize() / 1024));
                err = ena_eke_proxy_receive_daily_keys(date_string, current_page, ENA_EKE_PROXY_DEFAULT_LIMIT);
            }

            if (err != ESP_OK)
            {
                ESP_LOGD(ENA_EKE_PROXY_LOG, "error eke-proxy /%s/%u %d, ", date_string, last_check_tm.tm_hour, (xPortGetFreeHeapSize() / 1024));
            }
        }
    }
}

esp_err_t ena_eke_proxy_fetch_upload_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        break;
    case HTTP_EVENT_ON_FINISH:
        break;
    default:
        break;
    }
    return ESP_OK;
}

esp_err_t ena_eke_proxy_upload(char *token, uint32_t days_since_onset_of_symptoms)
{

    ESP_LOGD(ENA_EKE_PROXY_LOG, "try to upload keys:");
    esp_http_client_config_t config = {
        .url = ENA_EKE_PROXY_KEYFILES_UPLOAD_URL,
        .timeout_ms = 30000,
        .event_handler = ena_eke_proxy_fetch_upload_handler,
        .method = HTTP_METHOD_POST,
    };

    if (memcmp(ENA_EKE_PROXY_KEYFILES_UPLOAD_URL, "https", 5) == 0)
    {
        config.cert_pem = (char *)cert_pem_start;
    }

    uint32_t tek_count = ena_storage_tek_count();
    uint32_t teks_to_send = 0;
    char *output_buffer = malloc(tek_count * 28);
    ena_tek_t tek;
    for (int i = 0; i < tek_count; i++)
    {
        ena_storage_get_tek(i, &tek);
        if (((((uint32_t)time(NULL)) - (tek.enin * ENA_TIME_WINDOW)) / DAY_IN_SECONDS) < ENA_STORAGE_TEK_MAX)
        {
            memcpy(&output_buffer[teks_to_send * 28], &(tek.key_data), ENA_KEY_LENGTH);
            memcpy(&output_buffer[teks_to_send * 28 + ENA_KEY_LENGTH], &(tek.enin), 4);
            uint32_t rolling_period = tek.rolling_period;
            memcpy(&output_buffer[teks_to_send * 28 + ENA_KEY_LENGTH + 4], &rolling_period, 4);
            memcpy(&output_buffer[teks_to_send * 28 + ENA_KEY_LENGTH + 8], &days_since_onset_of_symptoms, 4);
            teks_to_send++;
        }
    }

    ESP_LOG_BUFFER_HEXDUMP(ENA_EKE_PROXY_LOG, output_buffer, teks_to_send * 28, ESP_LOG_DEBUG);

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Authorization", token);
    esp_http_client_set_header(client, "Content-Type", "application/binary");
    esp_http_client_set_header(client, "Accept", "application/octet-stream");
    esp_http_client_set_post_field(client, output_buffer, teks_to_send * 28);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int content_length = esp_http_client_get_content_length(client);
        int status = esp_http_client_get_status_code(client);
        if (status == 200)
        {
            ESP_LOGI(ENA_EKE_PROXY_LOG, "successfully uploaded keys : url = %s, status = %d, content_length = %d", ENA_EKE_PROXY_KEYFILES_UPLOAD_URL, status, content_length);
            err = ESP_OK; 
        }
        else
        {
            ESP_LOGW(ENA_EKE_PROXY_LOG, "failed to upload keys : url = %s, status = %d, content_length = %d", ENA_EKE_PROXY_KEYFILES_UPLOAD_URL, status, content_length);
            err = ESP_FAIL;
        }
    }
    else
    {
        ESP_LOGW(ENA_EKE_PROXY_LOG, "Keyupload failed!");
    }

    free(output_buffer);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return err;
}