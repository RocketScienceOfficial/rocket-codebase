#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// --- DATALINK ---
#define CFG_GCS_SRC_ID 0xDF
#define CFG_GCS_DST_ID 0x11

// --- LORA ---
#define CFG_LORA_SPI 2
#define CFG_LORA_PIN_CS 18
#define CFG_LORA_SPI_SCK_PIN 5
#define CFG_LORA_SPI_MOSI_PIN 27
#define CFG_LORA_SPI_MISO_PIN 19
#define CFG_LORA_PIN_DIO0 26
#define CFG_LORA_PIN_RESET 23
#define CFG_LORA_FREQ 433
#define CFG_LORA_BANDWIDTH 250
#define CFG_LORA_SF 7
#define CFG_LORA_TX_POWER 17

// --- UART ---
#define CFG_UART 1
#define CFG_UART_FREQ 9600
#define CFG_UART_TX 34
#define CFG_UART_RX 12

// --- I2C ---
#define CFG_I2C 0
#define CFG_I2C_FREQUENCY 400000
#define CFG_I2C_SDA_PIN 21
#define CFG_I2C_SCL_PIN 22

// --- GPIO ---
#define CFG_BUTTON_PIN 38

// --- SIM ---
#define CFG_SIM_SERIAL_PORT 12349
#define CFG_SIM_LORA_PORT 12348

#endif