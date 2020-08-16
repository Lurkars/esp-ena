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
/**
 * @file
 * 
 * @brief interface functionality via input buttons for control and setup
 *  
 */
#ifndef _interface_H_
#define _interface_H_

#include "driver/gpio.h"

#define INTERFACE_LOG "INTERFACE" // TAG for Logging

#define INTERFACE_BUTTON_RST GPIO_NUM_32
#define INTERFACE_BUTTON_SET GPIO_NUM_33
#define INTERFACE_BUTTON_MID GPIO_NUM_25
#define INTERFACE_BUTTON_RHT GPIO_NUM_26
#define INTERFACE_BUTTON_LFT GPIO_NUM_27
#define INTERFACE_BUTTON_DWN GPIO_NUM_14
#define INTERFACE_BUTTON_UP GPIO_NUM_12

#define INTERFACE_FORMAT_TIME "%X"

#define INTERFACE_NUM_LOCALE 2

/**
 * @brief available locales
 */
typedef enum
{
    EN = 0,
    DE,
} interface_locale_t;

typedef struct
{

    char *text[INTERFACE_NUM_LOCALE];

} interface_label_t;

// label variables
interface_label_t interface_text_button_cancel;
interface_label_t interface_text_button_ok;
interface_label_t interface_text_button_menu;
interface_label_t interface_text_button_report;

interface_label_t interface_text_headline_tan;
interface_label_t interface_text_headline_wifi;
interface_label_t interface_text_headline_time;
interface_label_t interface_text_headline_data;
interface_label_t interface_text_headline_settings;
interface_label_t interface_text_headline_debug;

interface_label_t interface_text_settings_locale;
interface_label_t interface_text_settings_locales[INTERFACE_NUM_LOCALE];
interface_label_t interface_text_settings_timezone;

interface_label_t interface_text_wifi_scanning;

interface_label_t interface_text_data_del[5];

interface_label_t interface_texts_weekday[7];

interface_label_t interface_texts_month[12];

/**
 * @brief       callback function for button press
 */
typedef void (*interface_button_callback)(void);

/**
 * @brief       current display function
 */
typedef void (*interface_display_function)(void);

/**
 * @brief       callback function for text_input
 * 
 * @param[in]   text    the text from input
 * @param[in]   cursor  current cursor position 
 */
typedef void (*interface_input_callback)(char *text, uint8_t cursor);

/**
 * @brief       init label
 */
void interface_init_label(void);

/**
 * @brief       get text from label for set locale
 * 
 * @param[in]   label the label to get the text from
 */
char *interface_get_label_text(interface_label_t *label);

/**
 * @brief       set locale for interface
 * 
 * @return
 *              interface_locale_t  current locale
 */
interface_locale_t interface_get_locale(void);

/**
 * @brief       set locale for interface
 * 
 * @param[in]   locale the locale to set
 */
void interface_set_locale(interface_locale_t locale);

/**
 * @brief       register a callback function for button event
 * 
 * @param[in]   button_gpio id of the button to listen to
 * @param[in]   callback    callback function
 */
void interface_register_button_callback(int button_gpio, interface_button_callback callback);

/**
 * @brief       set the display function
 * 
 * @param[in]   display_function    display  function
 */
void interface_set_display_function(interface_display_function display_function);

/**
 * @brief       start interface logic
 * 
 * This will initialize the controls and start a task to listen to button inputs and calling the callbacks.
 */
void interface_start(void);

/**
 * @brief       start delete data interface
 */
void interface_data_start(void);

/**
 * @brief       start datetime interface
 */
void interface_datetime_start(void);

/**
 * @brief       start main interface
 */
void interface_main_start(void);

/**
 * @brief       start report interface
 */
void interface_report_start(void);

/**
 * @brief       start wifi interface
 */
void interface_wifi_start(void);

/**
 * @brief       start settings interface
 */
void interface_settings_start(void);

/**
 * @brief       start interface for input
 * 
 * @param[in]   callback_rst    function to call on RST
 * @param[in]   callback_set    function to call on SET
 * @param[in]   limit           max character allowed (0=255)   
 */
void interface_input(interface_input_callback callback_rst, interface_input_callback callback_set, uint8_t limit);

/**
 * @brief       set text for input interface
 * 
 * @param[in]   text    the text to set
 */
void interface_input_set_text(char *text);

#endif