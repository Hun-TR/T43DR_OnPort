#include "uart_handler.h"
#include "log_system.h"

#define UART_RX_PIN 4
#define UART_TX_PIN 2
#define UART_BAUD   115200
#define UART_PORT   Serial2
#define UART_TIMEOUT 1000

String lastResponse = "";

void initUART() {
  UART_PORT.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  addLog("UART başlatıldı.");
}

bool requestFirstFault() {
  while (UART_PORT.available()) UART_PORT.read();
  UART_PORT.print("12345v\n");

  unsigned long start = millis();
  while (millis() - start < UART_TIMEOUT) {
    if (UART_PORT.available()) {
      lastResponse = UART_PORT.readStringUntil('\n');
      lastResponse.trim();
      return true;
    }
  }
  return false;
}

bool requestNextFault() {
  while (UART_PORT.available()) UART_PORT.read();
  UART_PORT.print("n\n");

  unsigned long start = millis();
  while (millis() - start < UART_TIMEOUT) {
    if (UART_PORT.available()) {
      lastResponse = UART_PORT.readStringUntil('\n');
      lastResponse.trim();
      return true;
    }
  }
  return false;
}

String getLastFaultResponse() {
  return lastResponse;
}
