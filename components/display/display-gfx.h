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
 * @brief gfx elements: font, icons, elements
 *  
 */
#ifndef _display_GFX_H_
#define _display_GFX_H_

#include <stdint.h>

uint8_t display_gfx_clear[8];

// Dogica Font https://www.dafont.com/de/dogica.font starting at space (32)
uint8_t display_gfx_font[224][8];

uint8_t display_gfx_button[3][64];

uint8_t display_gfx_button_sel[3][64];

uint8_t display_gfx_clock[8];

uint8_t display_gfx_menu_head[112];

uint8_t display_gfx_sad[4][24];

uint8_t display_gfx_smile[4][24];

uint8_t display_gfx_question[4][24];

uint8_t display_gfx_wifi[8];

uint8_t display_gfx_wifi_low[8];

uint8_t display_gfx_wifi_lowest[8];

uint8_t display_gfx_arrow_down[8];

uint8_t display_gfx_arrow_left[8];

uint8_t display_gfx_arrow_right[8];

uint8_t display_gfx_arrow_up[8];

uint8_t display_gfx_cross[8];

uint8_t display_gfx_logo[8][64];

#endif