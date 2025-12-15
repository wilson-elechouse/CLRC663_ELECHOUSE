/*
  CLRC663 UART tag demo

  - Tested on ESP32 using UART2.
  - Put the module into UART mode (IFSEL pins / resistors per the CLRC663 module docs).
  - Wiring (ESP32): TX -> RC663 RX, RX -> RC663 TX, GND, 3V3. IRQ is optional.
*/

#include <Arduino.h>
#include <HardwareSerial.h>
#include "CLRC663.h"
#include "mfrc630_def.h"

// ESP32 UART2 pins used in the sanity test example
#define RC663_RX 16
#define RC663_TX 17
#define IRQ_PIN  -1
#define RC663_BAUD 115200

HardwareSerial RC663Serial(2);
CLRC663 reader(&RC663Serial, RC663_RX, RC663_TX, RC663_BAUD, -1, IRQ_PIN);

// one-time diagnostics flag
bool diag_done = false;

// Hex print for blocks without printf.
void print_block(uint8_t *block, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    if (block[i] < 16) {
      Serial.print("0");
      Serial.print(block[i], HEX);
    } else {
      Serial.print(block[i], HEX);
    }
    Serial.print(" ");
  }
  Serial.println("");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("startup CLRC663 UART RFID-reader..");
  reader.begin();

  Serial.print("CLRC663 version: 0x");
  uint8_t version = reader.getVersion();
  if (version < 0x10) { Serial.print("0"); }
  Serial.println(version, HEX);

  runDiagnosticsOnce();
}

void loop() {
  uint8_t uid[10] = {0};
  bool cardFound = false;

  Serial.println("Scan (UART)...");
  reader.softReset();  // also re-asserts UART baud internally now
  reader.AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);
  uint8_t uid_len = reader.read_iso14443_uid(uid);

  if (uid_len != 0) {
    Serial.print("ISO-14443 tag found! UID of ");
    Serial.print(uid_len);
    Serial.print(" bytes: ");
    print_block(uid, uid_len);
    Serial.print("\n");
    delay(200);
    return;
  }

  reader.softReset();
  reader.AN1102_recommended_registers(MFRC630_PROTO_ISO15693_1_OF_4_SSC);

  uint8_t password[] = {0x0F, 0x0F, 0x0F, 0x0F};
  uid_len = reader.read_iso18693_uid(uid, password);
  if (uid_len != 0) {
    Serial.print("ISO-18693 tag found! UID of ");
    Serial.print(uid_len);
    Serial.print(" bytes: ");
    print_block(uid, uid_len);
    Serial.print("\n");
    delay(200);
    return;
  }

  Serial.println("No tag, retrying...");
  delay(300);
}

// Perform UART-specific diagnostics once after startup.
void runDiagnosticsOnce() {
  if (diag_done) return;
  diag_done = true;

  // Check SERIALSPEED register and computed baud.
  uint8_t serialSpeed = reader.read_reg(MFRC630_REG_SERIALSPEED);
  uint32_t calcBaud = (27120000UL / (2 * (serialSpeed + 1)));
  Serial.print("SERIALSPEED reg: 0x");
  if (serialSpeed < 0x10) Serial.print("0");
  Serial.print(serialSpeed, HEX);
  Serial.print(" -> calc baud ~");
  Serial.println(calcBaud);

  // Probe a write/read round-trip on DRVMOD.
  uint8_t drv_before = reader.read_reg(MFRC630_REG_DRVMOD);
  reader.write_reg(MFRC630_REG_DRVMOD, 0x8E); // field on value used later
  uint8_t drv_after = reader.read_reg(MFRC630_REG_DRVMOD);
  Serial.print("DRVMOD before/after: 0x");
  if (drv_before < 0x10) Serial.print("0");
  Serial.print(drv_before, HEX);
  Serial.print(" -> 0x");
  if (drv_after < 0x10) Serial.print("0");
  Serial.println(drv_after, HEX);

  // Single REQA probe to see if transceive triggers IRQ/FIFO changes.
  reader.write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_IDLE);
  reader.write_reg(MFRC630_REG_FIFOCONTROL, 1 << 4); // flush FIFO
  reader.write_reg(MFRC630_REG_FIFODATA, 0x26);      // REQA
  reader.write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_TRANSCEIVE);
  delay(5);
  uint8_t irq0 = reader.get_irq0();
  uint8_t irq1 = reader.get_irq1();
  uint8_t fifoLen = reader.read_reg(MFRC630_REG_FIFOLENGTH);
  Serial.print("REQA probe: IRQ0=0x");
  if (irq0 < 0x10) Serial.print("0");
  Serial.print(irq0, HEX);
  Serial.print(" IRQ1=0x");
  if (irq1 < 0x10) Serial.print("0");
  Serial.print(irq1, HEX);
  Serial.print(" FIFO=");
  Serial.println(fifoLen);
}
