# esp-ena

Implementation of contact tracing with the Covid-19 Exposure Notification API by Apple and Google on an ESP32 (with [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html)). 
More information about the Covid-19 Exposure Notification at [Apple](https://www.apple.com/covid19/contacttracing/) and [Google](https://www.google.com/covid19/exposurenotifications/). This is meant for people without smartphone or without smartphones with Apples/Googles implementation.

[Demo Video](https://twitter.com/Lurkars/status/1282223547579019264)

This implementation fully covers for the BLE part including the cryptography specifications needed (see Bluetooth Specifications and Cryptography Specifications documents in the links above):
* send beacons
* store TEKs on flash (last 14)
* receive beacons
* received beacons are stored after 5 minutes threshold (storage is limited, ~100k beacons can be stored)

Additional features for full ENA device
* calculating risks scores (after adding reported keys and storing exposure information)
* RTC support with DS3231
* display support with SSD1306
* interface to
    * set time

Features missing for now are:
* retrieve infected list and parse from binary (started with binary parsing)

Extensions planned:
* interface to
    * delete data
    * show status
    * report infection?
* receive infected beacons list (will test [Corona Warn App](https://github.com/corona-warn-app))
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
* service UUID is send reversed, must RPI and AEM also beeing send in reverse? Don't know BLE specification enough

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
* BLE *Scan Duplicate* (By Device Address and Advertising Data)

debug options
* Log output set to Debug
* Exposure Notification API / Storage enable *Dump storage* 
 

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

## Troubleshooting

Sometimes I get errors from BT-stack of ESP-IDF printed. Didn't affect functionality for now, but I also could not find out what it caused and what it means.

```
E (909164) BT_HCI: btu_hcif_hdl_command_complete opcode 0x2005 status 0xc
```

## Structure

The project is divided in different components. The main.c just wrap up all components.

### ena

The ena module contains the main functions of eps-ena with bluetooth scanning and adverting, storing data and handle beacons.
* *ena-beacons* handles scanned data by storing temporary beacons, check for threshold and store beacons permanently
* *ena-crypto* covers cryptography part (key creation, encryption etc.)
* *ena-storage* storage part to store own TEKs and beacons
* *ena-bluetooth-scan* BLE scans for detecting other beacons
* *ena-bluetooth-advertise* BLE advertising to send own beacons
* *ena* run all together and timing for scanning and advertising

### ena-interface

Adds interface functionality via touch pads for control and setup.

### i2c-main

Just start I2C driver for display and RTC.

### ssd1306

I2C driver for a SSD1306 display.