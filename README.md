# mhv2109's Garden Monitor

Device to track conditions in my garden. Connects to my home automation through MQTT. Tracks air temperature and humidity, soil moisture, and light intensity.

## Requirements
* [ESP-IDF](https://www.espressif.com/en/products/sdks/esp-idf) (currently using v4.2)
* [MQTT broker](https://mqtt.org/) (I'm using [Eclipse Mosquitto](https://mosquitto.org/))
* WiFi network
* Hardware:
  - [ESP 32 development board](https://www.espressif.com/en/products/devkits)
  - [Adafruit STEMMA soil sensor](https://www.adafruit.com/product/4026)
  - [SparkFun APDS 3901](https://www.sparkfun.com/products/retired/14350)
  - [SHT 20](https://www.dfrobot.com/product-1636.html)

## Getting Started
### Wiring it all up
I connected all devices to I2C Bus 0 using pins D15 and D2. I kept their default I2C addresses (if applicable).

### Configuration
Project configuration is handled using KConfig, and thus, configs are compile-time constants.

To set configurations, run `idf.py menuconfig`

* WiFi configuration [here](./components/wifi/README.md#Configuration)
* MQTT broker configuration [here](./components/gm_mqtt/README.md#Configuration)

## Building
Build, flash, and monitor using `idf.py --port <port> flash monitor`.

My board/peripheral setup has the I2C devices using the 3.3v power supply near the USB port. I've noticed that I cannot flash my device with my peripherals connected to that 3.3v pin. I believe that it's to do with the I2C bus, but I must disconnect the power to my peripherals from the 3.3v pin to flash.
