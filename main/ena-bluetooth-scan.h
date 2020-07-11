/**
 * provide bluetooth scanning part of Exposure Notification API v1.2 as defined by Apple/Google
 * 
 * Source documents:
 * 
 * https://blog.google/documents/70/Exposure_Notification_-_Bluetooth_Specification_v1.2.2.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf
 * 
 * 
 * 
 */

#ifndef _ena_BLUETOOTH_SCAN_H_
#define _ena_BLUETOOTH_SCAN_H_

#define ENA_SCAN_LOG "ESP-ENA-scan" // TAG for Logging

#define ENA_SCANNING_TIME 30      // scan for 30 seconds
#define ENA_SCANNING_INTERVAL 300 // scan every 5 minutes

#include "esp_gap_ble_api.h"

typedef enum
{
    ENA_SCAN_STATUS_SCANNING = 0,
    ENA_SCAN_STATUS_NOT_SCANNING,
    ENA_SCAN_STATUS_WAITING,
} ena_bluetooth_scan_status;

void ena_bluetooth_scan_init(void);

void ena_bluetooth_scan_start(uint32_t duration);

void ena_bluetooth_scan_stop(void);

int ena_bluetooth_scan_get_status(void);

#endif