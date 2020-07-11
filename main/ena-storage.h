#ifndef _ena_STORAGE_H_
#define _ena_STORAGE_H_

#include "ena-crypto.h"

#define ENA_STORAGE_LOG "ESP-ENA-storage" // TAG for Logging
#define PARTITION_NAME "ena"
#define ENA_STOARGE_TEK_STORE_PERIOD (14)                                                                                                         // Period of storing TEKs
#define ENA_STORAGE_TEK_COUNT_ADDRESS (0)                                                                                                         // starting address for TEK COUNT
#define ENA_STORAGE_TEK_START_ADDRESS (ENA_STORAGE_TEK_COUNT_ADDRESS + 1)                                                                         // starting address for TEKs
#define ENA_STORAGE_TEK_LENGTH (ENA_KEY_LENGTH + 4)                                                                                               // length of a stored TEK -> TEK keysize + 4 Bytes for ENIN
#define ENA_STORAGE_DETECTION_LENGTH (ENA_KEY_LENGTH + ENA_AEM_METADATA_LENGTH + 4 + sizeof(int))                                                             // length of a stored detection -> RPI keysize + AEM size + 4 Bytes for ENIN + 4 Bytes for RSSI
#define ENA_STOARGE_TEMP_DETECTIONS_MAX (1000)                                                                                                      // Maximum number of temporary stored detections
#define ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS (ENA_STORAGE_TEK_START_ADDRESS + ENA_STORAGE_TEK_LENGTH * ENA_STOARGE_TEK_STORE_PERIOD)           // starting address for temporary detections COUNT (offset from max. stored TEKs)
#define ENA_STORAGE_TEMP_DETECTIONS_START_ADDRESS (ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS + 4)                                                     // starting address for temporary detections
#define ENA_STORAGE_DETECTIONS_COUNT_ADDRESS (ENA_STORAGE_TEMP_DETECTIONS_COUNT_ADDRESS + ENA_STORAGE_DETECTION_LENGTH * ENA_STOARGE_TEMP_DETECTIONS_MAX) // starting address for detections COUNT (offset from max. stored temporary detections)
#define ENA_STORAGE_DETECTIONS_START_ADDRESS (ENA_STORAGE_DETECTIONS_COUNT_ADDRESS + 4)                                                               // starting address of detections


/**
 * read bytes at given address
 */
void ena_storage_read(size_t address, uint8_t *data, size_t size);

/**
 * store bytes at given address
 */
void ena_storage_write(size_t address, uint8_t data[], size_t size);

/**
 * deletes bytes at given address and shift other data back
 */
void ena_storage_shift_delete(size_t address, size_t size);

/**
 * store TEK with ENIN
 */
void ena_storage_write_tek(uint32_t enin, uint8_t tek[]);

/**
 * get last stored ENIN
 */
uint32_t ena_storage_read_enin(void);

/**
 * get last stored TEK
 */
void ena_storage_read_tek(uint8_t tek[]);

/**
 * get number of stored temporary detections
 */
uint32_t ena_storage_temp_detections_count(void);

/**
 * store temporary detection (RPI + AEM + RSSI with UNIX timestamp)
 * 
 * returns index
 */
uint32_t ena_storage_write_temp_detection(uint32_t timestamp, uint8_t rpi[], uint8_t aem[], int rssi);

/**
 * get temporary detection (RPI + AEM + RSSI with UNIX timestamp) at given index
 */
void ena_storage_read_temp_detection(uint32_t index, uint32_t *timestamp, uint8_t rpi[], uint8_t aem[], int *rssi);

/**
 * remove temporary detection at given index
 */
void ena_storage_remove_temp_detection(uint32_t index);

/**
 * get number of stored detections
 */
uint32_t ena_storage_detections_count(void);

/**
 * store detection (RPI + AEM + RSSI with ENIN)
 */
void ena_storage_write_detection(uint32_t timestamp, uint8_t rpi[], uint8_t aem[], int rssi);

/**
 * get detection (RPI + AEM + RSSI with ENIN) at given index
 */
void ena_storage_read_detection(uint32_t index, uint32_t *enin, uint8_t rpi[], uint8_t aem[], int *rssi);

uint8_t ena_storage_read_u8(size_t address);

uint32_t ena_storage_read_u32(size_t address);

void ena_storage_write_u8(size_t address, uint8_t byte);

void ena_storage_write_u32(size_t address, uint32_t value);

int ena_storage_read_int(size_t address);

void ena_storage_write_int(size_t address, int value);

void ena_storage_erase(void);

void ena_storage_dump_tek(void);

void ena_storage_dump_temp_detections(void); 

void ena_storage_dump_detections(void);

#endif