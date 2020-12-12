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
/**
 * @file
 * 
 * @brief connection to an Exposure Key export proxy
 * 
 * This is for receiving the Exposure Key export from a compatible proxy server and upload own infected keys to it.
 * 
 */
#ifndef _ena_EKE_PROXY_H_
#define _ena_EKE_PROXY_H_

#include "esp_err.h"
#include "ena-crypto.h"

#define ENA_EKE_PROXY_LOG "ESP-ENA-eke-proxy" // TAG for Logging

#define ENA_EKE_PROXY_KEYFILES_DAILY_URL CONFIG_ENA_EKE_PROXY_KEYFILES_DAILY_URL

#ifdef CONFIG_ENA_EKE_PROXY_KEYFILES_HOURLY
#define ENA_EKE_PROXY_KEYFILES_HOURLY true
#else
#define ENA_EKE_PROXY_KEYFILES_HOURLY false
#endif

#if ENA_EKE_PROXY_KEYFILES_HOURLY
#define ENA_EKE_PROXY_KEYFILES_HOURLY_URL CONFIG_ENA_EKE_PROXY_KEYFILES_HOURLY_URL
#else
#define ENA_EKE_PROXY_KEYFILES_HOURLY_URL "/%s/%u/%u/%u"
#endif

#define ENA_EKE_PROXY_KEYFILES_DAILY_FORMAT CONFIG_ENA_EKE_PROXY_KEYFILES_DAILY_FORMAT
#define ENA_EKE_PROXY_KEYFILES_UPLOAD_URL CONFIG_ENA_EKE_PROXY_KEYFILES_UPLOAD_URL
#define ENA_EKE_PROXY_DEFAULT_LIMIT CONFIG_ENA_EKE_PROXY_KEY_LIMIT
#define ENA_EKE_PROXY_MAX_PAST_DAYS CONFIG_ENA_EKE_PROXY_MAX_PAST_DAYS // ENA_STORAGE_TEK_MAX

/**
 * @brief fetch key export from given url
 * 
 * @param[in] url       the url to fetch the data from
 */
esp_err_t ena_eke_proxy_receive_keys(char *url);

/**
 * @brief fetch key export for given date with limit and offset
 * 
 * @param[in] date_string   the date to fetch the data for
 * @param[in] page          the page to request
 * @param[in] size          the size of a page
 */
esp_err_t ena_eke_proxy_receive_daily_keys(char *date_string, size_t page, size_t size);

/**
 * @brief fetch key export for given date with limit and offset
 * 
 * @param[in] date_string   the date to fetch the data for
 * @param[in] hour          the hour to fetch the data for
 * @param[in] page          the page to request
 * @param[in] size          the size of a page
 */
esp_err_t ena_eke_proxy_receive_hourly_keys(char *date_string, uint8_t hour, size_t page, size_t size);

/**
 * @brief run ena eke proxy
 */
void ena_eke_proxy_run(void);

/**
 * @brief Upload own keys to server
 * 
 * @param[in] token                             token for authentication
 * @param[in] days_since_onset_of_symptomstoken days after test/symptoms?
 */
esp_err_t ena_eke_proxy_upload(char *token, uint32_t days_since_onset_of_symptoms);

/**
 * @brief pause requests
 * 
 */
void ena_eke_proxy_pause(void);

/**
 * @brief resume requests
 * 
 */
void ena_eke_proxy_resume(void);

#endif