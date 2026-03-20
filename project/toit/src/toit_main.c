// Copyright (C) 2026 Toit contributors.
//
// Entry point for the Toit runtime on EC618. This file is compiled by
// the PLAT SDK's build system and uses the INIT_TASK_EXPORT macro to
// register the Toit startup task during system boot.

#include "common_api.h"
#include "luat_rtos.h"

// Defined in src/toit_ec618.cc (the Toit VM library).
extern void toit_start(void);

static luat_rtos_task_handle toit_task_handle;

static void toit_task(void *param) {
  toit_start();
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
