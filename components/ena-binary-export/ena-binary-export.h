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
 * @brief decode Exposure Key export and run exposure check
 * 
 */
#ifndef _ena_BINARY_H_
#define _ena_BINARY_H_

#include <stdio.h>
#include "esp_err.h"

/**
 * @brief reads a Temporary Exposue Key Export binary and check for exposures
 * 
 * @param[in] buf the buffer containing the binary data
 * @param[in] size the size of the buffer
 * 
 * @return 
 *      esp_err_t status of reading binary
 */
esp_err_t ena_binary_export_check_export(uint8_t *buf, size_t size);

#endif