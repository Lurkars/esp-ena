#ifndef _ena_STORAGE_H_
#define _ena_STORAGE_H_

#include "ena-crypto.h"
#include "ena-datastructures.h"

#define ENA_STORAGE_LOG "ESP-ENA-storage" // TAG for Logging
#define PARTITION_NAME "ena"
#define ENA_STOARGE_TEK_STORE_PERIOD (14)      // Period of storing TEKs                                                                            // length of a stored detection -> RPI keysize + AEM size + 4 Bytes for ENIN + 4 Bytes for RSSI
#define ENA_STOARGE_TEMP_DETECTIONS_MAX (1000) // Maximum number of temporary stored detections

/**
 * read bytes at given address
 */
void ena_storage_read(size_t address, void *data, size_t size);

/**
 * store bytes at given address
 */
void ena_storage_write(size_t address, void *data, size_t size);

/**
 * deletes bytes at given address and shift other data back
 */
void ena_storage_shift_delete(size_t address, size_t end_address, size_t size);

/**
 * get last stored TEK
 * 
 * return cound
 */
uint8_t ena_storage_read_last_tek(ena_tek_t *tek);

/**
 * store TEK
 */
void ena_storage_write_tek(ena_tek_t *tek);

/**
 * get number of stored temporary detections
 */
uint32_t ena_storage_temp_detections_count(void);

/**
 * get temporary detection (RPI + AEM + RSSI with UNIX timestamp) at given index
 */
void ena_storage_read_temp_detection(uint32_t index, ena_temp_detection_t *detection);

/**
 * store temporary detection (RPI + AEM + RSSI with UNIX timestamp)
 * 
 * returns index
 */
uint32_t ena_storage_write_temp_detection(ena_temp_detection_t *detection);

/**
 * remove temporary detection at given index
 */
void ena_storage_remove_temp_detection(uint32_t index);

/**
 * get number of stored detections
 */
uint32_t ena_storage_detections_count(void);

/**
 * get detection (RPI + AEM + RSSI with ENIN) at given index
 */
void ena_storage_read_detection(uint32_t index, ena_detection_t *detection);

/**
 * store detection (RPI + AEM + RSSI with ENIN)
 */
void ena_storage_write_detection(ena_detection_t *detection);

void ena_storage_erase(void);

void ena_storage_dump_tek(void);

void ena_storage_dump_temp_detections(void);

void ena_storage_dump_detections(void);

#endif