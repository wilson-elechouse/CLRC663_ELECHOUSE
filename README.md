# CLRC663 - RFID-reader library

An Arduino/PlatformIO library for the CLRC633 RFID-reader module.

Tested with Arduino IDE 2.x and PlatformIO.

Copyright (c) 2023 tueddy (Dirk Carstensen) 



## Features:
   - can be connected via SPI, I2C, or UART (2 GPIOs less for I2C/UART)
   - UART host interface support (for modules strapped for UART)
   - read ISO-14443 UID's
   - read ISO-18693 UID's
   - support for ICode-SLIX password protected tags (privacy mode) / custom tags
   - LPCD - low power card detection
   - Auto calibration for LPCD-mode. Avoid false wakeups like in PN5180-library: https://github.com/tueddy/PN5180-Library
  

Some header declarations and functions taken and extended from https://github.com/kenny5660/CLRC663.git

You can get this reader module on AliExpress or eBay:

![Module](https://user-images.githubusercontent.com/11274319/222262881-19112874-c9b8-4305-ae55-17aae3fbe02f.png)

![CLRC663](https://user-images.githubusercontent.com/11274319/222130502-1bac1d0e-7034-4ce1-81d3-b94f3365112e.jpg)

## Wireing

The CLRC663 reader can be connected with SPI or I2C, modules are delivered in SPI mode. Connect these pins:

- GND/3.3V for power
- Connect SD (shutdown/Power-down) to GND
- IRQ_PIN is optional and can be IRQ_PIN=-1. The IRQ-Pin is for wakeup the ESP-32 from deep-sleep.


For SPI use MOSI, MISO, SCLK

Create a SPI instance with
```` c++
CLRC663 reader(&SPI, CHIP_SELECT, IRQ_PIN);
````

For I2C use SDA and SCL
Create a I2C instance with
```` c++
CLRC663 reader(0x2A, IRQ_PIN);
````

For UART put the module into UART mode (IFSEL pins / resistors) and connect TX/RX:
```` c++
HardwareSerial RC663Serial(2);
CLRC663 reader(&RC663Serial, RX_PIN, TX_PIN, 115200, RESET_PIN, IRQ_PIN);
````


Module is delivered in SPI mode by default. If you need to change your CLRC663 module to work with I2C, you can easily do so by using a hot air gun/rework station and changing two resistors R2 -> R1, and R4 -> R8:
![I2C](https://user-images.githubusercontent.com/11274319/222263210-958d5883-7d8d-4567-b8d4-93587f05dde8.jpg)

I2C address is 0x2A. 
More default information changing to I2C and change I2C address to 0x28 here: 
https://blog.edhayes.us/2022/02/23/clrc663-module-spi-i2c/



## Examples

- `examples/CLRC663-SPI/CLRC663-SPI.ino` shows SPI wiring and basic tag reading.
- `examples/CLRC663_I2C/CLRC663_I2C.ino` shows I2C wiring and tag reading.
- `examples/CLRC663_UART/CLRC663_UART.ino` shows UART wiring and tag reading (set module to UART mode).
- `examples/UART-testing/UART-testing.ino` is a minimal serial communication smoke test.

## Installation

For Arduino IDE 2.x download the library as zip and use **Sketch → Include Library → Add .ZIP Library…**. Or clone this project into your Arduino `libraries` folder, e.g.:
```
cd ~/Documents/Arduino/libraries
git clone https://github.com/wilson-elechouse/CLRC663_ELECHOUSE.git
```

For PlatformIO include the library in `platformio.ini`:
```
lib_deps =
  https://github.com/wilson-elechouse/CLRC663_ELECHOUSE.git
```
