// Copyright (C) 2026 Toit contributors.
//
// Override sysROSpaceCheck from libstartup.a to allow Toit to write
// to the flash registry region, which falls inside the AP image area
// when __USER_CODE__ is defined (AP_FLASH_LOAD_SIZE = 0x2E0000).
//
// The linker resolves object-file symbols before archive symbols, so
// this definition takes precedence over the one in libstartup.a.

#include <stdint.h>

// Writable window for flash operations. Set these before performing
// flash writes to regions inside the AP image area (flash registry, OTA).
uint32_t toit_ap_image_modify_start = 0;
uint32_t toit_ap_image_modify_end   = 0;

#define BOOTLOADER_END  0x22000
#define AP_IMAGE_START  0x24000
#define AP_IMAGE_END    0x304000  // 0x24000 + 0x2E0000

static uint8_t sysROAddrCheck(uint32_t addr) {
    if (addr < BOOTLOADER_END) {
        return 1;  // Bootloader — always read-only.
    }
    if (addr >= AP_IMAGE_START && addr < AP_IMAGE_END) {
        // Allow if inside the Toit-designated writable window.
        if (toit_ap_image_modify_start <= addr
            && addr < toit_ap_image_modify_end) {
            return 0;
        }
        return 1;  // AP image — read-only by default.
    }
    return 0;  // Everything else is writable.
}

uint8_t sysROSpaceCheck(uint32_t addr, uint32_t size) {
    if (sysROAddrCheck(addr))            return 1;
    if (size > 0 && sysROAddrCheck(addr + size - 1)) return 1;
    return 0;
}
