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

#ifndef _ena_DATASTRUCTURES_H_
#define _ena_DATASTRUCTURES_H_

#include <stdio.h>
#include "ena-crypto.h"

/**
 * @brief different risk levels
 * 
 * not used for now
 */
typedef enum
{
    RISK_LEVEL_INVALID = 0,
    RISK_LEVEL_LOWEST,
    RISK_LEVEL_LOW,
    RISK_LEVEL_LOW_MEDIUM,
    RISK_LEVEL_MEDIUM,
    RISK_LEVEL_MEDIUM_HIGH,
    RISK_LEVEL_HIGH,
    RISK_LEVEL_VERY_HIGH,
    RISK_LEVEL_HIGHEST,
} ena_risklevel_t;

/**
 * @brief configuration for risk score calculation
 * 
 * not used for now
 */
typedef struct
{
    int minimum_risk_score;
    int attenuation_score[8];
    int attenuation_weight;
    int days_sinse_last_exposure_score[8];
    int days_sinse_last_exposure_weight;
    int duration_scores[8];
    float duration_weight;
    int transmission_risk_scores[8];
    float transmission_risk_weight;
    uint8_t duration_at_attenuation_thresholds[2];
} __packed ena_config_t;

/**
 * @brief structure for TEK
 */
typedef struct
{
    uint8_t key_data[ENA_KEY_LENGTH]; // key data for encryption
    uint32_t enin;                    // ENIN marking start of validity
    uint8_t rolling_period;           // period after validity start to mark key as expired
} __packed ena_tek_t;

/**
 * @brief sturcture for a temporary detection
 */
typedef struct
{
    uint8_t rpi[ENA_KEY_LENGTH];          // received RPI of detection
    uint8_t aem[ENA_AEM_METADATA_LENGTH]; // received AEM of detection
    uint32_t timestamp_first;             // timestamp of first recognition
    uint32_t timestamp_last;              // timestamp of last recognition
    int rssi;                             // last measured RSSI
} __packed ena_temp_detection_t;

/**
 * @brief sturcture for a detection
 */
typedef struct
{
    uint8_t rpi[ENA_KEY_LENGTH];          // received RPI of detection
    uint8_t aem[ENA_AEM_METADATA_LENGTH]; // received AEM of detection
    uint32_t timestamp;                   // timestamp of last recognition
    int rssi;                             // last measured RSSI
} __packed ena_detection_t;

#endif