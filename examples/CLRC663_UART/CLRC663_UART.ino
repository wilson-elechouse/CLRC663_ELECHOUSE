/*
  CLRC663 UART tag demo

  - Tested on ESP32 using UART2.
  - Put the module into UART mode (IFSEL pins / resistors per the CLRC663 module docs).
  - Wiring (ESP32): TX -> RC663 RX, RX -> RC663 TX, GND, 3V3. IRQ is optional.
*/

/* Hardware connection UART
  ELECHOUSE CLRC663 MODULE  --------  ESP32 DEV MODULE
  5V                        --------  5V
  PDOWN                     --------  GND
  MOSI/RX                   --------  D17
  SCK/SCL                   --------  NC
  MISO/TX                   --------  D16
  NSS/SDA                   --------  D5
  IRQ                       --------  NC
  3V3                       --------  3V3
  GND                       --------  GND
*/


#include <Arduino.h>
#include <HardwareSerial.h>
#include "CLRC663.h"
#include "mfrc630_def.h"

// ESP32 UART2 pins used in the sanity test example
#define RC663_RX 16
#define RC663_TX 17
#define IF3_PIN 5
#define IRQ_PIN  -1
#define RC663_BAUD 115200

HardwareSerial RC663Serial(2);
CLRC663 reader(&RC663Serial, RC663_RX, RC663_TX, RC663_BAUD, -1, IRQ_PIN);

// one-time diagnostics flag
bool diag_done = false;

uint32_t calc_uart_baud(uint8_t serialSpeed) {
  uint8_t br_t0 = (serialSpeed >> 5) & 0x07;
  uint8_t br_t1 = serialSpeed & 0x1F;
  if (br_t0 == 0) {
    return 27120000UL / (br_t1 + 1);
  }
  return 27120000UL / (br_t1 + 33) / (1UL << (br_t0 - 1));
}

void print_hex_byte(uint8_t value) {
  if (value < 0x10) {
    Serial.print("0");
  }
  Serial.print(value, HEX);
}

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

void print_14443a_type_hint(uint16_t atqa, uint8_t sak) {
  Serial.print("Type hint: ");
  if (sak & 0x04) {
    Serial.print("UID not complete (cascade). ");
  }
  if (sak & 0x20) {
    Serial.print("ISO-DEP (Type 4A). ");
  }
  if (sak == 0x00) {
    Serial.print("Ultralight/NTAG or Type A (no ISO-DEP). ");
  } else if (sak == 0x08) {
    Serial.print("MIFARE Classic 1K (likely). ");
  } else if (sak == 0x18) {
    Serial.print("MIFARE Classic 4K (likely). ");
  }
  uint8_t atqa_low = static_cast<uint8_t>(atqa & 0xFF);
  uint8_t uid_size = (atqa_low >> 6) & 0x03;
  const char *uid_desc = "unknown";
  if (uid_size == 0) {
    uid_desc = "single";
  } else if (uid_size == 1) {
    uid_desc = "double";
  } else if (uid_size == 2) {
    uid_desc = "triple";
  }
  Serial.print("UID size=");
  Serial.print(uid_desc);
  Serial.println(".");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("startup CLRC663 UART RFID-reader..");
  reader.begin();
  delay(100);
  pinMode(IF3_PIN, OUTPUT);
  digitalWrite(IF3_PIN, HIGH);
  delay(2);
  reader.softReset();

  uint8_t version = 0x00;
  for (uint8_t attempt = 0; attempt < 5; attempt++) {
    version = reader.read_reg(MFRC630_REG_VERSION);
    if (version != 0x00 && version != 0xFF) {
      break;
    }
    delay(20);
  }
  Serial.print("SW version (UART reg): 0x");
  if (version < 0x10) { Serial.print("0"); }
  Serial.print(version, HEX);
  Serial.print(" (major ");
  Serial.print((version >> 4) & 0x0F);
  Serial.print(", minor ");
  Serial.print(version & 0x0F);
  Serial.println(")");
  if (version == 0x00 || version == 0xFF) {
    Serial.println("Warning: UART read looks invalid.");
  }

  runDiagnosticsOnce();
}

void loop() {
  // Card scan code.
  uint8_t uid[10] = {0};
  uint8_t uid_len = 0;
  Serial.println("Scan (UART)...");
  reader.softReset();  // also re-asserts UART baud internally now
  reader.AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);
  delay(5);
  uint16_t atqa = reader.iso14443a_reqa_debug();
  if (atqa != 0) {
    uint8_t sak = 0;
    uid_len = reader.iso14443a_select_debug(uid, &sak);
    Serial.print("ATQA=0x");
    print_hex_byte(static_cast<uint8_t>(atqa >> 8));
    print_hex_byte(static_cast<uint8_t>(atqa & 0xFF));
    Serial.print(" SAK=0x");
    print_hex_byte(sak);
    Serial.println("");
    if (uid_len != 0) {
      Serial.print("ISO-14443 tag found! UID of ");
      Serial.print(uid_len);
      Serial.print(" bytes: ");
      print_block(uid, uid_len);
      print_14443a_type_hint(atqa, sak);
      Serial.print("\n");
      delay(200);
      return;
    }
    Serial.println("ISO-14443 select failed.");
  } else {
    Serial.println("ISO-14443 REQA no response.");
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
  uint32_t calcBaud = calc_uart_baud(serialSpeed);
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
