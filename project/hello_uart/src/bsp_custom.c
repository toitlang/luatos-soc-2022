// Copyright (C) 2026 Toit contributors.
//
// Board-specific initialization for the Toit project on EC618.

#include <stdio.h>
#include <string.h>

#include "bsp.h"
#include "bsp_custom.h"
#include "clock.h"
#include "slpman.h"

#include "Driver_USART.h"

extern ARM_DRIVER_USART Driver_USART1;

// Newlib _write syscall: bridges printf -> io_putchar -> UART SendPolling.
extern int io_putchar(int ch);

int _write(int file, char *ptr, int len) {
    (void)file;
    for (int i = 0; i < len; i++) {
        io_putchar(*ptr++);
    }
    return len;
}

static void SetPrintUart(void) {
    GPR_setClockSrc(FCLK_UART1, FCLK_UART1_SEL_26M);
    GPR_clockEnable(FCLK_UART1);
    GPR_swReset(RST_FCLK_UART1);

    Driver_USART1.Initialize(NULL);
    Driver_USART1.PowerControl(ARM_POWER_FULL);
    Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
                          ARM_USART_DATA_BITS_8 |
                          ARM_USART_PARITY_NONE |
                          ARM_USART_STOP_BITS_1 |
                          ARM_USART_FLOW_CONTROL_NONE,
                          921600);
    Driver_USART1.Control(ARM_USART_CONTROL_TX, 1);

    UsartPrintHandle = &Driver_USART1;
}

void BSP_CustomInit(void) {
    slpManAonWdtStop();
    SetPrintUart();
    setvbuf(stdout, NULL, _IONBF, 0);

    // Test: write directly via SendPolling (synchronous).
    const char *msg = "[toit] BSP_CustomInit reached\r\n";
    UsartPrintHandle->SendPolling((const uint8_t*)msg, 31);
}
