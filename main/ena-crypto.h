
/**
 * provide cryptographic part of Exposure Notification API v1.2 as defined by Apple/Google
 * 
 * Source documents:
 * 
 * https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-CryptographySpecificationv1.2.pdf
 * 
 * 
 * 
 */

#ifndef _ena_CRYPTO_H_
#define _ena_CRYPTO_H_

#define ENA_TIME_WINDOW 600        // time window every 10 minutes
#define ENA_KEY_LENGTH 16          // key length
#define ENA_AEM_METADATA_LENGTH 4  // size of metadata
#define ENA_TEK_ROLLING_PERIOD 144 // TEKRollingPeriod

#include <stdio.h>

/**
 * initialize crypto (setup entropy for key generation)
 */
void ena_crypto_init(void);

/**
 * Calculate ENIntervalNumber (ENIN) for given UNIX timestamp
 */
uint32_t ena_crypto_enin(uint32_t seconds);

/**
 * calculate a new random Temporary Exposure Key (TEK)
 */
void ena_crypto_tek(uint8_t *tek);

/**
 * calculate a new Rolling Proximity Identifier Key (RPIK) with given TEK
 */
void ena_crypto_rpik(uint8_t *rpik, uint8_t *tek);

/**
 * calculate a new Rolling Proximity Identifier with given RPIK and ENIN
 */
void ena_crypto_rpi(uint8_t *rpi, uint8_t *rpik, uint32_t enin);

/**
 * calculate a new Associated Encrypted Metadata Key (AEMK) with given TEK
 */
void ena_crypto_aemk(uint8_t *aemk, uint8_t *tek);

/**
 * create Associated Encrypted Metadata (AEM) with given AEMK along the RPI
 */
void ena_crypto_aem(uint8_t *aem, uint8_t *aemk, uint8_t *rpi, uint8_t power_level);

#endif