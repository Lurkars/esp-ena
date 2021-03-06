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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi-controller.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;

static bool initialized = false;
static bool connecting = false;

static wifi_ap_record_t current_wifi_ap;

static wifi_callback wifi_connected_callback;
static wifi_callback wifi_scan_callback;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        connecting = false;
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        if (wifi_connected_callback != NULL)
        {
            (*wifi_connected_callback)();
        }
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        connecting = false;
        heap_caps_check_integrity_all(true);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        if (wifi_scan_callback != NULL)
        {
            (*wifi_scan_callback)();
        }
    }
    else
    {
        ESP_LOGD(WIFI_LOG, "other wifi event: event base %s, event id %d", event_base, event_id);
    }
}

void wifi_controller_init(void)
{
    // init NVS for WIFI
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    initialized = true;
}

void wifi_controller_scan(wifi_ap_record_t *ap_info, uint16_t *ap_count, wifi_callback callback)
{
    if (!initialized)
    {
        wifi_controller_init();
    }

    wifi_scan_callback = callback;

    uint16_t number = 10;

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_stop());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(ap_count));
    ESP_ERROR_CHECK(esp_wifi_stop());
}

esp_err_t wifi_controller_connect(wifi_config_t wifi_config, wifi_callback callback)
{
    if (connecting)
    {
        return ESP_ERR_NOT_SUPPORTED;
    }
    connecting = true;
    wifi_connected_callback = callback;
    if (!initialized)
    {
        wifi_controller_init();
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_stop());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_event_handler_instance_t instance_any_id;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_wifi_event_group = xEventGroupCreate();

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    vEventGroupDelete(s_wifi_event_group);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGV(WIFI_LOG, "connected to ap SSID:%s", wifi_config.sta.ssid);
        return ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(WIFI_LOG, "Failed to connect to SSID:%s with PASSWORD:%s", wifi_config.sta.ssid, wifi_config.sta.password);
        return ESP_ERR_INVALID_ARG;
    }
    else
    {
        ESP_LOGE(WIFI_LOG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}

esp_err_t wifi_controller_reconnect(wifi_callback callback)
{
    if (connecting)
    {
        return ESP_ERR_NOT_SUPPORTED;
    }
    connecting = true;

    wifi_connected_callback = callback;
    if (!initialized)
    {
        wifi_controller_init();
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_stop());
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_event_handler_instance_t instance_any_id;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_wifi_event_group = xEventGroupCreate();

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    vEventGroupDelete(s_wifi_event_group);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGV(WIFI_LOG, "reconnected to last ap");
        return ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGD(WIFI_LOG, "Failed to reconnect to last ap");
        return ESP_ERR_INVALID_ARG;
    }
    else
    {
        ESP_LOGE(WIFI_LOG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}

esp_err_t wifi_controller_disconnect(void)
{
    if (esp_wifi_sta_get_ap_info(&current_wifi_ap) == ESP_OK)
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }
    return ESP_OK;
}

wifi_ap_record_t *wifi_controller_connection(void)
{
    if (esp_wifi_sta_get_ap_info(&current_wifi_ap) == ESP_OK)
    {
        ESP_LOGD(WIFI_LOG, "Current AP: %s", current_wifi_ap.ssid);
        return &current_wifi_ap;
    }
    return NULL;
}