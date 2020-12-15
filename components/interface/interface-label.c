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
 * @brief texts for intefaces
 *  
 */
#include <string.h>
#include "interface.h"

static interface_locale_t current_locale = EN;

void interface_init_label(void)
{
    // EN
    interface_text_button_cancel.text[EN] = "CANCEL";
    interface_text_button_ok.text[EN] = "OK";
    interface_text_button_back.text[EN] = "BACK";
    interface_text_button_menu.text[EN] = "MENU";
    interface_text_button_report.text[EN] = "REPORT";

    interface_text_headline_tan.text[EN] = "ENTER TAN";
    interface_text_headline_report.text[EN] = "REPORT";
    interface_text_headline_wifi.text[EN] = "WIFI";
    interface_text_headline_time.text[EN] = "TIME/DATE";
    interface_text_headline_data.text[EN] = "DEL DATA";
    interface_text_headline_settings.text[EN] = "SETTING";
    interface_text_headline_info.text[EN] = "INFO";
    interface_text_headline_debug.text[EN] = "DEBUG";

    interface_text_wifi_waiting.text[EN] = "Waiting...";
    interface_text_wifi_scanning.text[EN] = "Scanning...";
    interface_text_wifi_connecting.text[EN] = "Connecting...";
    interface_text_wifi_nothing.text[EN] = "None...";

    interface_text_settings_locale.text[EN] = "Language:";
    interface_text_settings_locales[EN].text[EN] = "EN";
    interface_text_settings_locales[DE].text[EN] = "DE";

    interface_text_settings_timezone.text[EN] = "UTC:";

    interface_text_info_num_keys.text[EN] = "Seen:";
    interface_text_info_last_keys.text[EN] = "<=30m:";
    interface_text_info_exp_days.text[EN] = "Last Exp:";
    interface_text_info_exp_num.text[EN] = "Num Exp:";
    interface_text_info_exp_max.text[EN] = "Score:";
    interface_text_info_exp_sum.text[EN] = "Scores:";

    interface_text_report_pending.text[EN] = "Uploading...";
    interface_text_report_success.text[EN] = "Upload succeed!";
    interface_text_report_fail.text[EN] = "Upload failed!";

    interface_text_data_del[0].text[EN] = "DEL TEK";
    interface_text_data_del[1].text[EN] = "DEL Exp Info";
    interface_text_data_del[2].text[EN] = "DEL Tmp RPI";
    interface_text_data_del[3].text[EN] = "DEL RPI";
    interface_text_data_del[4].text[EN] = "DEL Lst Upd.";
    interface_text_data_del[5].text[EN] = "DEL All Data";

    interface_texts_weekday[0].text[EN] = "Sun";
    interface_texts_weekday[1].text[EN] = "Mon";
    interface_texts_weekday[2].text[EN] = "Tue";
    interface_texts_weekday[3].text[EN] = "Wed";
    interface_texts_weekday[4].text[EN] = "Thu";
    interface_texts_weekday[5].text[EN] = "Fri";
    interface_texts_weekday[6].text[EN] = "Sat";

    interface_texts_month[0].text[EN] = "Jan";
    interface_texts_month[1].text[EN] = "Feb";
    interface_texts_month[2].text[EN] = "Mar";
    interface_texts_month[3].text[EN] = "Apr";
    interface_texts_month[4].text[EN] = "May";
    interface_texts_month[5].text[EN] = "Jun";
    interface_texts_month[6].text[EN] = "Jul";
    interface_texts_month[7].text[EN] = "Aug";
    interface_texts_month[8].text[EN] = "Sep";
    interface_texts_month[9].text[EN] = "Oct";
    interface_texts_month[10].text[EN] = "Nov";
    interface_texts_month[11].text[EN] = "Dec";

    // DE
    interface_text_button_cancel.text[DE] = "ZURÜCK";
    interface_text_button_back.text[DE] = "ZURÜCK";
    interface_text_button_ok.text[DE] = "OK";
    interface_text_button_menu.text[DE] = "MENU";
    interface_text_button_report.text[DE] = "MELDEN";

    interface_text_headline_tan.text[DE] = "TAN EING.";
    interface_text_headline_report.text[DE] = "MELDEN";
    interface_text_headline_wifi.text[DE] = "WLAN";
    interface_text_headline_time.text[DE] = "ZEIT/DATUM";
    interface_text_headline_data.text[DE] = "DATEN ENTF";
    interface_text_headline_settings.text[DE] = "EINSTEL.";
    interface_text_headline_info.text[DE] = "INFOS";
    interface_text_headline_debug.text[DE] = "DEBUG";

    interface_text_wifi_waiting.text[DE] = "Warten...";
    interface_text_wifi_scanning.text[DE] = "Scannen...";
    interface_text_wifi_connecting.text[DE] = "Verbinden...";
    interface_text_wifi_nothing.text[DE] = "Keine...";

    interface_text_settings_locale.text[DE] = "Sprache:";
    interface_text_settings_locales[EN].text[DE] = "EN";
    interface_text_settings_locales[DE].text[DE] = "DE";

    interface_text_settings_timezone.text[DE] = "GMT:";

    interface_text_info_num_keys.text[DE] = "Gesehen:";
    interface_text_info_last_keys.text[DE] = "<=30m:";
    interface_text_info_exp_days.text[DE] = "letz. Exp:";
    interface_text_info_exp_num.text[DE] = "Anz. Exp:";
    interface_text_info_exp_max.text[DE] = "Score:";
    interface_text_info_exp_sum.text[DE] = "Scores:";

    interface_text_report_pending.text[DE] = "Hochladen...";
    interface_text_report_success.text[DE] = "Erfolgreich!";
    interface_text_report_fail.text[DE] = "Fehlgeschlagen!";

    interface_text_data_del[0]
        .text[DE] = "ENTF TEK";
    interface_text_data_del[1].text[DE] = "ENTF Exp Info";
    interface_text_data_del[2].text[DE] = "ENTF Tmp RPI";
    interface_text_data_del[3].text[DE] = "ENTF RPI";
    interface_text_data_del[4].text[DE] = "ENTF letz. Up";
    interface_text_data_del[5].text[DE] = "ENTF Daten";

    interface_texts_weekday[0].text[DE] = "So.";
    interface_texts_weekday[1].text[DE] = "Mo.";
    interface_texts_weekday[2].text[DE] = "Di.";
    interface_texts_weekday[3].text[DE] = "Mi.";
    interface_texts_weekday[4].text[DE] = "Do.";
    interface_texts_weekday[5].text[DE] = "Fr.";
    interface_texts_weekday[6].text[DE] = "Sa.";

    interface_texts_month[0].text[DE] = "Jan";
    interface_texts_month[1].text[DE] = "Feb";
    interface_texts_month[2].text[DE] = "Mär";
    interface_texts_month[3].text[DE] = "Apr";
    interface_texts_month[4].text[DE] = "Mai";
    interface_texts_month[5].text[DE] = "Jun";
    interface_texts_month[6].text[DE] = "Jul";
    interface_texts_month[7].text[DE] = "Aug";
    interface_texts_month[8].text[DE] = "Sep";
    interface_texts_month[9].text[DE] = "Okt";
    interface_texts_month[10].text[DE] = "Nov";
    interface_texts_month[11].text[DE] = "Dez";
}

interface_locale_t interface_get_locale(void)
{
    return current_locale;
}

void interface_set_locale(interface_locale_t locale)
{
    current_locale = locale;
}

char *interface_get_label_text(interface_label_t *label)
{
    return label->text[current_locale];
}