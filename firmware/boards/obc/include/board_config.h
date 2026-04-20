#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// --- ADC ---
#define CFG_ADC_VREF 2.5f

// --- PINS ---
#define CFG_PIN_LED 15
#define CFG_PIN_BUZZER 22
#define CFG_PIN_IGN_EN_1 21
#define CFG_PIN_IGN_EN_2 18
#define CFG_PIN_IGN_EN_3 20
#define CFG_PIN_IGN_EN_4 19
#define CFG_PIN_IGN_DET_1 29
#define CFG_PIN_IGN_DET_2 28
#define CFG_PIN_IGN_DET_3 27
#define CFG_PIN_IGN_DET_4 26
#define CFG_PIN_CS_H3LIS 0
#define CFG_PIN_CS_LSM 1
#define CFG_PIN_CS_MMC 9
#define CFG_PIN_CS_BMI_ACC 6
#define CFG_PIN_CS_BMI_GYRO 5
#define CFG_PIN_CS_MS56 7
#define CFG_PIN_CS_NEO 12
#define CFG_PIN_CS_ADS 13
#define CFG_PIN_VBAT 25
#define CFG_PIN_5V 23
#define CFG_PIN_3V3 24

// --- SPI ---
#define CFG_SPI 0
#define CFG_SPI_FREQ 5 * 1000 * 1000
#define CFG_SPI_SCK_PIN 2
#define CFG_SPI_MOSI_PIN 3
#define CFG_SPI_MISO_PIN 4

// --- UART ---
#define CFG_UART 0
#define CFG_UART_FREQ 1843200
#define CFG_UART_TX 16
#define CFG_UART_RX 17

// --- SIM ---
#define CFG_SIM_BRIDGE_PORT 12345
#define CFG_SIM_RADIO_PORT 12346
#define CFG_SIM_SERIAL_PORT 12347

#endif