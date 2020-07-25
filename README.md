# esp-ena

Implementation of contact tracing with the Covid-19 Exposure Notification API by Apple and Google on an ESP32 (with [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html)). 
More information about the Covid-19 Exposure Notification at [Apple](https://www.apple.com/covid19/contacttracing/) and [Google](https://www.google.com/covid19/exposurenotifications/). This is fully compatible with the official API and is meant for people without smartphone or without access to Apples/Googles implementation.

The main source (the Exposure Notification API) is a separate module in [**components/ena**](components/ena).

This implementation fully covers the BLE part including the cryptography specifications needed and the exposure check.

### Features implemented
* send/receive BLE beacons as defined in [Bluetooth® Specification (Apple/Google)](https://blog.google/documents/70/Exposure_Notification_-_Bluetooth_Specification_v1.2.2.pdf) and [Cryptography Specification (Apple/Google)](https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf)
* BLE privacy (change random MAC address in random interval)
* permanent storage on flash of last keys, beacons and exposures (storage is limited, ~100k beacons can be stored)
* parsing of Exposure Key export binaries as defined in [Exposure Key export file format and verification](https://developers.google.com/android/exposure-notifications/exposure-key-file-format) (big thanks to [nanopb](https://github.com/nanopb/nanopb) for making this easier than I thought!)
* calculating exposure risks/scores (after adding reported keys and storing exposure information) as defined in [ENExposureConfiguration (Apple)](https://developer.apple.com/documentation/exposurenotification/enexposureconfiguration)

Additional features for full ENA device
* RTC support with DS3231 (for correct system time)
* display support with SSD1306
* interface to
    * set time
    * show exposure status

### Features in development
* automatically receive Exposure Key export from web (started with [Corona Warn App](https://github.com/corona-warn-app))
* send infected status (will test [Corona Warn App](https://github.com/corona-warn-app))
* battery support
* 3d print case
* interface to
    * delete data
    * report infection

### Limitations/Problems
* storage only ~2.5mb available
* WiFi or other external connection needed for infections status (auto-connect to open WiFis?)
* obtaining accessibility
* all parameters (scanning time, thresholds etc.)

### Questions/Problems/Annotations
* memory is really low with BLE and WiFi enabled, unzipping a Key Export not possible for now, maybe disable BLE service for download.
* service UUID is send reversed, RPI and AEM also send in reverse? Don't know BLE specification enough

The following acronyms will be used in code and comments:
* *ENA* Exposure Notification Api
* *ENIN* ENIntervalNumber - timestamp with 10 minutes resolution
* *TEK* Temporary Exposure Key - personal secret key changed every 24h, published when infected
* *RPI* Rolling Proximity Identifier - send and received identifer changed every 10 minutes
* *AEM* Associated Encrypted Metadata - send and received metadata

## How to use

### Hardware Required

For now just an ESP32 is required. DS3231 RTC and SSD1306 Display are required for a complete device.

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

The project is divided in different components. The main.c just wrap up all components. The Exposure Notification API is in **ena** module

### ena

The ena module contains the main functions of eps-ena with bluetooth scanning and adverting, storing data, handle beacons and check exposure.
* *ena-beacons* handles scanned data by storing temporary beacons, check for threshold and store beacons permanently
* *ena-crypto* covers cryptography part (key creation, encryption etc.)
* *ena-storage* storage part to store own TEKs and beacons
* *ena-bluetooth-scan* BLE scans for detecting other beacons
* *ena-bluetooth-advertise* BLE advertising to send own beacons
* *ena-exposure* decode Key Export, compare with stored beacons, calculate score and risk
* *ena* run all together and timing for scanning and advertising

### ena-cwa

Connection to german Exposure App ([Corona Warn App](https://github.com/corona-warn-app)) for download Key Export (and maybe later report infection).

### ena-interface

Adds interface functionality via touch pads for control and setup.

### i2c-main

Just start I2C driver for display and RTC.

### ds3231

I2C driver for a DS3231 RTC

### ssd1306

I2C driver for a SSD1306 display.

### nanopb

[Nanopb](https://github.com/nanopb/nanopb) for reading Protocol Buffers of Key Export. Including already generated Headers from *.proto files.

### miniz

[Miniz](https://github.com/richgel999/miniz) for unzipping Key Export (not successful for now due to memory limit)


## Demo

[Demo Video (early stage)](https://twitter.com/Lurkars/status/1282223547579019264)