// Copyright (C) 2026 Toit contributors.
//
// Simple hello world that prints on UART1 for testing.

#include <stdio.h>

#include "common_api.h"
#include "luat_rtos.h"

static luat_rtos_task_handle hello_task_handle;

static void hello_task(void *param) {
  while (1) {
    printf("Hello world\r\n");
    luat_rtos_task_sleep(1000);
  }
}

static void hello_task_init(void) {
  luat_rtos_task_create(&hello_task_handle, 4096, 20, "hello", hello_task, NULL, 0);
}

INIT_TASK_EXPORT(hello_task_init, "1");
