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
 * @brief execute interface commands via simple push buttons
 *  
 */
#ifndef _button_input_H_
#define _button_input_H_

#define BUTTON_RST GPIO_NUM_32
#define BUTTON_SET GPIO_NUM_33
#define BUTTON_MID GPIO_NUM_25
#define BUTTON_RHT GPIO_NUM_26
#define BUTTON_LFT GPIO_NUM_27
#define BUTTON_DWN GPIO_NUM_14
#define BUTTON_UP GPIO_NUM_12

/**
 * @brief     
 * 
 * 
 */
void custom_input_start(void);

#endif