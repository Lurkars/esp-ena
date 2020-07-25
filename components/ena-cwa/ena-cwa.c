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

#include "esp_log.h"
#include "esp_http_client.h"
#include "miniz.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ena-cwa.h"

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
                    ESP_LOGE(ENA_CWA_LOG, "Failed to allocate memory for output buffer");
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

            size_t zip_image_size = esp_http_client_get_content_length(evt->client);
            // const char *file_name = "export.sig";

            /*
            mz_zip_archive zip_archive;
            mz_zip_archive_file_stat file_stat;
            mz_uint32 file_index;

            memset(&zip_archive, 0, sizeof(zip_archive));
            ESP_LOGD(ENA_CWA_LOG, "memory: %d kB", (xPortGetFreeHeapSize() / 1024));

            vTaskDelay(1000 / portTICK_PERIOD_MS);
            mz_zip_reader_init_mem(&zip_archive, output_buffer, sizeof(output_buffer), 0);
            ESP_LOGD(ENA_CWA_LOG, "memory: %d kB", (xPortGetFreeHeapSize() / 1024));
            mz_zip_reader_locate_file_v2(&zip_archive, file_name, NULL, 0, &file_index);
            ESP_LOGD(ENA_CWA_LOG, "memory: %d kB", (xPortGetFreeHeapSize() / 1024));
            mz_zip_reader_file_stat(&zip_archive, file_index, &file_stat);
            ESP_LOGD(ENA_CWA_LOG, "memory: %d kB", (xPortGetFreeHeapSize() / 1024));
            size_t extracted_size = file_stat.m_uncomp_size;

            ESP_LOGD(ENA_CWA_LOG, "size of export.sig: %d", extracted_size);

            char *file_buffer = malloc(extracted_size);

            mz_zip_reader_extract_to_mem(&zip_archive, file_index, file_buffer, extracted_size, 0);
            ESP_LOGD(ENA_CWA_LOG, "memory: %d kB", (xPortGetFreeHeapSize() / 1024));
            mz_zip_reader_end(&zip_archive);

            ESP_LOG_BUFFER_HEXDUMP(ENA_CWA_LOG, file_buffer, extracted_size, ESP_LOG_DEBUG);

            free(file_buffer);
            */

            mz_zip_archive zip_archive;
            ESP_LOGD(ENA_CWA_LOG, "1 memory: %d kB (min %d kB)", (esp_get_free_heap_size() / 1024), (esp_get_minimum_free_heap_size() / 1024));
            memset(&zip_archive, 0, sizeof(zip_archive));
            ESP_LOGD(ENA_CWA_LOG, "2 memory: %d kB (min %d kB)", (esp_get_free_heap_size() / 1024), (esp_get_minimum_free_heap_size() / 1024));
            mz_zip_reader_init_mem(&zip_archive, output_buffer, zip_image_size, 0);
            ESP_LOGD(ENA_CWA_LOG, "3 memory: %d kB (min %d kB)", (esp_get_free_heap_size() / 1024), (esp_get_minimum_free_heap_size() / 1024));

            mz_zip_reader_end(&zip_archive);

            ESP_LOGD(ENA_CWA_LOG, "4 memory: %d kB (min %d kB)", (esp_get_free_heap_size() / 1024), (esp_get_minimum_free_heap_size() / 1024));
            /* 
            p = mz_zip_reader_extract_file_to_heap(&zip_archive, file_name, &extracted_size, 0);
            ESP_LOGD(ENA_CWA_LOG, "4 memory: %d kB", (esp_get_free_heap_size() / 1024));
            mz_zip_reader_end(&zip_archive);
            ESP_LOG_BUFFER_HEXDUMP(ENA_CWA_LOG, p, extracted_size, ESP_LOG_DEBUG);
            free(p);
            */

            free(output_buffer);
            output_buffer = NULL;
            output_len = 0;
            ESP_LOGD(ENA_CWA_LOG, "memory freed: %d kB (min %d kB)", (esp_get_free_heap_size() / 1024), (esp_get_minimum_free_heap_size() / 1024));
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

void ena_cwa_receive_keys(char *date_string)
{
    char *url = malloc(strlen(ENA_CWA_KEYFILES_URL) + strlen(date_string));
    sprintf(url, ENA_CWA_KEYFILES_URL, date_string);
    esp_http_client_config_t config = {
        .url = url,
        .cert_pem = (char *)telekom_pem_start,
        .event_handler = ena_cwa_http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int content_length = esp_http_client_get_content_length(client);
        ESP_LOGD(ENA_CWA_LOG, "Url = %s, Status = %d, content_length = %d", url,
                 esp_http_client_get_status_code(client),
                 content_length);
    }
    // free(url);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}