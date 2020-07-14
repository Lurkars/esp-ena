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
#include "mbedtls/md.h"
#include "mbedtls/aes.h"
#include "mbedtls/hkdf.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include "esp_log.h"

#include "ena-crypto.h"

#define ESP_CRYPTO_LOG "ESP-CRYPTO"

static mbedtls_ctr_drbg_context ctr_drbg;

void ena_crypto_init(void)
{
    mbedtls_entropy_context entropy;
    uint8_t pers[] = "Exposure Notifcation API esp32";
    int ret;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, pers, sizeof(pers))) != 0)
    {
        ESP_LOGE(ESP_CRYPTO_LOG, " failed\n ! mbedtls_ctr_drbg_init returned -0x%04x\n", -ret);
    }
}

uint32_t ena_crypto_enin(uint32_t unix_epoch_time)
{
    return unix_epoch_time / ENA_TIME_WINDOW;
}

void ena_crypto_tek(uint8_t *tek)
{
    int ret;
    if ((ret = mbedtls_ctr_drbg_random(&ctr_drbg, tek, ENA_KEY_LENGTH)) != 0)
    {
        ESP_LOGE(ESP_CRYPTO_LOG, " failed\n ! mbedtls_ctr_drbg_random returned -0x%04x\n", -ret);
    }
}

void ena_crypto_rpik(uint8_t *rpik, uint8_t *tek)
{
    const uint8_t rpik_info[] = "EN-RPIK";
    mbedtls_hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), NULL, 0, tek, ENA_KEY_LENGTH, rpik_info, sizeof(rpik_info), rpik, ENA_KEY_LENGTH);
}

void ena_crypto_rpi(uint8_t *rpi, uint8_t *rpik, uint32_t enin)
{
    uint8_t padded_data[] = "EN-RPI";
    padded_data[12] = (enin & 0x000000ff);
    padded_data[13] = (enin & 0x0000ff00) >> 8;
    padded_data[14] = (enin & 0x00ff0000) >> 16;
    padded_data[15] = (enin & 0xff000000) >> 24;

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, rpik, ENA_KEY_LENGTH * 8);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, padded_data, rpi);
    mbedtls_aes_free(&aes);
}

void ena_crypto_aemk(uint8_t *aemk, uint8_t *tek)
{
    uint8_t aemkInfo[] = "EN-AEMK";
    mbedtls_hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), NULL, 0, tek, ENA_KEY_LENGTH, aemkInfo, sizeof(aemkInfo), aemk, ENA_KEY_LENGTH);
}

void ena_crypto_aem(uint8_t *aem, uint8_t *aemk, uint8_t *rpi, uint8_t power_level)
{
    uint8_t metadata[ENA_AEM_METADATA_LENGTH];
    metadata[0] = 0b01000000;
    metadata[1] = power_level;
    size_t count = 0;
    uint8_t sb[16] = {0};
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, aemk, ENA_KEY_LENGTH * 8);
    mbedtls_aes_crypt_ctr(&aes, ENA_AEM_METADATA_LENGTH, &count, rpi, sb, metadata, aem);
    mbedtls_aes_free(&aes);
}