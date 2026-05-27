
#include "mem_map.h"

/* Entry Point */
ENTRY(Reset_Handler)

/* Specify the memory areas */
MEMORY
{
  ASMB_AREA(rwx)              : ORIGIN = 0x00000000, LENGTH = 0x010000      /* 64KB */
  MSMB_AREA(rwx)              : ORIGIN = 0x00400000, LENGTH = 0x140000      /* 1.25MB */
#ifdef __LUATOS__
#if defined(FLASH_AREA_SIZE)
  FLASH_AREA(rx)              : ORIGIN = 0x00824000, LENGTH = FLASH_AREA_SIZE
#else
  FLASH_AREA(rx)              : ORIGIN = 0x00824000, LENGTH = 2212K         /* 2212K */
#endif
#else
  FLASH_AREA(rx)              : ORIGIN = 0x00824000, LENGTH = 2944K         /* 2944K */
#endif
}

/* Define output sections */
SECTIONS
{
  . = AP_FLASH_LOAD_ADDR;
  .vector :
  {
    KEEP(*(.isr_vector))
  } >FLASH_AREA
  .cache : ALIGN(128)
  {
    Image$$UNLOAD_NOCACHE$$Base = .;
    *libdriver*.a:cache.o(.text*) 
  } >FLASH_AREA
  
  .load_bootcode 0x0 :
  {
    . = ALIGN(4);
    Load$$LOAD_BOOTCODE$$Base = LOADADDR(.load_bootcode);
    Image$$LOAD_BOOTCODE$$Base = .;
    KEEP(*(.mcuVector))
    *(.ramBootCode)
    *libdriver*.a:qspi.o(.text*)
    *libdriver*.a:flash.o(.text*)
    . = ALIGN(4);
  } >ASMB_AREA AT>FLASH_AREA

    Image$$LOAD_BOOTCODE$$Length = SIZEOF(.load_bootcode);

  .load_ap_piram_asmb : ALIGN(4)
  {
   . = ALIGN(4);
   Load$$LOAD_AP_PIRAM_ASMB$$Base = LOADADDR(.load_ap_piram_asmb);
   Image$$LOAD_AP_PIRAM_ASMB$$Base = .;
   *(.psPARamcode)
   *(.platPARamcode)
   *libc*.a:*memset.o(.text*)
   *memcpy-armv7m.o(.text*)
   . = ALIGN(4);
  } >ASMB_AREA AT>FLASH_AREA

  Image$$LOAD_AP_PIRAM_ASMB$$Length = SIZEOF(.load_ap_piram_asmb);

  .load_ap_firam_asmb : ALIGN(4)
  {
   . = ALIGN(4);
   Load$$LOAD_AP_FIRAM_ASMB$$Base = LOADADDR(.load_ap_firam_asmb);
   Image$$LOAD_AP_FIRAM_ASMB$$Base = .;
   *(.psFARamcode)
   *(.platFARamcode)
   . = ALIGN(4);
  } >ASMB_AREA AT>FLASH_AREA

  Image$$LOAD_AP_FIRAM_ASMB$$Length = SIZEOF(.load_ap_firam_asmb);

  .load_apos : ALIGN(4)
  {
    . = ALIGN(4);
    Load$$LOAD_APOS$$Base = LOADADDR(.load_apos);
    Image$$LOAD_APOS$$Base = .;
    *libfreertos.a:event_groups.o(.text*)
    *libfreertos.a:heap_6.o(.text*)
    *libfreertos.a:tlsf.o(.text*)
    *libfreertos.a:list.o(.text*)
    *libfreertos.a:queue.o(.text*)
    *libfreertos.a:tasks.o(.text*)
    . = ALIGN(4);
  } >ASMB_AREA AT>FLASH_AREA

  Image$$LOAD_APOS$$Length = SIZEOF(.load_apos);

  .load_ap_rwdata_asmb : ALIGN(4)
  {
   . = ALIGN(4);
   Load$$LOAD_AP_FDATA_ASMB$$RW$$Base = LOADADDR(.load_ap_rwdata_asmb);
   Image$$LOAD_AP_FDATA_ASMB$$RW$$Base = .;
   *(.platFARWData)
   . = ALIGN(4);
  } >ASMB_AREA AT>FLASH_AREA
  Image$$LOAD_AP_FDATA_ASMB$$Length = SIZEOF(.load_ap_rwdata_asmb);
  
  .load_ps_rwdata_asmb : ALIGN(4)
  {
    Load$$LOAD_PS_FDATA_ASMB$$RW$$Base = LOADADDR(.load_ps_rwdata_asmb);
    Image$$LOAD_PS_FDATA_ASMB$$RW$$Base = .;
    *(.psFARWData)
    . = ALIGN(4);
  } >ASMB_AREA AT>FLASH_AREA
  Image$$LOAD_PS_FDATA_ASMB$$RW$$Length = SIZEOF(.load_ps_rwdata_asmb);
  
  .load_ap_zidata_asmb (NOLOAD):
  {
   . = ALIGN(4);
   Image$$LOAD_AP_FDATA_ASMB$$ZI$$Base = .;
   *(.platFAZIData)
   . = ALIGN(4);
   Image$$LOAD_AP_FDATA_ASMB$$ZI$$Limit = .;
   
   Image$$LOAD_PS_FDATA_ASMB$$ZI$$Base = .;
   *(.psFAZIData)
   . = ALIGN(4);
   Image$$LOAD_PS_FDATA_ASMB$$ZI$$Limit = .;
   *(.platPANoInit)
   *(.psFANoInitData)
   *(.exceptCheck)
  } >ASMB_AREA

  /* Toit RTC memory: placed after the ZI limit markers in ASMB.
     NOTE: On EC618, ASMB content does NOT survive SLP2 or HIBERNATE
     despite being in a (NOLOAD) section. The SLP2 save/restore
     mechanism corrupts this region. RTC persistence requires flash. */
  .toit_rtc_noinit (NOLOAD):
  {
    . = ALIGN(4);
    *(.toit.rtc.noinit)
    . = ALIGN(4);
  } >ASMB_AREA

  .unload_cpaon CP_AONMEMBACKUP_START_ADDR (NOLOAD):
  {

  } >ASMB_AREA

  .load_rrcmem 0xB000 (NOLOAD):
  {
    *(.rrcMem)
  } >ASMB_AREA

  .load_flashmem 0xC000 (NOLOAD):
  {
    *(.apFlashMem)
  } >ASMB_AREA

  .load_ap_piram_msmb MSMB_START_ADDR :
  {
    . = ALIGN(4);
    Load$$LOAD_AP_PIRAM_MSMB$$Base = LOADADDR(.load_ap_piram_msmb);
    Image$$LOAD_AP_PIRAM_MSMB$$Base = .;
    *(.psPMRamcode)
    *(.platPMRamcode)
    *(.platPMRamcodeFCLK)
    *(.recordNodeRO)
    . = ALIGN(4);
  } >MSMB_AREA AT>FLASH_AREA

  Image$$LOAD_AP_PIRAM_MSMB$$Length = SIZEOF(.load_ap_piram_msmb);

  .load_ap_firam_msmb : ALIGN(4)
  {
    . = ALIGN(4);
    Load$$LOAD_AP_FIRAM_MSMB$$Base = LOADADDR(.load_ap_firam_msmb);
    Image$$LOAD_AP_FIRAM_MSMB$$Base = .;
    *(.ramCode2)
    *(.upRamCode)
    *(.psFMRamcode)
    *(.platFMRamcode)
    *libfreertos.a:port_asm.o(.text*)
    *libfreertos.a:port.o(.text*)
    *libfreertos.a:timers.o(.text*)
    *libfreertos.a:cmsis_os2.o(.text*)
    . = ALIGN(4);
  } >MSMB_AREA AT>FLASH_AREA

  Image$$LOAD_AP_FIRAM_MSMB$$Length = SIZEOF(.load_ap_firam_msmb);

  .load_dram_bsp : ALIGN(4)
  {
    . = ALIGN(4);
    Load$$LOAD_DRAM_BSP$$Base = LOADADDR(.load_dram_bsp);
    Image$$LOAD_DRAM_BSP$$Base = .;
    *libdriver*:bsp_spi.o(.data*)
    *libdriver*:flash.o(.data*)
    *libdriver*:flash_rt.o(.data*)
    *libdriver*:gpr.o(.data*)
    *libdriver*:apmu.o(.data*)
    *libdriver*:apmuTiming.o(.data*)
    *libdriver*:bsp.o(.data*)
    *libdriver*:plat_config.o(.data*)
    *libstartup*:system_ec618.o(.data*)
    *libdriver*:unilog.o(.data*)
    *libdriver*:pad.o(.data*)
    *libdriver*:ic.o(.data*)
    *libdriver*:ec_main.o(.data*)
    *libdriver*:slpman.o(.data*)
    *libdriver*:bsp_usart.o(.data*)
    *libdriver*:bsp_lpusart.o(.data*)
    *libdriver*:timer.o(.data*)
    *libdriver*:dma.o(.data*)
    *libdriver*:adc.o(.data*)
    *libdriver*:wdt.o(.data*)
    *libmiddleware_ec*:usb_device.o(.data*)
    *libmiddleware_ec*:uart_device.o(.data*)
    *libdriver*:clock.o(.data*)
    *libdriver*:hal_adc.o(.data*)
    *libdriver*:hal_adcproxy.o(.data*)
    *libdriver*:hal_alarm.o(.data*)
    *libdriver*:exception_process.o(.data*)
    *libdriver*:exception_dump.o(.data*)
    . = ALIGN(4);
  } >MSMB_AREA AT>FLASH_AREA

  Image$$LOAD_DRAM_BSP$$Length = SIZEOF(.load_dram_bsp);

  .load_dram_bsp_zi (NOLOAD):
  {
    . = ALIGN(4);
    Image$$LOAD_DRAM_BSP$$ZI$$Base = .;
    *libdriver*:bsp_spi.o(.bss*)
    *libdriver*:flash.o(.bss*)
    *libdriver*:flash_rt.o(.bss*)
    *libdriver*:gpr.o(.bss*)
    *libdriver*:apmu.o(.bss*)
    *libdriver*:apmuTiming.o(.bss*)
    *libdriver*:bsp.o(.bss*)
    *libdriver*:plat_config.o(.bss*)
    *libstartup*:system_ec618.o(.bss*)
    *libdriver*:unilog.o(.bss*)
    *libdriver*:pad.o(.bss*)
    *libdriver*:ic.o(.bss*)
    *libdriver*:ec_main.o(.bss*)
    *libdriver*:slpman.o(.bss*)
    *libdriver*:bsp_usart.o(.bss*)
    *libdriver*:bsp_lpusart.o(.bss*)
    *libdriver*:timer.o(.bss*)
    *libdriver*:dma.o(.bss*)
    *libdriver*:adc.o(.bss*)
    *libdriver*:wdt.o(.bss*)
    *libmiddleware_ec*:usb_device.o(.bss*)
    *libmiddleware_ec*:uart_device.o(.bss*)
    *libdriver*:clock.o(.bss*)
    *libdriver*:hal_adc.o(.bss*)
    *libdriver*:hal_trim.o(.bss*)
    *libdriver*:hal_adcproxy.o(.bss*)
    *libdriver*:hal_alarm.o(.bss*)
    *libdriver*:exception_process.o(.bss*)
    *libdriver*:exception_dump.o(.bss*)
    *(.recordNodeZI)
    . = ALIGN(4);
  Image$$LOAD_DRAM_BSP$$ZI$$Limit = .;
  } >MSMB_AREA

  .unload_slpmem (NOLOAD):
  {
    *(.sleepmem)
  } >MSMB_AREA

  .load_dram_shared : ALIGN(4)
  {
    . = ALIGN(4);
    Load$$LOAD_DRAM_SHARED$$Base = LOADADDR(.load_dram_shared);
    Image$$LOAD_DRAM_SHARED$$Base = .;
    *(.data*)
    /* C++ static initializers — PLAT-side only here. VM constructors are
     * captured into the active slot and run by run_static_initializers()
     * in src/toit_ec618.cc against __vm_init_array_start/__vm_init_array_end. */
    . = ALIGN(4);
    __init_array_start = .;
    KEEP (EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(SORT(.init_array.*)))
    KEEP (EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(.init_array*))
    __init_array_end = .;
    . = ALIGN(4);
  } >MSMB_AREA AT>FLASH_AREA

  Image$$LOAD_DRAM_SHARED$$Length = SIZEOF(.load_dram_shared);

  .load_dram_shared_zi (NOLOAD):
  {
    . = ALIGN(4);
    Image$$LOAD_DRAM_SHARED$$ZI$$Base = .;
    *(.platBlSctZIData)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    *(.stack)               /* stack should be 4 byte align */
    Image$$LOAD_DRAM_SHARED$$ZI$$Limit = .;
    *(.USB_NOINIT_DATA_BUF)
  } >MSMB_AREA


  PROVIDE(end_ap_data = . );
  PROVIDE(start_up_buffer = up_buf_start);
  .load_up_buffer start_up_buffer(NOLOAD):
  {
    *(.catShareBuf)
    Image$$LOAD_UP_BUFFER$$Limit = .;
  } >MSMB_AREA

  PROVIDE(end_up_buffer = . );
  heap_size = start_up_buffer - end_ap_data;
  ASSERT(heap_size>=min_heap_size_threshold,"ap use too much ram, heap less than min_heap_size_threshold!")
  ASSERT(end_up_buffer<=MSMB_APMEM_END_ADDR,"ap use too much ram, exceed to MSMB_APMEM_END_ADDR")

  /*
   * Dual-slot OTA layout (Toit fork).
   *
   *   0x848000-0x990000 : PLAT .text  (~1.27 MB used, ~50 KB headroom)
   *   0x990000-0x991000 : .jt_data    (4 KB, fixed addr for jump table; lives inside PLAT objects)
   *   0x991000-0x9F1000 : .vm_a       (384 KB, slot A)
   *   0x9F1000-0xA51000 : .vm_b       (384 KB, slot B, reserved)
   *   0xA51000-0xA52000 : .slot_marker (4 KB, one sector — active-slot byte)
   *   0xA52000-0xAA4000 : extension   (~328 KB)
   *
   * The VM (libtoit_vm.a + mbedtls) and the .vm_entry pointer are picked
   * into .vm_a (default build) or .vm_b (slot-B build, selected with
   * -DTOIT_VM_SLOT_B). PLAT objects fall through into .text. The two
   * link passes are spliced together at the binary level to produce a
   * single image with both slots populated.
   */
#define TOIT_VM_A_ORIGIN  0x00991000
#define TOIT_VM_B_ORIGIN  0x009F1000
#define TOIT_VM_SLOT_SIZE 0x00060000
#define TOIT_JT_ORIGIN    0x00990000
#define TOIT_JT_SIZE      0x00001000
#define TOIT_SLOT_MARKER_ORIGIN 0x00A51000
#define TOIT_SLOT_MARKER_SIZE   0x00001000
#define TOIT_PLAT_TEXT_LIMIT TOIT_JT_ORIGIN

  .text :
  {
    EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(.rodata*)
    EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.vfpll_veneer)
    *(.v4_bx)
    *(.init*)
    *(.fini*)
    *(.iplt)
    *(.igot.plt)
    *(.rel.iplt)
  } >FLASH_AREA

  .preinit_fun_array :
  {
      . = ALIGN(4);
      __preinit_fun_array_start = .;
      KEEP (*(SORT(.preinit_fun_array.*)))
      KEEP (*(.preinit_fun_array*))
      __preinit_fun_array_end = .;
      . = ALIGN(4);
  } > FLASH_AREA
  .drv_init_fun_array :
  {
      . = ALIGN(4);
      __drv_init_fun_array_start = .;
      KEEP (*(SORT(.drv_init_fun_array.*)))
      KEEP (*(.drv_init_fun_array*))
      __drv_init_fun_array_end = .;
      . = ALIGN(4);
  } > FLASH_AREA

  .task_fun_array :
  {
      . = ALIGN(4);
      __task_fun_array_start = .;
      KEEP (*(SORT(.task_fun_array.*)))
      KEEP (*(.task_fun_array*))
      __task_fun_array_end = .;
      . = ALIGN(4);
  } > FLASH_AREA

  ASSERT(. <= TOIT_PLAT_TEXT_LIMIT,
         "PLAT region overflowed into VM slot A; reduce PLAT or move TOIT_VM_A_ORIGIN.")

  /* Jump-table data at fixed flash address. plat_jt.o annotates the
   * g_plat_jt[] array with __attribute__((section(".jt_data"))). The
   * table lives here (inside the PLAT region textually, but the address
   * is fixed so both VM_A and VM_B see g_plat_jt[] at the same XIP
   * address regardless of which link pass produced them).
   *
   * Each fixed-address section uses an explicit `addr :` prefix so the
   * linker plants it at the configured flash offset; the bare `>FLASH_AREA`
   * form keys off FLASH_AREA's running location counter, which would
   * stack the slots back-to-back at the end of PLAT.
   */
  .jt_data TOIT_JT_ORIGIN :
  {
    __jt_data_start = .;
    KEEP(*(.jt_data))
    __jt_data_end = .;
  } >FLASH_AREA

  ASSERT(__jt_data_end - __jt_data_start <= TOIT_JT_SIZE,
         "Jump-table section .jt_data exceeded TOIT_JT_SIZE")

  .vm_a TOIT_VM_A_ORIGIN :
  {
    __vm_a_start = .;
#ifndef TOIT_VM_SLOT_B
    KEEP(*(.vm_entry))
    /* VM-side C++ static initializers live inside the slot so each slot
     * is self-contained. run_static_initializers() in src/toit_ec618.cc
     * iterates __vm_init_array_*. Writable VM .data still lives in
     * .load_dram_shared (PLAT loads it from flash to RAM at startup);
     * .data values are slot-agnostic for the current VM where const
     * tables are in .rodata and writable globals don't bake in
     * slot-specific pointers. */
    . = ALIGN(4);
    __vm_init_array_start = .;
    KEEP(*libtoit_vm.a:*(SORT(.init_array.*)))
    KEEP(*libtoit_vm.a:*(.init_array*))
    KEEP(*libmbedtls.a:*(SORT(.init_array.*)))
    KEEP(*libmbedtls.a:*(.init_array*))
    KEEP(*libmbedx509.a:*(SORT(.init_array.*)))
    KEEP(*libmbedx509.a:*(.init_array*))
    KEEP(*libmbedcrypto.a:*(SORT(.init_array.*)))
    KEEP(*libmbedcrypto.a:*(.init_array*))
    __vm_init_array_end = .;
    *libtoit_vm.a:*(.rodata*)
    *libtoit_vm.a:*(.text*)
    *libmbedtls.a:*(.rodata*)
    *libmbedtls.a:*(.text*)
    *libmbedx509.a:*(.rodata*)
    *libmbedx509.a:*(.text*)
    *libmbedcrypto.a:*(.rodata*)
    *libmbedcrypto.a:*(.text*)
#endif
    __vm_a_end = .;
  } >FLASH_AREA

#ifndef TOIT_VM_SLOT_B
  ASSERT(__vm_a_end - __vm_a_start <= TOIT_VM_SLOT_SIZE,
         "VM slot A overflowed TOIT_VM_SLOT_SIZE")
#endif

  .vm_b TOIT_VM_B_ORIGIN :
  {
    __vm_b_start = .;
#ifdef TOIT_VM_SLOT_B
    KEEP(*(.vm_entry))
    . = ALIGN(4);
    __vm_init_array_start = .;
    KEEP(*libtoit_vm.a:*(SORT(.init_array.*)))
    KEEP(*libtoit_vm.a:*(.init_array*))
    KEEP(*libmbedtls.a:*(SORT(.init_array.*)))
    KEEP(*libmbedtls.a:*(.init_array*))
    KEEP(*libmbedx509.a:*(SORT(.init_array.*)))
    KEEP(*libmbedx509.a:*(.init_array*))
    KEEP(*libmbedcrypto.a:*(SORT(.init_array.*)))
    KEEP(*libmbedcrypto.a:*(.init_array*))
    __vm_init_array_end = .;
    *libtoit_vm.a:*(.rodata*)
    *libtoit_vm.a:*(.text*)
    *libmbedtls.a:*(.rodata*)
    *libmbedtls.a:*(.text*)
    *libmbedx509.a:*(.rodata*)
    *libmbedx509.a:*(.text*)
    *libmbedcrypto.a:*(.rodata*)
    *libmbedcrypto.a:*(.text*)
#endif
    __vm_b_end = .;
  } >FLASH_AREA

#ifdef TOIT_VM_SLOT_B
  ASSERT(__vm_b_end - __vm_b_start <= TOIT_VM_SLOT_SIZE,
         "VM slot B overflowed TOIT_VM_SLOT_SIZE")
#endif

  .slot_marker TOIT_SLOT_MARKER_ORIGIN :
  {
    __slot_marker_start = .;
    KEEP(*(.slot_marker))
    __slot_marker_end = .;
  } >FLASH_AREA

  PROVIDE(totalFlashLimit = .);

  .load_xp_sharedinfo XP_SHAREINFO_BASE_ADDR (NOLOAD):
  {
  *(.shareInfo)
  } >MSMB_AREA
  
  .load_dbg_area XP_DBGRESERVED_BASE_ADDR (NOLOAD):
  {
  *(.resetFlag)
  } >MSMB_AREA
  
  .unload_xp_ipcmem IPC_SHAREDMEM_START_ADDR (NOLOAD):
  {

  } >MSMB_AREA

}

GROUP(
    libgcc.a
    libc.a
    libm.a
 )