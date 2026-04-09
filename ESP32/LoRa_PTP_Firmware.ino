#include "LoRaWan_APP.h"
#include "Arduino.h"

// LoRa settings
#define RF_FREQUENCY 915000000   // 915 MHz
#define TX_OUTPUT_POWER 14
#define LORA_BANDWIDTH 0         // 125 kHz
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define BUFFER_SIZE 1024
#define RX_TIMEOUT_MS 2000

uint8_t txBuffer[BUFFER_SIZE];
uint8_t rxBuffer[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

volatile bool txDone = false;
volatile bool rxDone = false;
volatile bool rxTimeout = false;

uint16_t rxSize = 0;
int16_t rxRssi = 0;
int8_t rxSnr = 0;

void OnTxDone(void) {
  txDone = true;
  Radio.Sleep();
}

void OnTxTimeout(void) {
  txDone = false;
  Radio.Sleep();
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  if (size > BUFFER_SIZE) size = BUFFER_SIZE;

  memcpy(rxBuffer, payload, size);
  rxSize = size;
  rxRssi = rssi;
  rxSnr = snr;

  rxDone = true;
  Radio.Sleep();
}

void OnRxTimeout(void) {
  rxTimeout = true;
  Radio.Sleep();
}

void setup() {
  Serial.begin(115200);

  // Newer Heltec API requires 2 arguments
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.RxTimeout = OnRxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  Serial.println("ESP32 LoRa Ready");
}

void loop() {
  // Must be called regularly so callbacks can run
  Radio.IrqProcess();

  if (Serial.available()) {
    uint8_t cmd = Serial.read();

    // =========================
    // SEND MODE (0x02)
    // =========================
    if (cmd == 0x02) {
      while (Serial.available() < 4) {
        Radio.IrqProcess();
      }

      uint32_t size = 0;
      for (int i = 0; i < 4; i++) {
        size = (size << 8) | (uint8_t)Serial.read();
      }

      if (size > BUFFER_SIZE) size = BUFFER_SIZE;

      uint32_t received = 0;
      while (received < size) {
        Radio.IrqProcess();
        if (Serial.available()) {
          txBuffer[received++] = (uint8_t)Serial.read();
        }
      }

      txDone = false;
      Radio.Send(txBuffer, size);

      // Wait for TX complete
      unsigned long start = millis();
      while (!txDone && millis() - start < 5000) {
        Radio.IrqProcess();
      }
    }

    // =========================
    // RECEIVE MODE (0x03)
    // =========================
    else if (cmd == 0x03) {
      rxDone = false;
      rxTimeout = false;
      rxSize = 0;

      Radio.Rx(0);

      unsigned long start = millis();
      while (!rxDone && !rxTimeout && millis() - start < RX_TIMEOUT_MS) {
        Radio.IrqProcess();
      }

      if (rxDone && rxSize > 0) {
        uint8_t sizeBytes[4];
        sizeBytes[0] = (rxSize >> 24) & 0xFF;
        sizeBytes[1] = (rxSize >> 16) & 0xFF;
        sizeBytes[2] = (rxSize >> 8) & 0xFF;
        sizeBytes[3] = rxSize & 0xFF;

        Serial.write(sizeBytes, 4);
        Serial.write(rxBuffer, rxSize);
      } else {
        uint8_t zero[4] = {0, 0, 0, 0};
        Serial.write(zero, 4);
      }
    }
  }
}