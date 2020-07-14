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

#ifndef _ena_CRYPTO_H_
#define _ena_CRYPTO_H_

#define ENA_TIME_WINDOW (600)        // time window every 10 minutes
#define ENA_KEY_LENGTH (16)          // key length
#define ENA_AEM_METADATA_LENGTH (4)  // size of metadata
#define ENA_TEK_ROLLING_PERIOD (144) // TEKRollingPeriod

#include <stdio.h>

/**
 * @brief initialize cryptography 
 * 
 * This initialize the cryptography by setting up entropy.
 */
void ena_crypto_init(void);

/**
 * @brief calculate ENIntervalNumber (ENIN) for given UNIX timestamp
 * 
 * Source documents (Section: ENIntervalNumber)
 * 
 * https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-CryptographySpecificationv1.2.pdf
 * 
 * 
 * @param[in] unix_timestamp UNIX Timestamp to calculate ENIN for 
 * 
 * @return
 *      ENIN for given timestamp
 */
uint32_t ena_crypto_enin(uint32_t unix_timestamp);

/**
 * @brief calculate a new random Temporary Exposure Key (TEK)
 * 
 * Source documents (Section: Temporary Exposure Key)
 * 
 * https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-CryptographySpecificationv1.2.pdf
 * 
 * @param[out] tek pointer to the new TEK
 */
void ena_crypto_tek(uint8_t *tek);

/**
 * @brief calculate a new Rolling Proximity Identifier Key (RPIK) with given TEK
 * 
 * Source documents (Section: Rolling Proximity Identifier Key)
 * 
 * https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-CryptographySpecificationv1.2.pdf
 * 
 * @param[out] rpik pointer to the new RPIK
 * @param[in] tek TEK for calculating RPIK
 */
void ena_crypto_rpik(uint8_t *rpik, uint8_t *tek);

/**
 * @brief calculate a new Rolling Proximity Identifier with given RPIK and ENIN
 * 
 * Source documents (Section: Rolling Proximity Identifier)
 * 
 * https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-CryptographySpecificationv1.2.pdf
 * 
 * @param[out] rpi pointer to the new RPI
 * @param[in] rpik RPIK for encrypting RPI
 * @param[in] enin ENIN to encrypt in RPI
 */
void ena_crypto_rpi(uint8_t *rpi, uint8_t *rpik, uint32_t enin);

/**
 * @brief       calculate a new Associated Encrypted Metadata Key (AEMK) with given TEK
 * 
 * Source documents (Section: Associated Encrypted Metadata Key)
 * 
 * https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-CryptographySpecificationv1.2.pdf
 * 
 * @param[out]  aemk    pointer to the new AEMK
 * @param[in]   tek     TEK for calculating AEMK
 */
void ena_crypto_aemk(uint8_t *aemk, uint8_t *tek);

/**
 * @brief       create Associated Encrypted Metadata (AEM) with given AEMK along the RPI
 * 
 * Source documents (Section: Associated Encrypted Metadata)
 * 
 * https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-CryptographySpecificationv1.2.pdf
 * 
 * @param[out]  aem             pointer to the new AEM
 * @param[in]   aemk            AEMK for encrypting AEM
 * @param[in]   rpi             RPI for encrypting AEM
 * @param[in]   power_level     BLE power level to encrypt in AEM
 */
void ena_crypto_aem(uint8_t *aem, uint8_t *aemk, uint8_t *rpi, uint8_t power_level);

#endif