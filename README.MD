<h1 align = "center">🌟LilyGO T-ETH-Series</h1>

<!-- **English | [中文](README_CN.MD)** -->

## News:

* The changed **ETHClass** is used by default, and **W5500 SPI** Ethernet port support is added


## Product:

| Product(PinMap)                       | SOC                                   | Flash | PSRAM     | Ethernet | POE      | SDCard |
| ------------------------------------- | ------------------------------------- | ----- | --------- | -------- | -------- | ------ |
| [T-ETH-POE][1]                        | ESP32-WROOM-32                        | 4MB   | 4MB(QSPI) | LAN8720  | ✅        | ✅      |
| [T-ETH-POE-PRO][2]                    | ESP32-WROVER-E                        | 16MB  | 8MB(QSPI) | LAN8720  | ✅        | ❌      |
| [T-INTER-COM][3]                      | ESP32-WROVER-E                        | 16MB  | 8MB(QSPI) | LAN8720  | ❌        | ✅      |
| [T-ETH-Lite-ESP32][4]                 | ESP32-WROVER-E                        | 16MB  | 8MB(QSPI) | RTL8201  | (Option) | ✅      |
| [T-ETH-Lite-ESP32S3][5]               | ESP32-S3-WROOM-1                      | 16MB  | 8MB(OPI)  | W5500    | (Option) | ✅      |
| [T-ETH-Lite-POE-Shield][6]            | POE Shield (IEEE802.3 af/Class 0)     |       |           |          |          |        |
| [T-ETH-Lite-ESP32S3-Camera-Shield][6] | Camera Shield (Support OV2640/OV5640) |       |           |          |          |        |

[1]: https://www.lilygo.cc/products/t-internet-poe
[2]: https://www.lilygo.cc
[3]: https://www.lilygo.cc/products/t-internet-com
[4]: https://www.lilygo.cc/products/t-eth-lite?variant=43120880746677
[5]: https://www.lilygo.cc/products/t-eth-lite?variant=43120880779445
[6]: https://www.lilygo.cc/products/t-eth-lite?variant=43120887038133


## Examples

```
examples/
├── UnitTestExample             # Board function example
├── ETHOTA                      # Upgrade firmware via Ethernet
├── HelloServer                 # WebServer using ETH
├── SDWebServer                 # SDWebServer using ETH
├── MQTTClient                  # MQTT Example
├── ESPMQTTSClient              # MQTTS Example
├── StaticIPAddress             # Example of static IP address
├── UDPClientReceiver           # UDP Receiver exampe
├── UDPClientSender             # UDP Sender exampe
├── UnitTestExample             # Hardware test exampe
├── TCPClient                   # TCP Client
├── TCPServer                   # TCP Server
├── RS485_Master                # RS485 Sender (ETH-Pro only)
├── RS485_Slave                 # RS485 Reciver (ETH-Pro only)
├── TFT_ILI9341_Shield          # Screen interface example (ETH-Pro/ETH-Lite-ESP32 only)
├── CameraShield                # Camera shield example (ETH-Lite-ESP32S3 only)
├── LoRaShield                  # LoRa Shield Sender Example (ETH-PRO only)
├── SPIExample                  # SPI initialization example (ETH-PRO only)
├── SPI_Wire_DevicesExample     # SPI bus & Wire sharing example  (ETH-PRO only)
├── SharingSPIBus               # SPI bus sharing example
└── WireExample                 # Wire initialization example
```

## 1️⃣ PlatformIO Quick Start <Recommended>

1. Install [Visual Studio Code](https://code.visualstudio.com/) and [Python](https://www.python.org/)
2. Search for the `PlatformIO` plugin in the `VisualStudioCode` extension and install it.
3. After the installation is complete, you need to restart `VisualStudioCode`
4. After restarting `VisualStudioCode`, select `File` in the upper left corner of `VisualStudioCode` -> `Open Folder` -> select the `LilyGO T-ETH-Series` directory
5. Wait for the installation of third-party dependent libraries to complete
6. Click on the `platformio.ini` file, and in the `platformio` column
7. First change the default_envs to the board used
8. Uncomment one of the lines src_dir = xxxx to make sure only one line works
9. Click the (✔) symbol in the lower left corner to compile
10. Connect the board to the computer USB (If there is no onboard downloader, USB2TTL must be connected)
11. Click (→) to upload firmware
12. Click (plug symbol) to monitor serial output
13. Since **T-LITE-ESP32** uses **IO0** as the clock input, the default working level of **DTR** is high level, so after the download is completed, you need to remove the connection between **DTR** and **IO0**, so that ETH will work, otherwise the initialization will fail
14. For **T-LITE-ESP32**, when uploading a sketch, you need to press and hold the *BOOT* button to run the upload


## 2️⃣ Arduino IDE Quick Start
* It is recommended to use platformio without cumbersome steps
1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [Arduino ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/) 
3. Copy everything from this repository [lib folder](./lib/)  to Arduino library folder (e.g. C:\Users\YourName\Documents\Arduino\libraries)
4. Open ArduinoIDE -> Tools
   ![](image/ArduinoIDE.jpg)
5. Open the [examples](./examples/),such as [examples/UnitTestExample](./examples/UnitTestExample)
6. Switch to the [utilities.h](./examples/UnitTestExample/utilities.h) file, at the top of the sketch, uncomment the definition of the board name you use , such as **LILYGO_T_ETH_POE**
7.  Connect external [USB2TTL](https://www.amazon.com/usb-ttl-adapter/s?k=usb+to+ttl+adapter)，such as **CP2102,CP2104,CH340X**
      * Has an onboard downloader(**T-ETH-Pro**)
          If the board has a USB-C, plug it directly into the USB-C, select the corresponding port, and upload.
      * no onboard downloader(**T-Lite-ESP32/T-Lite-ESP32S3/T-ETH-POE**)
          Additional USB2TTL is required to connect the board with USB2TTL
      * Connection example:


        | USB2TTL(Has DTR,RTS) | USB2TTL(NO DTR,RTS) | T-ETH-Series |
        | -------------------- | ------------------- | ------------ |
        | 5V                   | 5V                  | 5V IN        |
        | GND                  | GND                 | GND          |
        | RX                   | RX                  | TXD          |
        | TX                   | TX                  | RXD          |
        | RTS                  |                     | RST          |
        | DTR                  |                     | IO0          |
8. Select the corresponding port and click Upload <If the upload fails, View the FAQ below>
9. Since **T-LITE-ESP32** uses **IO0** as the clock input, the default working level of **DTR** is high level, so after the download is completed, you need to remove the connection between **DTR** and **IO0**, so that ETH will work, otherwise the initialization will fail


<h2 align = "left">5️⃣ ESP32 basic examples </h2>

- [BLE Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)
- [WiFi Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
- [SPIFFS Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/SPIFFS)
- [FFat Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/FFat)
- For more examples of esp32 chip functions, please refer to [arduino-esp32-libraries](https://github.com/espressif/arduino-esp32/tree/master/libraries)


<h2 align = "left">6️⃣ FAQ </h2>

1. Unable to upload skecth,Please enter the upload mode manually. 
    1. Connect the board via the USB cable or connect USB2TTL
    2. Press and hold the BOOT button , While still pressing the BOOT button, press RST
    3. Release the RST
    4. Finally release the BOOT Button(The BOOT button must be released last)
    5. Upload sketch


