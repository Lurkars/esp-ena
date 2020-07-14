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

#ifndef _ena_INTERFACE_DATETIME_H_
#define _ena_INTERFACE_DATETIME_H_

typedef enum
{
    ENA_INTERFACE_DATETIME_STATE_YEAR = 0,
    ENA_INTERFACE_DATETIME_STATE_MONTH,
    ENA_INTERFACE_DATETIME_STATE_DAY,
    ENA_INTERFACE_DATETIME_STATE_HOUR,
    ENA_INTERFACE_DATETIME_STATE_MINUTE,
    ENA_INTERFACE_DATETIME_STATE_SECONDS,
} ena_inerface_datetime_state;

void ena_interface_datetime_start(void);

int ena_interface_datetime_state(void);

#endif