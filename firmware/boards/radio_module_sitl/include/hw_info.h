#ifndef _HW_INFO_H_
#define _HW_INFO_H_

#ifdef __cplusplus
extern "C"
{
#endif

// --- SIM ---
#define CFG_SIM_UART_SERVER "127.0.0.1"
#define CFG_SIM_UART_PORT 12346
#define CFG_SIM_LORA_SERVER "127.0.0.1"
#define CFG_SIM_LORA_PORT 12348

// --- INIT FUNCTION ---
void hw_init(void);

#ifdef __cplusplus
}
#endif

#endif