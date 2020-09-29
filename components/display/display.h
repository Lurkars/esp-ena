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
 * @brief interface for display
 *  
 */
#ifndef _display_H_
#define _display_H_

#include "esp_system.h"

/**
 * 
 */
void display_utf8_to_ascii(char *input, char *output);

/**
 * 
 */
uint8_t *display_text_to_data(char *text, size_t text_length, size_t *length);

/**
 * @brief initalize display
 */
void display_start(void);

/**
 * @brief clear the display
 * 
 * @param[in] line the line to clear
 * @param[in] invert if true, image is inverted
 */
void display_clear_line( uint8_t line, bool invert);

/**
 * @brief clear the display
 * 
 */
void display_clear(void);

/**
 * @brief set display on or offf
 * 
 * @param[in] on true if display on, false if display off
 */
void display_on(bool on);

/**
 * 
 */
uint8_t *display_text_to_data(char *text, size_t text_length, size_t *length);

/**
 * @brief write raw bytes to display line at starting column
 * 
 * @param[in] data bytes to display  
 * @param[in] length length of data
 * @param[in] line the line to write to
 * @param[in] offset number of offset chars to start
 * @param[in] invert if true, image is inverted
 */
void display_data(uint8_t *data, size_t length, uint8_t line, uint8_t offset, bool invert);

/**
 * @brief write chars to display
 * 
 * @param[in] text text to display  
 * @param[in] length length of text
 * @param[in] line the line to write to
 * @param[in] offset number of offset chars to start
 * @param[in] invert if true, image is inverted
 */
void display_chars(char *text, size_t length, uint8_t line, uint8_t offset, bool invert);

/**
 * @brief write text to display line at starting column
 * 
 * @param[in] text text to display  
 * @param[in] line the line to write to
 * @param[in] offset number of offset chars to start
 * @param[in] invert if true, image is inverted
 */
void display_text_line_column(char *text, uint8_t line, uint8_t offset, bool invert);

/**
 * @brief write text to display line
 * 
 * @param[in] text text to display  
 * @param[in] line the line to write to
 * @param[in] invert if true, image is inverted
 */
void display_text_line(char *text, uint8_t line, bool invert);

/**
 * @brief display a button element
 * 
 * @param[in] text button text
 * @param[in] selected is button selected
 * @param[in] primary is button primary
 */
void display_set_button(char *text, bool selected, bool primary);

/**
 * @brief display a menu headline
 * 
 * @param[in] text headline text
 * @param[in] arrows if left right arrows should be displays
 * @param[in] line line the line to write to
 */
void display_menu_headline(char *text, bool arrows, uint8_t line);

#endif