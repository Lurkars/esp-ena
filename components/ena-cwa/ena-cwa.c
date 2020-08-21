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
#include "miniz.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ena-storage.h"
#include "ena-exposure.h"
#include "ena-cwa.h"
#include "wifi-controller.h"

extern uint8_t export_bin_start[] asm("_binary_export_bin_start"); // test data from Google or https://svc90.main.px.t-online.de/version/v1/diagnosis-keys/country/DE/date/2020-07-22
extern uint8_t export_bin_end[] asm("_binary_export_bin_end");

extern const uint8_t telekom_pem_start[] asm("_binary_telekom_pem_start");
extern const uint8_t telekom__pem_end[] asm("_binary_telekom_pem_end");

esp_err_t ena_cwa_http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;
    static int output_len;
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            if (output_buffer == NULL)
            {
                output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client));
                output_len = 0;
                if (output_buffer == NULL)
                {
                    ESP_LOGE(ENA_CWA_LOG, "Failed to allocate memory for output buffer, memory: %d kB", (xPortGetFreeHeapSize() / 1024));
                    return ESP_FAIL;
                }
            }
            memcpy(output_buffer + output_len, evt->data, evt->data_len);
            output_len += evt->data_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        if (output_buffer != NULL)
        {
            ESP_LOGD(ENA_CWA_LOG, "memory: %d kB", (xPortGetFreeHeapSize() / 1024));
            free(output_buffer);
            output_buffer = NULL;
            output_len = 0;
            ESP_ERROR_CHECK_WITHOUT_ABORT(ena_exposure_check_export(export_bin_start, (export_bin_end - export_bin_start)));
        }
        else
        {
            return ESP_FAIL;
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

esp_err_t ena_cwa_receive_keys(char *date_string)
{
    char *url = malloc(strlen(ENA_CWA_KEYFILES_URL) + strlen(date_string));
    sprintf(url, ENA_CWA_KEYFILES_URL, date_string);
    esp_http_client_config_t config = {
        .url = url,
        .cert_pem = (char *)telekom_pem_start,
        .event_handler = ena_cwa_http_event_handler,
    };

    ESP_LOGD(ENA_CWA_LOG, "start memory: %d kB, %s", (xPortGetFreeHeapSize() / 1024), date_string);
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int content_length = esp_http_client_get_content_length(client);
        ESP_LOGD(ENA_CWA_LOG, "Url = %s, Status = %d, content_length = %d", url,
                 esp_http_client_get_status_code(client),
                 content_length);
    }
    free(url);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return err;
}

void ena_cwa_run(void)
{

    static time_t current_time = 0;
    static uint32_t last_check = 0;
    current_time = time(NULL);
    last_check = ena_storage_read_last_exposure_date();
    if ((((uint32_t)current_time) - last_check) / (60 * 60 * 24) > 0 && wifi_controller_connection() != NULL)
    {
        char date_string[11];
        struct tm *time_info;
        time_info = localtime(&current_time);
        time_info->tm_mday--;
        strftime(date_string, 11, "%Y-%m-%d", time_info);
        esp_err_t err = ena_cwa_receive_keys(date_string);
        if (err == ESP_OK)
        {
            ena_storage_write_last_exposure_date((uint32_t)current_time);
            ena_exposure_summary(ena_exposure_default_config());
        }
    }
}