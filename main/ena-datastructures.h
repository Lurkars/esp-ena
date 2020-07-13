/**
 * provide data structure models of Exposure Notification API
 * 
 * 
 */

#ifndef _ena_DATASTRUCTURES_H_
#define _ena_DATASTRUCTURES_H_

#include <stdio.h>
#include "ena-crypto.h"

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

// maybe used later
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

typedef struct 
{
    uint8_t key_data[ENA_KEY_LENGTH];
    uint32_t enin;
    uint8_t rolling_period;
} __packed ena_tek_t;

typedef struct 
{
    uint8_t rpi[ENA_KEY_LENGTH];
    uint8_t aem[ENA_AEM_METADATA_LENGTH];
    uint32_t timestamp_first;
    uint32_t timestamp_last;
    int rssi;
} __packed ena_temp_detection_t;

typedef struct 
{
    uint8_t rpi[ENA_KEY_LENGTH];
    uint8_t aem[ENA_AEM_METADATA_LENGTH];
    uint32_t timestamp;
    int rssi;
} __packed ena_detection_t;

#endif