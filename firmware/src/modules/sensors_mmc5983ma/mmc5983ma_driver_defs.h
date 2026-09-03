#ifndef _MMC5983MA_DRIVER_DEFS_H
#define _MMC5983MA_DRIVER_DEFS_H

/* --- Device Identification & I2C --- */
#define MMC5983MA_I2C_ADDR 0x30 /* 0110000b */
#define MMC5983MA_PRODUCT_ID 0x30
#define MMC5983MA_READ_MASK 0x80
#define MMC5983MA_WRITE_MASK 0x7F

/* --- Register Map --- */
#define MMC5983MA_REG_XOUT0 0x00    /* Xout [17:10] */
#define MMC5983MA_REG_XOUT1 0x01    /* Xout [9:2] */
#define MMC5983MA_REG_YOUT0 0x02    /* Yout [17:10] */
#define MMC5983MA_REG_YOUT1 0x03    /* Yout [9:2] */
#define MMC5983MA_REG_ZOUT0 0x04    /* Zout [17:10] */
#define MMC5983MA_REG_ZOUT1 0x05    /* Zout [9:2] */
#define MMC5983MA_REG_XYZOUT2 0x06  /* Xout[1:0], Yout[1:0], Zout[1:0] */
#define MMC5983MA_REG_TOUT 0x07     /* Temperature output */
#define MMC5983MA_REG_STATUS 0x08   /* Device status */
#define MMC5983MA_REG_CTRL0 0x09    /* Internal control 0 */
#define MMC5983MA_REG_CTRL1 0x0A    /* Internal control 1 */
#define MMC5983MA_REG_CTRL2 0x0B    /* Internal control 2 */
#define MMC5983MA_REG_CTRL3 0x0C    /* Internal control 3 */
#define MMC5983MA_REG_PROD_ID1 0x2F /* Product ID */

/* --- XYZ Data Assembly Shifts --- */
#define MMC5983MA_OUT0_SHIFT 10
#define MMC5983MA_OUT1_SHIFT 2
#define MMC5983MA_XYZOUT2_X_MASK 0xC0
#define MMC5983MA_XYZOUT2_X_SHIFT 6
#define MMC5983MA_XYZOUT2_Y_MASK 0x30
#define MMC5983MA_XYZOUT2_Y_SHIFT 4
#define MMC5983MA_XYZOUT2_Z_MASK 0x0C
#define MMC5983MA_XYZOUT2_Z_SHIFT 2

/* --- STATUS_REG (08h) Bit Fields --- */
#define MMC5983MA_STATUS_OTP_RD_DONE (1 << 6)
#define MMC5983MA_STATUS_MEAS_T_DONE (1 << 1)
#define MMC5983MA_STATUS_MEAS_M_DONE (1 << 0)

/* --- CTRL_REG0 (09h) Bit Fields --- */
#define MMC5983MA_CTRL0_OTP_READ (1 << 6)
#define MMC5983MA_CTRL0_AUTO_SR_EN (1 << 5)
#define MMC5983MA_CTRL0_RESET (1 << 4)
#define MMC5983MA_CTRL0_SET (1 << 3)
#define MMC5983MA_CTRL0_INT_MEAS_DONE (1 << 2)
#define MMC5983MA_CTRL0_TM_T (1 << 1)
#define MMC5983MA_CTRL0_TM_M (1 << 0)

/* --- CTRL_REG1 (0Ah) Bit Fields --- */
#define MMC5983MA_CTRL1_SW_RST (1 << 7)
#define MMC5983MA_CTRL1_X_INHIBIT (1 << 3)
#define MMC5983MA_CTRL1_YZ_INHIBIT (1 << 2)
#define MMC5983MA_CTRL1_BW_MASK (0x03 << 0) /* Bits 1:0 */

/* Bandwidth / Decimation Filter Selection (BW1-BW0) */
#define MMC5983MA_CTRL1_BW_100HZ (0x00 << 0) /* 8ms measurement time */
#define MMC5983MA_CTRL1_BW_200HZ (0x01 << 0) /* 4ms measurement time */
#define MMC5983MA_CTRL1_BW_400HZ (0x02 << 0) /* 2ms measurement time */
#define MMC5983MA_CTRL1_BW_800HZ (0x03 << 0) /* 0.5ms measurement time */

/* --- CTRL_REG2 (0Bh) Bit Fields --- */
#define MMC5983MA_CTRL2_EN_PRD_SET (1 << 7)
#define MMC5983MA_CTRL2_PRD_SET_MASK (0x07 << 4) /* Bits 6:4 */
#define MMC5983MA_CTRL2_CMM_EN (1 << 3)
#define MMC5983MA_CTRL2_CM_FREQ_MASK (0x07 << 0) /* Bits 2:0 */

/* Continuous Measurement Mode Frequency (CM_Freq2-CM_Freq0) */
#define MMC5983MA_CTRL2_CM_FREQ_OFF (0x00 << 0)
#define MMC5983MA_CTRL2_CM_FREQ_1HZ (0x01 << 0)
#define MMC5983MA_CTRL2_CM_FREQ_10HZ (0x02 << 0)
#define MMC5983MA_CTRL2_CM_FREQ_20HZ (0x03 << 0)
#define MMC5983MA_CTRL2_CM_FREQ_50HZ (0x04 << 0)
#define MMC5983MA_CTRL2_CM_FREQ_100HZ (0x05 << 0)
#define MMC5983MA_CTRL2_CM_FREQ_200HZ (0x06 << 0)
#define MMC5983MA_CTRL2_CM_FREQ_1000HZ (0x07 << 0)

/* Periodic Set Rate (Prd_set2-Prd_set0) - Number of measurements between SETs */
#define MMC5983MA_CTRL2_PRD_SET_1 (0x00 << 4)
#define MMC5983MA_CTRL2_PRD_SET_25 (0x01 << 4)
#define MMC5983MA_CTRL2_PRD_SET_75 (0x02 << 4)
#define MMC5983MA_CTRL2_PRD_SET_100 (0x03 << 4)
#define MMC5983MA_CTRL2_PRD_SET_250 (0x04 << 4)
#define MMC5983MA_CTRL2_PRD_SET_500 (0x05 << 4)
#define MMC5983MA_CTRL2_PRD_SET_1000 (0x06 << 4)
#define MMC5983MA_CTRL2_PRD_SET_2000 (0x07 << 4)

/* --- CTRL_REG3 (0Ch) Bit Fields --- */
#define MMC5983MA_CTRL3_SPI_3W (1 << 6)
#define MMC5983MA_CTRL3_ST_ENM (1 << 2) /* Apply extra negative current for self-test */
#define MMC5983MA_CTRL3_ST_ENP (1 << 1) /* Apply extra positive current for self-test */

#endif