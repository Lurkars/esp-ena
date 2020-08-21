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
 * @brief decode Exposure Key export, compare with stored beacons, calculate score and risk
 * 
 */
#ifndef _ena_EXPOSURE_H_
#define _ena_EXPOSURE_H_

#include <stdio.h>
#include "esp_err.h"
#include "ena-crypto.h"

#define ENA_EXPOSURE_LOG "ESP-ENA-exposure" // TAG for Logging

/**
 * @brief report type
 */
typedef enum
{
    UNKNOWN = 0,
    CONFIRMED_TEST_LOW = 1,
    CONFIRMED_TEST_STANDARD = 2,
    CONFIRMED_TEST_HIGH = 3,
    CONFIRMED_CLINICAL_DIAGNOSIS = 4,
    SELF_REPORT = 5,
    NEGATIVE = 6,
    RECURSIVE = 7,
} ena_report_type_t;

/**
 * @brief duration risk
 */
typedef enum
{
    MINUTES_0 = 0,      // D = 0 min
    MINUTES_5 = 1,      // D <= 5 min
    MINUTES_10 = 2,     // D <= 10 min
    MINUTES_15 = 3,     // D <= 15 min
    MINUTES_20 = 4,     // D <= 20 min
    MINUTES_25 = 5,     // D <= 25 min
    MINUTES_30 = 6,     // D <= 30 min
    MINUTES_LONGER = 7, // D > 30 min
} ena_duration_risk_t;

/**
 * @brief day risk
 */
typedef enum
{
    DAYS_14 = 0, // >= 14 days
    DAYS_13 = 1, // 12-13 days
    DAYS_11 = 2, // 10-11 days
    DAYS_9 = 3,  // 8-9 days
    DAYS_7 = 4,  // 6-7 days
    DAYS_5 = 5,  // 4-5 days
    DAYS_3 = 6,  // 2-3 days
    DAYS_0 = 7,  // 0-1 days
} ena_day_risk_t;

/**
 * @brief attenuation risk
 */
typedef enum
{
    ATTENUATION_73 = 0,    // A > 73 dB
    ATTENUATION_63 = 1,    // 73 >= A > 63
    ATTENUATION_51 = 2,    // 63 >= A > 61
    ATTENUATION_33 = 3,    // 51 >= A > 33
    ATTENUATION_27 = 4,    // 33 >= A > 27
    ATTENUATION_15 = 5,    // 27 >= A > 15
    ATTENUATION_10 = 6,    // 15 >= A > 10
    ATTENUATION_LOWER = 7, // A <= 10
} ena_attenuation_risk_t;

/**
 * @brief risk level from 0-8
 */
typedef enum
{
    ZERO = 0,
    MINIMAL = 1,
    VERY_LOW = 2,
    LOW = 3,
    MEDIUM = 4,
    INCREASED = 5,
    HIGH = 6,
    VERY_HIGH = 7,
    MAXIMUM = 8,
} ena_risk_level_t;

/**
 * @brief structure for exposure configuration
 * 
 * The exposure configuration is used to calculate the risk score.
 */
typedef struct __attribute__((__packed__))
{
    uint8_t transmission_risk_values[8];
    uint8_t duration_risk_values[8];
    uint8_t days_risk_values[8];
    uint8_t attenuation_risk_values[8];
} ena_exposure_config_t;

/**
 * @brief structure for exposure parameter
 * 
 * These parameter are obtained from an exposure information to calculate the risk score.
 */
typedef struct __attribute__((__packed__))
{
    ena_report_type_t report_type;
    int days;
    int duration;
    int attenuation;
} ena_exposure_parameter_t;

/**
 * @brief structure for exposure summary
 * 
 * This represents the current state of all exposures.
 */
typedef struct __attribute__((__packed__))
{
    uint32_t last_update;         // timestamp of last update of exposure data
    int days_since_last_exposure; // Number of days since the most recent exposure.
    int num_exposures;            // Number of all exposure information
    int max_risk_score;           // max. risk score of all exposure information
    int risk_score_sum;           // sum of all risk_scores
} ena_exposure_summary_t;

/**
 * @brief calculate transmission risk score
 * 
 * @param[in] config the exposure configuration used for calculating score
 * @param[in] params the exposure parameter to calculate with
 * 
 * @return
 */
int ena_exposure_transmission_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params);

/**
 * @brief calculate duration risk score
 * 
 * @param[in] config the exposure configuration used for calculating score
 * @param[in] params the exposure parameter to calculate with
 * 
 * @return
 */
int ena_exposure_duration_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params);

/**
 * @brief calculate days risk score
 * 
 * @param[in] config the exposure configuration used for calculating score
 * @param[in] params the exposure parameter to calculate with
 * 
 * @return
 */
int ena_exposure_days_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params);

/**
 * @brief calculate attenuation risk score
 * 
 * @param[in] config the exposure configuration used for calculating score
 * @param[in] params the exposure parameter to calculate with
 * 
 * @return
 */
int ena_exposure_attenuation_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params);

/**
 * @brief calculate overall risk score
 * 
 * @param[in] config the exposure configuration used for calculating score
 * @param[in] params the exposure parameter to calculate with
 * 
 * @return
 */
int ena_exposure_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params);

/**
 * @brief returns the current exposure summary
 * 
 * @param[in] config the exposure configuration used for calculating scores
 */
void ena_exposure_summary(ena_exposure_config_t *config);

/**
 * @brief return the current exposure summary
 * 
 * @return
 *          ena_exposure_summary_t pointer to the current exposure summary
 */
ena_exposure_summary_t *ena_exposure_current_summary(void);

/**
 * @brief return a default exposure configuration
 * 
 * @return
 *      ena_exposure_config_t   default exposure configuration
 */
ena_exposure_config_t *ena_exposure_default_config(void);

/**
 * @brief reads a Temporary Exposue Key Export binary and check for exposures
 * 
 * @param[in] buf the buffer containing the binary data
 * @param[in] size the size of the buffer
 * 
 * @return 
 *      esp_err_t status of reading binary
 */
esp_err_t ena_exposure_check_export(uint8_t *buf, size_t size);

#endif