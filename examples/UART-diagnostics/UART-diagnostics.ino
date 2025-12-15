/*
  CLRC663 UART diagnostics

  - Confirms UART register R/W, SerialSpeed setting, and a single REQA probe.
  - Defaults: ESP32 UART2 pins RX=16, TX=17, baud=115200. Adjust as needed.
  - Ensure the module is strapped for UART mode (IFSEL pins/resistors).
*/

#include <Arduino.h>
#include <HardwareSerial.h>
#include "CLRC663.h"
#include "mfrc630_def.h"

#define RC663_RX 16
#define RC663_TX 17
#define IRQ_PIN  -1
#define RC663_BAUD 115200
// Set to true to force the chip's SERIALSPEED register to match RC663_BAUD at startup.
#define FORCE_SERIALSPEED true

HardwareSerial RC663Serial(2);
CLRC663 reader(&RC663Serial, RC663_RX, RC663_TX, RC663_BAUD, -1, IRQ_PIN);

uint8_t serialSpeedSettingForBaud(uint32_t baud) {
  uint32_t setting = (27120000UL / (2 * baud));
  if (setting > 0) setting -= 1;
  if (setting > 255) setting = 255;
  return (uint8_t)setting;
}

void printHexByte(uint8_t v) {
  if (v < 0x10) Serial.print("0");
  Serial.print(v, HEX);
}

void probeSerialSpeed() {
  uint8_t regVal = reader.read_reg(MFRC630_REG_SERIALSPEED);
  uint32_t calcBaud = (27120000UL / (2 * (regVal + 1)));
  Serial.print("SERIALSPEED reg: 0x");
  printHexByte(regVal);
  Serial.print(" -> calc baud ~");
  Serial.println(calcBaud);

  if (FORCE_SERIALSPEED) {
    uint8_t desired = serialSpeedSettingForBaud(RC663_BAUD);
    reader.write_reg(MFRC630_REG_SERIALSPEED, desired);
    uint8_t verify = reader.read_reg(MFRC630_REG_SERIALSPEED);
    Serial.print("Forced SERIALSPEED to 0x");
    printHexByte(desired);
    Serial.print(", verify readback: 0x");
    printHexByte(verify);
    Serial.println();
  }
}

void probeDrvMod() {
  uint8_t before = reader.read_reg(MFRC630_REG_DRVMOD);
  reader.write_reg(MFRC630_REG_DRVMOD, 0x8E);
  uint8_t after = reader.read_reg(MFRC630_REG_DRVMOD);
  Serial.print("DRVMOD before/after: 0x");
  printHexByte(before);
  Serial.print(" -> 0x");
  printHexByte(after);
  Serial.println();
}

void probeReqa() {
  reader.write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_IDLE);
  reader.write_reg(MFRC630_REG_FIFOCONTROL, 1 << 4); // flush FIFO
  reader.write_reg(MFRC630_REG_FIFODATA, 0x26);      // REQA
  reader.write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_TRANSCEIVE);
  delay(5);
  uint8_t irq0 = reader.get_irq0();
  uint8_t irq1 = reader.get_irq1();
  uint8_t fifoLen = reader.read_reg(MFRC630_REG_FIFOLENGTH);
  Serial.print("REQA probe: IRQ0=0x");
  printHexByte(irq0);
  Serial.print(" IRQ1=0x");
  printHexByte(irq1);
  Serial.print(" FIFO=");
  Serial.println(fifoLen);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("CLRC663 UART diagnostics starting...");
  reader.begin();

  Serial.print("CLRC663 version: 0x");
  printHexByte(reader.getVersion());
  Serial.println();

  probeSerialSpeed();
  probeDrvMod();

  // Minimal protocol setup before REQA probe.
  reader.AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);
  probeReqa();
}

void loop() {
  // Idle: diagnostics are one-shot in setup.
  delay(1000);
}
