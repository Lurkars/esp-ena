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
 * @brief interface for changing current date and time
 *  
 */
#ifndef _interface__DATETIME_H_
#define _interface__DATETIME_H_

typedef enum
{
    INTERFACE_DATETIME_STATE_YEAR = 0,
    INTERFACE_DATETIME_STATE_MONTH,
    INTERFACE_DATETIME_STATE_DAY,
    INTERFACE_DATETIME_STATE_HOUR,
    INTERFACE_DATETIME_STATE_MINUTE,
    INTERFACE_DATETIME_STATE_SECONDS,
} interface_datetime_state_t;

void interface_datetime_start(void);

int interface_datetime_state(void);

#endif