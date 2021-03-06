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
 * @brief interface for RTC
 * 
 */
#ifndef _rtc_H_
#define _rtc_H_

#include <time.h>

/**
 * @brief Read time 
 */
void rtc_get_time(struct tm *time);

/**
 * @brief Write time
 */
void rtc_set_time(struct tm *time);

#endif