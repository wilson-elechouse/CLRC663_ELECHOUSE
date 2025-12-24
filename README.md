# CLRC663 (ELECHOUSE) — ESP32 Arduino Library + Module Guide

This repository provides an **open-source ESP32 Arduino library** and examples for the **ELECHOUSE CLRC663 Reader Module V1** (on-board PCB antenna).

- Product page: https://www.elechouse.com/product/clrc663-reader-module/
- Library repository: https://github.com/wilson-elechouse/CLRC663_ELECHOUSE

---

## 1. Module Overview (CLRC663 Reader Module V1)

**CLRC663 Reader Module V1** is a high-performance 13.56 MHz HF/NFC reader module designed for:
- Access control / attendance terminals
- Industrial identification / readers
- IoT device integration
- Developer learning & prototyping

### Key Features
- **Tri-interface in one module:** **SPI / I²C / UART** (selected via on-board DIP switch, guided by PCB silkscreen)
- **Open-source ESP32 Arduino library:** ready-to-run examples for fast bring-up
- **Long read range (verified):** up to **9 cm** with a standard PVC white card (see test conditions below)
- **Robust performance / strong anti-interference capability**
- **Low-power operation:** supports **auto sleep** and **card-present wake-up** (**LPCD**)
- **SAM-ready expansion:** reserved interface for external SAM connection (SAM implementation depends on your solution)

### Verified Read Range (Defined Conditions)
**Max measured:** up to **9 cm** (PVC white card), card facing the antenna plane  
- **Card types:** MIFARE One S70 and ISO15693 (UID reading)
- **Orientation:** aligned, facing antenna (front side)
- **Power:** TVDD 5V, VDD 3.3V
- **Environment:** no ferrite, no metal nearby, no known interference sources
- **Distance definition:** card-to-antenna-plane distance

**Recommended stable operating range:**
- **W/o ferrite shee:** approx. **2.5–8.7 cm** stable
- **W/ ferrite sheet on backside:** approx. **0–7.5 cm** stable

> Disclaimer: Read range depends heavily on card type, antenna tuning, power configuration, and environment (metal, ferrite, EMI sources, enclosure materials, etc.). Always validate in your final installation environment.

### Mechanical Summary
- **Board size (V1):** ~ **40.02 mm × 42.36 mm**
- **On-board PCB antenna:** 4-turn, ~ **39.07 mm × 41.73 mm**
- Pin names / silkscreen: printed on PCB (also shown on the product images/manual)

---

## 2. Supported Protocols (Library Status)

### Verified in this repository
- **ISO14443A** (tested)
- **ISO15693** (tested)

> Note: The CLRC66303 silicon supports multiple HF/NFC standards. This repo focuses on **practical, tested** ESP32 Arduino examples.

---

## 3. Quick Start (Arduino IDE)

1. Download this repo as a ZIP and install it via:  
   **Arduino IDE → Sketch → Include Library → Add .ZIP Library...**
2. Open an example:  
   **File → Examples → CLRC663_ELECHOUSE → ...**
3. Select board: **ESP32 Dev Module**
4. Set interface mode (SPI / I²C / UART) on the module (see section 5)
5. Wire the module according to the selected mode (see section 4)
6. Upload and run

---

## 4. Hardware Connections (ESP32 Dev Module)

The module uses a single header where signals are **re-used** across SPI / I²C / UART modes.  
**Make sure your DIP switch mode matches your wiring before powering on.**

### 4.1 SPI Mode Wiring (ESP32 Dev Module)

| ELECHOUSE CLRC663 MODULE | ESP32 DEV MODULE |
|---|---|
| 5V | 5V |
| PDOWN | GND |
| MOSI/RX | D23 |
| SCK/SCL | D18 |
| MISO/TX | D19 |
| NSS/SDA | D5 |
| IRQ | D16 |
| 3V3 | 3V3 |
| GND | GND |

### 4.2 I²C Mode Wiring (ESP32 Dev Module)

| ELECHOUSE CLRC663 MODULE | ESP32 DEV MODULE |
|---|---|
| 5V | 5V |
| PDOWN | GND |
| MOSI/RX | NC *(or GND for IF0=0 address bit)* |
| SCK/SCL | D22 |
| MISO/TX | NC *(or GND for IF1=0 address bit)* |
| NSS/SDA | D21 |
| IRQ | D16 |
| 3V3 | 3V3 |
| GND | GND |

**I²C address bits (IF0/IF1) note:**  
- In I²C mode, **MOSI/RX = IF0** and **MISO/TX = IF1** can be left **NC** (default = 1), or pulled to **GND** (bit = 0) to change I²C address.  
- Changing IF0/IF1 wiring may affect other interface modes. Only modify IF0/IF1 if you will **permanently use I²C mode**.

### 4.3 UART Mode Wiring (ESP32 Dev Module)

| ELECHOUSE CLRC663 MODULE | ESP32 DEV MODULE |
|---|---|
| 5V | 5V |
| PDOWN | GND |
| MOSI/RX | D17 |
| SCK/SCL | NC |
| MISO/TX | D16 |
| NSS/SDA | D5 |
| IRQ | NC |
| 3V3 | 3V3 |
| GND | GND |

> Notes:
> - UART RX/TX pins and baud rate are defined inside the UART examples. Adjust the `#define` values in the sketch if you use different GPIOs.
> - Some UART examples use an extra GPIO (e.g., `NSS/SDA`) as a control line; follow the exact sketch you are running.

---

## 5. Interface Mode Switching (DIP Switch)

This module supports **SPI / I²C / UART** selection using an on-board **2-position DIP switch**.

**How to switch (high-level):**
1. **Power off** your system.
2. Set the DIP switch by following the **silkscreen guidance on the PCB** (visual alignment to the printed mode label).
3. Wire the module according to the selected interface (section 4).
4. Power on and run the corresponding example.

> Detailed DIP-switch mapping (including exact ON/OFF combinations) is provided in the **Module Operation Manual**.  
> This README keeps the switching section concise by design.

---
