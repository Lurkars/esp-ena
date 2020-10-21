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
#include <string.h>
#include <time.h>
#include <limits.h>

#include "esp_err.h"
#include "esp_log.h"

#include "ena-crypto.h"
#include "ena-storage.h"
#include "ena-beacons.h"

#include "ena-exposure.h"

static ena_exposure_summary_t *current_summary;

static ena_exposure_config_t DEFAULT_ENA_EXPOSURE_CONFIG = {
    // transmission_risk_values
    {
        MINIMAL,   // UNKNOWN
        LOW,       // CONFIRMED_TEST_LOW
        MEDIUM,    // CONFIRMED_TEST_STANDARD
        HIGH,      // CONFIRMED_TEST_HIGH
        VERY_HIGH, // CONFIRMED_CLINICAL_DIAGNOSIS
        VERY_LOW,  // SELF_REPORT
        ZERO,      // NEGATIVE
        MINIMAL    // RECURSIVE
    },
    // duration_risk_values
    {
        MINIMAL,   // D = 0 min
        MINIMAL,   // D <= 5 min
        MEDIUM,    // D <= 10 min
        VERY_HIGH, // D <= 15 min
        VERY_HIGH, // D <= 20 min
        MAXIMUM,   // D <= 25 min
        MAXIMUM,   // D <= 30 min
        MAXIMUM    //  D > 30 min
    },
    // days_risk_values
    {
        MINIMAL,  // >= 14 days
        VERY_LOW, // 12-13 days
        VERY_LOW, // 10-11 days
        MEDIUM,   // 8-9 days
        HIGH,     // 6-7 days
        MAXIMUM,  // 4-5 days
        MAXIMUM,  // 2-3 days
        MAXIMUM   // 0-1 days
    },
    // attenuation_risk_values
    {
        MINIMAL, // A > 73 dB
        MINIMAL, // 73 >= A > 63
        MINIMAL, // 63 >= A > 61
        MAXIMUM, // 51 >= A > 33
        MAXIMUM, // 33 >= A > 27
        MAXIMUM, // 27 >= A > 15
        MAXIMUM, // 15 >= A > 10
        MAXIMUM  // A <= 10
    },
};

int ena_exposure_transmission_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params)
{
    return config->transmission_risk_values[params.report_type];
}

int ena_exposure_duration_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params)
{
    // calc duration level
    int duration_level = MINUTES_0;
    if (params.duration > 0)
    {
        if (params.duration <= 5)
        {
            duration_level = MINUTES_5;
        }
        else if (params.duration <= 10)
        {
            duration_level = MINUTES_10;
        }
        if (params.duration <= 15)
        {
            duration_level = MINUTES_15;
        }
        if (params.duration <= 20)
        {
            duration_level = MINUTES_20;
        }
        if (params.duration <= 25)
        {
            duration_level = MINUTES_25;
        }
        if (params.duration <= 30)
        {
            duration_level = MINUTES_30;
        }
        else
        {
            duration_level = MINUTES_LONGER;
        }
    }

    return config->duration_risk_values[duration_level];
}

int ena_exposure_days_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params)
{
    // calc days level
    int days_level = DAYS_14;

    if (params.days < 2)
    {
        days_level = DAYS_0;
    }
    else if (params.days < 4)
    {
        days_level = DAYS_3;
    }
    else if (params.days < 6)
    {
        days_level = DAYS_5;
    }
    else if (params.days < 8)
    {
        days_level = DAYS_7;
    }
    else if (params.days < 10)
    {
        days_level = DAYS_9;
    }
    else if (params.days < 12)
    {
        days_level = DAYS_11;
    }
    else if (params.days < 14)
    {
        days_level = DAYS_13;
    }

    return config->days_risk_values[days_level];
}

int ena_exposure_attenuation_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params)
{
    // calc attenuation level
    int attenuation_level = ATTENUATION_73;

    if (params.attenuation <= 10)
    {
        attenuation_level = ATTENUATION_LOWER;
    }
    else if (params.attenuation <= 15)
    {
        attenuation_level = ATTENUATION_10;
    }
    else if (params.attenuation <= 27)
    {
        attenuation_level = ATTENUATION_15;
    }
    else if (params.attenuation <= 33)
    {
        attenuation_level = ATTENUATION_27;
    }
    else if (params.attenuation <= 51)
    {
        attenuation_level = ATTENUATION_33;
    }
    else if (params.attenuation <= 63)
    {
        attenuation_level = ATTENUATION_51;
    }
    else if (params.attenuation <= 73)
    {
        attenuation_level = ATTENUATION_63;
    }

    return config->attenuation_risk_values[attenuation_level];
}

int ena_exposure_risk_score(ena_exposure_config_t *config, ena_exposure_parameter_t params)
{
    int score = 1;
    score *= ena_exposure_transmission_risk_score(config, params);

    score *= ena_exposure_duration_risk_score(config, params);

    score *= ena_exposure_days_risk_score(config, params);

    score *= ena_exposure_attenuation_risk_score(config, params);

    if (score > 255)
    {
        score = 255;
    }

    return score;
}

void ena_exposure_summary(ena_exposure_config_t *config)
{
    uint32_t count = ena_storage_exposure_information_count();
    uint32_t current_time = (uint32_t)time(NULL);

    if (current_summary == NULL)
    {
        current_summary = malloc(sizeof(ena_exposure_summary_t));
    }

    current_summary->last_update = ena_storage_read_last_exposure_date();
    current_summary->days_since_last_exposure = INT_MAX;
    current_summary->max_risk_score = 0;
    current_summary->risk_score_sum = 0;
    current_summary->num_exposures = count;

    if (count == 0)
    {
        current_summary->days_since_last_exposure = -1;
    }

    ena_exposure_information_t exposure_info;
    ena_exposure_parameter_t params;
    for (int i = 0; i < count; i++)
    {
        ena_storage_get_exposure_information(i, &exposure_info);
        params.days = (current_time - exposure_info.day) / (60 * 60 * 24); // difference in days
        if (params.days < current_summary->days_since_last_exposure)
        {
            current_summary->days_since_last_exposure = params.days;
        }
        params.duration = exposure_info.duration_minutes;
        params.attenuation = exposure_info.typical_attenuation;
        int score = ena_exposure_risk_score(config, params);
        if (score > current_summary->max_risk_score)
        {
            current_summary->max_risk_score = score;
        }
        current_summary->risk_score_sum += score;
    }
}

ena_exposure_summary_t *ena_exposure_current_summary(void)
{
    if (current_summary == NULL)
    {
        ena_exposure_summary(ena_exposure_default_config());
    }

    return current_summary;
}

ena_exposure_config_t *ena_exposure_default_config(void)
{
    return &DEFAULT_ENA_EXPOSURE_CONFIG;
}

void ena_exposure_check(ena_beacon_t beacon, ena_temporary_exposure_key_t temporary_exposure_key)
{
    uint32_t timestamp_day_start = temporary_exposure_key.rolling_start_interval_number * ENA_TIME_WINDOW;
    uint32_t timestamp_day_end = temporary_exposure_key.rolling_start_interval_number * ENA_TIME_WINDOW + temporary_exposure_key.rolling_period * ENA_TIME_WINDOW;

    if (beacon.timestamp_first > timestamp_day_start && beacon.timestamp_last < timestamp_day_end)
    {
        ESP_LOGD(ENA_EXPOSURE_LOG, "matched timestamps!");

        ESP_LOGD(ENA_EXPOSURE_LOG, "Beacon: %u,%u,%d", beacon.timestamp_first, beacon.timestamp_last, beacon.rssi);
        ESP_LOG_BUFFER_HEXDUMP(ENA_EXPOSURE_LOG, beacon.rpi, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
        ESP_LOG_BUFFER_HEXDUMP(ENA_EXPOSURE_LOG, beacon.aem, ENA_AEM_METADATA_LENGTH, ESP_LOG_DEBUG);

        ESP_LOGD(ENA_EXPOSURE_LOG, "Key: %u,%u,%d", timestamp_day_start, timestamp_day_end, temporary_exposure_key.rolling_period);
        ESP_LOG_BUFFER_HEXDUMP(ENA_EXPOSURE_LOG, temporary_exposure_key.key_data, ENA_KEY_LENGTH, ESP_LOG_DEBUG);
        bool match = false;
        ena_exposure_information_t exposure_info;
        exposure_info.duration_minutes = 0;
        exposure_info.min_attenuation = INT_MAX;
        exposure_info.typical_attenuation = 0;
        exposure_info.report_type = temporary_exposure_key.report_type;
        uint8_t rpi[ENA_KEY_LENGTH];
        uint8_t rpik[ENA_KEY_LENGTH];
        ena_crypto_rpik(rpik, temporary_exposure_key.key_data);

        for (int i = 0; i < temporary_exposure_key.rolling_period; i++)
        {
            ena_crypto_rpi(rpi, rpik, temporary_exposure_key.rolling_start_interval_number + i);
            if (memcmp(beacon.rpi, rpi, sizeof(ENA_KEY_LENGTH)) == 0)
            {
                match = true;
                exposure_info.day = timestamp_day_start;
                exposure_info.duration_minutes += (ENA_BEACON_TRESHOLD / 60);
                exposure_info.typical_attenuation = (exposure_info.typical_attenuation + beacon.rssi) / 2;
                if (beacon.rssi < exposure_info.min_attenuation)
                {
                    exposure_info.min_attenuation = beacon.rssi;
                }
            }
        }

        if (match)
        {
            ena_storage_add_exposure_information(&exposure_info);
        }
    }
}

void ena_exposure_check_temporary_exposure_key(ena_temporary_exposure_key_t temporary_exposure_key)
{
    ena_beacon_t beacon;
    uint32_t beacons_count = ena_storage_beacons_count();
    for (int y = 0; y < beacons_count; y++)
    {
        ena_storage_get_beacon(y, &beacon);
        ena_exposure_check(beacon, temporary_exposure_key);
    }
}