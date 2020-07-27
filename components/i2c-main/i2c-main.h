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
 * @brief start I2C driver for display and RTC.
 *  
 */
#ifndef _i2c_main_H_
#define _i2c_main_H_

#define I2C_SDA_PIN (21)
#define I2C_SCL_PIN (22)
#define I2C_CLK_SPEED (1000000)

/**
 * @brief initialize main I2C interface
 */
void i2c_main_init();

/**
 * @brief check if I2C interface already initialized
 * 
 * @return
 *      - false I2C not initialized
 *      - true  I2C initialized
 */
bool i2c_is_initialized();

#endif