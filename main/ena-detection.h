/**
 * combine bluetooth and crypto parts to build EXPOSURE NOTIFICATION
 * 
 */

#ifndef _ena_DETECTION_H_
#define _ena_DETECTION_H_

#define ENA_DETECTION_LOG "ESP-ENA-detection" // TAG for Logging

#define ENA_DETECTION_TRESHOLD 300 // meet for longer than 5 minutes

#include "ena-crypto.h"

void ena_detections_temp_refresh(uint32_t unix_timestamp);

void ena_detection(uint32_t unix_timestamp, uint8_t rpi[], uint8_t aem[], int rssi);

#endif