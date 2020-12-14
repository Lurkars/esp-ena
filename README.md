# esp-ena

Implementation of contact tracing with the Covid-19 Exposure Notification API by Apple and Google on an ESP32 (with [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html)). 
More information about the Covid-19 Exposure Notification at [Apple](https://www.apple.com/covid19/contacttracing/) and [Google](https://www.google.com/covid19/exposurenotifications/). This is fully compatible with the official API and is meant for people without smartphone or without access to Apple/Google's implementation.

This is the main source (the Exposure Notification API). Full device is in the [**main branch**](https://github.com/Lurkars/esp-ena/).

This implementation fully covers the BLE part including the cryptography specifications needed and the exposure check.

The following acronyms will be used in code and comments:
* *ENA* Exposure Notification API
* *ENIN* ENIntervalNumber - timestamp with 10 minutes resolution
* *TEK* Temporary Exposure Key - personal secret key changed every 24h, published when infected
* *RPI* Rolling Proximity Identifier - send and received identifier changed every 10 minutes
* *AEM* Associated Encrypted Metadata - send and received metadata

### Features implemented
* send/receive BLE beacons as defined in [Bluetooth® Specification (Apple/Google)](https://blog.google/documents/70/Exposure_Notification_-_Bluetooth_Specification_v1.2.2.pdf) and [Cryptography Specification (Apple/Google)](https://blog.google/documents/69/Exposure_Notification_-_Cryptography_Specification_v1.2.1.pdf)
* BLE privacy (change random MAC address in random interval)
* permanent storage on flash of last keys, beacons and exposures (storage is limited, see [storage math](#some-storage-math) for details)
* calculating exposure risks/scores (after adding reported keys and storing exposure information) as defined in [ENExposureConfiguration (Apple v1)](https://developer.apple.com/documentation/exposurenotification/enexposureconfiguration/calculating_the_exposure_risk_value_in_exposurenotification_version_1)
* receive Exposure Key export from an ENA Exposure Key export proxy server [ena-eke-proxy module](#ena-eke-proxy), see [ena-eke-proxy server reference implemenation](https://github.com/Lurkars/ena-eke-proxy)
* Upload of own Exposure keys to proxy server

### Limitations/Problems/Questions
* WiFi or other external connection needed for infections status (auto-connect to open WiFis?)
* obtaining accessibility
* all parameters (scanning time, thresholds etc.)
* service UUID is send reversed, RPI and AEM also send in reverse? Don't know BLE specification enough

### some storage math

Due to limited storage, I made some calculations. I have fixed counting of TEKs (14 for two weeks), temporary beacons (1000, longest period for temp. storage is 20 minutes, so recognizing about 1000 different beacons in 20 minutes is possible) and exposure information (choose 500, this is like a limit of infected keys to be met). So the biggest limitation is to store beacons permanently after threshold of 5 minutes. That's what those calculations are for to check, if storage is enough for practical use.

overview of storage in bytes without permanent beacons:
|               | size (B) |  num | overall (B) |
| :-----------: | -------: | ---: | ----------: |
|      TEK      |       21 |   14 |         294 |
| Exposure Info |       20 |  500 |       10000 |
| temp. Beacon  |       32 | 1000 |       32000 |

Additional 4 bytes counting for every type gives overall 42310B used without perm. beacons.

For now, a partition size of 2494464B will leave 2452154B free for met beacons which leads to a total storage of 76629
beacons. This gives the following table, where I added some lower boundaries to calculate with.
| total beacons | aver. per day | aver. for 10 minute window |
| ------------: | ------------: | -------------------------: |
|         50000 |          3571 |                         24 |
|         70000 |          5000 |                         34 |
|         76629 |          5473 |                         38 |

So on average it is possible to meet 38 (24 on a lower boundary) different devices inside of 10 minutes. I have no practical experience/numbers how many beacons are stored on average for a 14-days period in currently running ENA-Apps. But I think regarding the average is calculated for 24h (which is quite unpractical because of sleep and hours without meeting many people), the storage should be enough for the purpose of contact tracing.   

## How to use

### Hardware Required

For base functionality just an ESP32 is required.

### Include into project

To include these components into an esp-idf project, include the following into the CMakeLists.txt

```
include(FetchContent)
FetchContent_Declare(
  esp-ena
  GIT_REPOSITORY https://github.com/Lurkars/esp-ena.git
  GIT_TAG        origin/component
)
FetchContent_MakeAvailable(esp-ena)
set(EXTRA_COMPONENT_DIRS ${esp-ena_SOURCE_DIR}/components)
```

### Configure the project

```
idf.py menuconfig
```

**required**
* enable Bluetooth (BLE)
> Component config -> Bluetooth -> [*] Bluetooth
* add partition-table for storage (currently hardcoded name "ena")
> Partition Table -> Partition table -> (x) Custom partition table CSV
* mbedTLS enable HKDF
> Component config -> mbedTLS -> [*] HKDF algorithm (RFC 5869)
* flash size > 3.9GB
> Serial flasher config -> Flash size ->  (x) 4MB

**recommended**
* BLE *Scan Duplicate* (By Device Address and Advertising Data)
> Component config -> Bluetooth -> Bluetooth controller -> Scan Duplicate Type -> (X) Scan Duplicate By Device Address And Advertising Data

**debug options**
* Log output set to Debug
> Component config -> Log output -> Default log verbosity -> (X) Debug
* Exposure Notification API / Storage enable *Dump storage*
> Exposure Notification API -> Storage -> [X] Dump storage

#### Configure SSL cert manually!

For *ena-eke-proxy* connection over SSL a valid certificate for used server under *components/ena-eke-proxy/certs/cert.pem* is required. 

For my own proxy server, I have added a self signed cert for cwa-proxy.champonthis.de. For using, copy or rename  *components/ena-eke-proxy/certs/cwa-proxy.champonthis.de.pem* to *components/ena-eke-proxy/certs/cert.pem*.

> copy valid cert to *components/ena-eke-proxy/certs/cert.pem*


## Structure

The project is divided in different components. The main.c just wrap up all components. The Exposure Notification API is in **ena** module

### ena

The ena module contains the main functions of eps-ena with Bluetooth scanning and advertising, storing data, handle beacons and check exposure.
* *ena-beacons* handles scanned data by storing temporary beacons, check for threshold and store beacons permanently
* *ena-crypto* covers cryptography part (key creation, encryption etc.)
* *ena-storage* storage part to store own TEKs and beacons
* *ena-bluetooth-scan* BLE scans for detecting other beacons
* *ena-bluetooth-advertise* BLE advertising to send own beacons
* *ena-exposure* compare exposed keys with stored beacons, calculate score and risk
* *ena* run all together and timing for scanning and advertising

### ena-eke-proxy

This module is for connecting to an Exposure Key export proxy server. The server must provide daily (and could hourly) fetch of daily keys in binary blob batches with the following format

| Key Data | Rolling Start Interval Number | Rolling Period | Days Since Onset Of Symptoms |
| :------: | :---------------------------: | :------------: | :--------------------------: |
| 16 bytes |            4 bytes            |    4 bytes     |           4 bytes            |

Request URL is parametrized with {day-string},({hour} in hourly mode,) {page}, {page-size}.

### ena-binary-export  \[deprecated\]

Module to decode Exposure Key export. \[Deprecated through ena-eke-proxy module\]

### ena-cwa \[deprecated\]

Connection to German Exposure App ([Corona Warn App](https://github.com/corona-warn-app)) for download Exposure Key export (and maybe later report infection). \[Deprecated through ena-eke-proxy module\]

### nanopb \[deprecated\]

[Nanopb](https://github.com/nanopb/nanopb) for reading Protocol Buffers of Exposure Key export. Including already generated Headers from *.proto files.  \[Deprecated through ena-eke-proxy module\]
