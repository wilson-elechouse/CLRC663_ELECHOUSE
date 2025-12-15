#include <HardwareSerial.h>

// 定义 ESP32 连接 CLRC663 的引脚
#define RXD2 16
#define TXD2 17

// CLRC663 寄存器地址
#define REG_VERSION 0x7F 
#define REG_COMMAND 0x00

// 实例化 HardwareSerial 对象，使用 UART2
HardwareSerial RC663Serial(2);

void setup() {
  // 初始化调试串口 (连接电脑)
  Serial.begin(115200);
  while (!Serial) { delay(10); } // 等待串口就绪
  
  Serial.println("\n--- ESP32 CLRC663 UART Test ---");

  // 初始化与 CLRC663 的通信串口
  // 参数: 波特率, 数据位/停止位/校验, RX引脚, TX引脚
  // CLRC663 默认波特率 115200, 8N1
  RC663Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  
  delay(100); // 给一点上电稳定时间

  // 执行软复位 (可选，但推荐)
  // 写入 Command 寄存器 (0x00), 发送 SoftReset 命令 (0x1F)
  writeReg(REG_COMMAND, 0x1F); 
  delay(50); // 等待复位完成
  
  Serial.println("Soft Reset sent.");
}

void loop() {
  // 尝试读取 Version 寄存器
  uint8_t version = readReg(REG_VERSION);

  Serial.print("Read Version Register (0x7F): 0x");
  if (version < 0x10) Serial.print("0"); // 补零格式化
  Serial.println(version, HEX);

  // 检查结果
  // CLRC66301/02 通常返回 0x18, CLRC66303 返回 0x1A
  if (version == 0x18 || version == 0x1A) {
    Serial.println(">> SUCCESS: CLRC663 detected!");
  } else if (version == 0x00 || version == 0xFF) {
    Serial.println(">> ERROR: Communication failed (Check wiring or IFSEL pins).");
  } else {
    Serial.println(">> UNKNOWN: Valid communication but unknown version.");
  }

  delay(2000); // 每2秒读取一次
}

// --- 底层通信函数 ---

// 读取寄存器函数
uint8_t readReg(uint8_t regAddr) {
  // 1. 清空缓冲区，防止读到旧数据
  while (RC663Serial.available()) {
    RC663Serial.read();
  }

  // 2. 构造地址字节: (Address << 1) | 1 表示读
  uint8_t addressByte = (regAddr << 1) | 0x01;
  
  // 3. 发送请求
  RC663Serial.write(addressByte);

  // 4. 等待响应 (简单的超时机制)
  unsigned long startTime = millis();
  while (!RC663Serial.available()) {
    if (millis() - startTime > 100) {
      Serial.println("Timeout waiting for response!");
      return 0xFF; // 超时返回
    }
  }

  // 5. 读取返回的一个字节数据
  return RC663Serial.read();
}

// 写入寄存器函数
void writeReg(uint8_t regAddr, uint8_t value) {
  // 1. 构造地址字节: (Address << 1) & 0xFE 表示写
  uint8_t addressByte = (regAddr << 1) & 0xFE;

  // 2. 发送地址和数据
  RC663Serial.write(addressByte);
  RC663Serial.write(value);
}