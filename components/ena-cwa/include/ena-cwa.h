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
#ifndef _ena_CWA_H_
#define _ena_CWA_H_

#define ENA_CWA_LOG "ESP-ENA-corona-warn-app" // TAG for Logging

#define ENA_CWA_KEYFILES_URL "http://svc90.main.px.t-online.de/version/v1/diagnosis-keys/country/DE/date/%s"

/**
 * @brief fetch key export for given date
 * 
 * @param[in] date_string the date to fetch the data for
 */
void ena_cwa_receive_keys(char *date_string);

#endif