// Copyright (C) 2026 Toit contributors.
//
// Board-specific initialization for the Toit project on EC618.
// This overrides the default BSP_CustomInit from libcore_airm2m.a.

#include "common_api.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "plat_config.h"
#include "slpman.h"

// Override the default BSP_CustomInit to set up UART1 as the print port.
// This is called early in the boot process by the PLAT startup code.
void BSP_CustomInit(void) {
    // Disable the sleep manager watchdog — without this the device
    // reboots after ~27 seconds.
    slpManAonWdtStop();

    // Set UART1 as the debug print output port.
    SetUnilogUart(PORT_USART_1, 921600, false);

    // Route log output to USB CDC serial.
    BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_PORT_SEL, PLAT_CFG_ULG_PORT_USB);
}
