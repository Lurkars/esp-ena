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
 * @brief connection to german Exposure Notification App (Corona Warn App)
 * 
 * This is for receiving the Exposure Key export for germany and for report own infection inside germany.
 * 
 */
#ifndef _ena_CWA_H_
#define _ena_CWA_H_

#include "esp_err.h"

#define ENA_CWA_LOG "ESP-ENA-cwa" // TAG for Logging

#define ENA_CWA_KEYFILES_URL "https://svc90.main.px.t-online.de/version/v1/diagnosis-keys/country/DE/date/%s"

/**
 * @brief fetch key export for given date
 * 
 * @param[in] date_string the date to fetch the data for
 */
esp_err_t ena_cwa_receive_keys(char *date_string);

/**
 * @brief start ena CWA
 */
void ena_cwa_run(void);

#endif