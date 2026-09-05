#ifndef _HW_INFO_H_
#define _HW_INFO_H_

#ifdef __cplusplus
extern "C"
{
#endif

// --- SIM ---
#define CFG_SIM_BRIDGE_PORT 12345
#define CFG_SIM_UART_PORT 12346
#define CFG_SIM_SERIAL_PORT 12347

// --- INIT FUNCTION ---
void hw_init(void);

#ifdef __cplusplus
}
#endif

#endif