// Copyright (C) 2026 Toit contributors.
//
// Entry point for the Toit runtime on EC618. Reads the active-slot byte
// from .slot_marker, then tail-calls through the slot's .vm_entry
// pointer. Each VM slot's first word is a function pointer to its own
// toit_start; that decoupling is what makes dual-linked A/B slots work
// without a fixed-offset entry symbol inside the slot.

#include <stdint.h>
#include "common_api.h"
#include "luat_rtos.h"

// Slot-marker byte at fixed flash address (see ec618_0h00_flash.c). A
// fresh build initialises it to 'A'; the OTA path erases the marker
// sector and writes 'B' to switch slots on next boot. Erased flash
// reads 0xFF, which we treat as "default slot A".
__attribute__((section(".slot_marker"), used))
const volatile uint8_t toit_active_slot = 'A';

// Linker-script symbols marking the slot base addresses. Declared as
// arrays so referring to them yields their address (the slot's first
// flash word), not the bytes at that address.
extern uint32_t __vm_a_start[];
extern uint32_t __vm_b_start[];

typedef void (*toit_start_fn)(void);

static luat_rtos_task_handle toit_task_handle;

static void toit_task(void *param) {
  uint8_t slot = toit_active_slot;
  const uint32_t *slot_base;
  if (slot == 'B') {
    slot_base = __vm_b_start;
    printf("[toit] INFO: booting VM slot B\n");
  } else {
    slot_base = __vm_a_start;
    printf("[toit] INFO: booting VM slot A\n");
  }
  // The slot's first word is a function pointer (.vm_entry, written by
  // the VM build). The Thumb bit is already set in the linker
  // relocation, so a plain indirect call lands in toit_start.
  toit_start_fn entry = (toit_start_fn)slot_base[0];
  entry();
  // toit_start() does not return in normal operation (enters deep sleep).
  // If it does return, halt.
  while (1) {
    luat_rtos_task_sleep(10000);
  }
}

static void toit_task_init(void) {
  // 8KB stack for the Toit main task.
  luat_rtos_task_create(&toit_task_handle, 8 * 1024, 20, "toit", toit_task, NULL, 0);
}

// Register at task init level 1 (runs after hardware and driver init).
INIT_TASK_EXPORT(toit_task_init, "1");
