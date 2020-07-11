*worked on this a few days offtime, now here is the first working source*

# esp-ena

Implementation of the Covid-19 Exposure Notification API by Apple and Google on an ESP32 (with ESP-IDF). 
More information about the Covid-19 Exposure Notification at [Apple](https://www.apple.com/covid19/contacttracing/) and [Google](https://www.google.com/covid19/exposurenotifications/). This is meant for people without smartphone or without smartphones with Apples/Googles implementation.

This implementation covers for now the BLE part including the cryptography specifications needed (see Bluetooth Specifications and Cryptography Specifications documents in the links above):
* send tokens
* store send tokens on flash (last 14 tokens)
* receive tokens
* received tokens are stored after 5 minutes threshold (storage is limited, ~100k tokens can be stored)

Features missing for now are:
* compare received tokens with infected list
* calculating risks scores

Extensions planned:
* add RTC (will test DS3231)
* add display (will test SSD1306)
* interface to
    * set time
    * delete tokens
    * show status
    * report infection?
* receive infected token list for Germany (will test [Corona Warn App](https://github.com/corona-warn-app))
* send infected status (will test [Corona Warn App](https://github.com/corona-warn-app))
* battery support
* 3d print case

Limitations/Problems
* storage only ~2.8mb available
* WiFi or other external connection needed for infections status (auto-connect to open WiFis?)
* obtaining accessibility
* all parameters (scanning time, thresholds etc.)

The following acronyms will be used in code and comments:
* *ENA* Exposure Notification Api
* *ENIN* ENIntervalNumber - timestamp with 10 minutes resolution
* *TEK* Temporary Exposure Key - personal secret key changed every 24h, published when infected
* *RPI* Rolling Proximity Identifier - send and received identifer changed every 10 minutes
* *AEM* Associated Encrypted Metadata - send and received metadata

Open questions
* now save ENIN for stored detection (documentation says timestamp), but for infection status ENIN should be enough!?
* service UUID is send reversed, must RPI and AEM also beeing send in reverse? Don't know BLE specification enough
* fixed change of advertise payload every 10 minutes, random value between ~15 minutes better?

## How to use

### Hardware Required

For now just an ESP32 is required. For full device later RTC (DS3231) and Display (SSD1306) will be required.

### Configure the project

```
idf.py menuconfig
```

required
* enable bluetooth (BLE)
* add partition-table for storage (currently hardcoded name "ena")
* mbedTLS enable HKDF

recommended
* BLE Scan Duplicate (By Device Address and Advertising Data)
 

### Build and Flash

May flash partition table:

```
idf.py partition_table-flash
```

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

## Example Output

For now some debug outputs are set. Besides, after each scan a CSV output is printed with stored TEKs, temporary detections (RPI) and full detections (RPI)

```
I (1201484) ESP-ENA-advertise: payload for ENIN 2657432
D (1201494) ESP-ENA-advertise: 0x3ffbb6c4   02 01 1a 03 03 6f fd 17  16 6f fd 9a ee 95 9a 24  |.....o...o.....$|
D (1201494) ESP-ENA-advertise: 0x3ffbb6d4   f0 f9 8e 56 0f 6d 68 5f  ac 12 e5 7f 94 a1 47     |...V.mh_......G|
I (1201524) ESP-ENA-scan: start scanning...
D (1202224) ESP-ENA-detection: New temporary detection at 0 with timestamp 1594459201
D (1202224) ESP-ENA-detection: 19 05 e3 3a 73 16 4e 74 2d 48 fc 0c 41 f6 26 3b 
D (1202234) ESP-ENA-detection: 5e 7d a9 48 
D (1202234) ESP-ENA-detection: RSSI -79
#,enin,tek
0,2657430,d5 13 92 b2 44 e4 7e b6 ca a7 20 c4 f 37 c0 1c
#,timestamp,rpi,aem,rssi
0,1594459201,19 5 e3 3a 73 16 4e 74 2d 48 fc c 41 f6 26 3b,5e 7d a9 48,-79
#,enin,rpi,aem,rssi
0,2657430,c7 2e b6 66 af 84 42 db b d1 a 0 f1 fd 86 2,4d 1 b2 d1,-76
1,2657431,19 5 e3 3a 73 16 4e 74 2d 48 fc c 41 f6 26 3b,5e 7d a9 48,-76
I (1231754) ESP-ENA-scan: finished scanning...
```

## Troubleshooting

Sometimes I get errors from BT-stack of ESP-IDF printed. Didn't affect functionality for now, but I also could not find out what it caused and what it means.

```
E (909164) BT_HCI: btu_hcif_hdl_command_complete opcode 0x2005 status 0xc
```

## Structure

The project is divided in different files
* *ena-crypto* covers cryptography part (key creation, encryption etc.)
* *ena-storage* storage part to store own TEKs and detections
* *ena-detection* handles scanned data by storing temporary detections, check for threshold and store full detections
* *ena-bluetooth-scan* BLE scans for detecting other tokens
* *ena-bluetooth-advertise* BLE advertising to send own tokens
* *ena* run all together and timing for scanning and advertising
* *main* start and run main program
