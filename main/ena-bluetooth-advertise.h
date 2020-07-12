/**
 * provide bluetooth part of Exposure Notification API v1.2 as defined by Apple/Google
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

#ifndef _ena_BLUETOOTH_ADVERTISE_H_
#define _ena_BLUETOOTH_ADVERTISE_H_

#include "esp_gap_ble_api.h"

#define ENA_ADVERTISE_LOG "ESP-ENA-advertise" // TAG for Logging
#define ENA_BLUETOOTH_TAG_DATA 0x1A

void ena_bluetooth_advertise_start(void);

void ena_bluetooth_advertise_set_payload(uint32_t enin, uint8_t *tek);

void ena_bluetooth_advertise_stop(void);

#endif