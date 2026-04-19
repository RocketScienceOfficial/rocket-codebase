#ifndef _MS56XX_DRIVER_REGS_H
#define _MS56XX_DRIVER_REGS_H

/* --- I2C Slave Addresses --- */
/* Address is 111011Cx, where C is the complement of the CSB pin */
#define MS56XX_I2C_ADDR_CSB_HIGH 0x76 /* 1110110b */
#define MS56XX_I2C_ADDR_CSB_LOW 0x77  /* 1110111b */

/* --- Basic Commands --- */
#define MS56XX_CMD_RESET 0x1E
#define MS56XX_CMD_ADC_READ 0x00

/* --- Conversion Commands (D1 = Pressure, D2 = Temperature) --- */
#define MS56XX_CMD_CONVERT_D1_OSR_256 0x40
#define MS56XX_CMD_CONVERT_D1_OSR_512 0x42
#define MS56XX_CMD_CONVERT_D1_OSR_1024 0x44
#define MS56XX_CMD_CONVERT_D1_OSR_2048 0x46
#define MS56XX_CMD_CONVERT_D1_OSR_4096 0x48

#define MS56XX_CMD_CONVERT_D2_OSR_256 0x50
#define MS56XX_CMD_CONVERT_D2_OSR_512 0x52
#define MS56XX_CMD_CONVERT_D2_OSR_1024 0x54
#define MS56XX_CMD_CONVERT_D2_OSR_2048 0x56
#define MS56XX_CMD_CONVERT_D2_OSR_4096 0x58

/* --- PROM Read Commands --- */
/* The 128-bit PROM is accessed via 8 addresses (16 bits each) */
#define MS56XX_CMD_PROM_READ_BASE 0xA0

#define MS56XX_PROM_ADDR_0_MANUFACT 0xA0
#define MS56XX_PROM_ADDR_1_C1 0xA2
#define MS56XX_PROM_ADDR_2_C2 0xA4
#define MS56XX_PROM_ADDR_3_C3 0xA6
#define MS56XX_PROM_ADDR_4_C4 0xA8
#define MS56XX_PROM_ADDR_5_C5 0xAA
#define MS56XX_PROM_ADDR_6_C6 0xAC
#define MS56XX_PROM_ADDR_7_CRC 0xAE

/* --- Timeouts (Microseconds) --- */
#define MS56XX_TIMEOUT_RESET_US 2800    /* 2.80 ms memory reload after reset */
#define MS56XX_TIMEOUT_OSR_256_US 600   /* 0.60 ms max */
#define MS56XX_TIMEOUT_OSR_512_US 1170  /* 1.17 ms max */
#define MS56XX_TIMEOUT_OSR_1024_US 2280 /* 2.28 ms max */
#define MS56XX_TIMEOUT_OSR_2048_US 4540 /* 4.54 ms max */
#define MS56XX_TIMEOUT_OSR_4096_US 9040 /* 9.04 ms max */

#endif