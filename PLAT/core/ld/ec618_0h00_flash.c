
#include "mem_map.h"

/* Fixed-size pooled reservation for the VM's contribution to shared RAM
 * (the .vm_dram_data / .vm_dram_zi sections in the dram map). The heap
 * start (end_ap_data) is pinned at the reserve's END, so VM .data/.bss
 * growth within the reserve is INVISIBLE to the base — the structural half
 * of the frozen-base contract (docs/frozen-base-design.md phase 3). Actual
 * use (2026-07: data ~0x2D4, zi ~0x1300) is ASSERTed against the pool;
 * growing the reserve is a BASE change (full flash). */
#define TOIT_VM_DATA_RESERVE 0x00002000
#define TOIT_VM_ZI_RESERVE   0x00004000

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
  /* Toit: 3072K reaches the 768 KB slot B + marker (ends 0xB13000), reclaiming
   * the dead FOTA region; stays below LittleFS (0xB84000) and the FDB. */
  FLASH_AREA(rx)              : ORIGIN = 0x00824000, LENGTH = 3072K         /* 3072K */
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
    /* PLAT/SDK writable .data: stays in the base image. It is live before the
     * VM boots (PLAT startup loads it and PLAT code mutates it), so it is NEVER
     * carried per-slot or overwritten by the slot copy below. */
    EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(.data*)
    /* The VM's writable .data lives in its own reserved section .vm_dram_data
     * BELOW, so its size never moves PLAT symbols or the heap start. */
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
    /* VM .bss/COMMON live in the reserved .vm_dram_zi section below. */
    EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(.bss*)
    EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(COMMON)
    . = ALIGN(4);
    *(.stack)               /* stack should be 4 byte align */
    Image$$LOAD_DRAM_SHARED$$ZI$$Limit = .;
    *(.USB_NOINIT_DATA_BUF)
  } >MSMB_AREA


  /* VM (+mbedtls) writable .data — the per-slot data region of the OTA
   * contract. Bracketed by __vm_data_start/__vm_data_end and grouped
   * contiguously so each firmware can carry its OWN .data init image inside
   * its slot: tools/ec618/gen-slot-reloc.toit extracts THIS range from the
   * base LMA and appends it to the slot; the device copies the ACTIVE slot's
   * copy back here at boot (toit_ec618.cc) before
   * relocate_data_slot_pointers() fixes the slot pointers. The {RAM base,
   * reserve} is part of the frozen base/VM ABI: see docs/ota-contract.md.
   * The section occupies only its ACTUAL size in flash; the reserve is
   * enforced by placing .vm_dram_zi at the reserve limit. */
  .vm_dram_data : ALIGN(4)
  {
    Load$$VM_DRAM_DATA$$Base = LOADADDR(.vm_dram_data);
    Image$$VM_DRAM_DATA$$Base = .;
    __vm_data_start = .;
    *libtoit_vm.a:*(.data*)
    *libmbedtls.a:*(.data*)
    *libmbedx509.a:*(.data*)
    *libmbedcrypto.a:*(.data*)
    . = ALIGN(4);
    __vm_data_end = .;
  } >MSMB_AREA AT>FLASH_AREA

  /* C-friendly alias of the section's flash LMA: the VM's boot path falls
   * back to this base-carried init image when the slot carries no .data
   * region (see load_active_slot_vm_data in src/toit_ec618.cc). */
  __vm_data_load = LOADADDR(.vm_dram_data);

  /* VM .bss, in the remainder of the reservation. PLAT's ZI loop does NOT
   * cover this section: the VM zeroes it itself at entry
   * (load_active_slot_vm_data in src/toit_ec618.cc, before anything reads
   * a VM static). */
  .vm_dram_zi (NOLOAD):
  {
    __vm_zi_start = .;
    *libtoit_vm.a:*(.bss*)
    *libtoit_vm.a:*(COMMON)
    *libmbedtls.a:*(.bss*)
    *libmbedtls.a:*(COMMON)
    *libmbedx509.a:*(.bss*)
    *libmbedx509.a:*(COMMON)
    *libmbedcrypto.a:*(.bss*)
    *libmbedcrypto.a:*(COMMON)
    . = ALIGN(4);
    __vm_zi_end = .;
  } >MSMB_AREA

  /* The heap starts at the RESERVE limit, not at the VM's actual end — the
   * whole point: VM .data/.bss growth inside the (pooled) reserve cannot
   * move it, so the base's heap placement survives any slot OTA that fits.
   * __vm_data_start depends only on PLAT's dram use, so it is stable for a
   * given base; the reserve is one pooled budget for data + zi. */
  PROVIDE(end_ap_data = __vm_data_start + TOIT_VM_DATA_RESERVE + TOIT_VM_ZI_RESERVE);
  ASSERT(__vm_zi_end <= end_ap_data,
         "VM .data+.bss exceeded the pooled VM dram reserve — grow it (BASE change, full flash)")
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
   * Dual-slot OTA layout (Toit fork). Each VM slot is 768 KB, reclaiming the
   * dead LuatOS FOTA region (0xB04000-0xB84000, unused by Toit and removed when
   * the FOTA copy-back was deleted). The LittleFS (0xB84000) and FDB / flash
   * registry (0xBCC000, used by Toit) above stay untouched.
   *
   *   0x848000-0x991000 : PLAT .text  (~1.27 MB used, ~50 KB headroom)
   *   0x991000-0xA51000 : .vm_a       (768 KB, slot A)
   *   0xA51000-0xB11000 : .vm_b       (768 KB, slot B)
   *   0xB11000-0xB13000 : .slot_marker (8 KB, two sectors — power-fail-safe
   *                                     active-slot record, ping-ponged)
   *   0xB13000-0xB84000 : free        (reclaimed FOTA region)
   *
   * The VM (libtoit_vm.a + mbedtls), the bundled extension (containers + config),
   * and the .vm_entry pointer are linked once at .vm_a (or .vm_b for the slot-B
   * byte-identity oracle, -DTOIT_VM_SLOT_B). PLAT objects fall through into
   * .text. The single position-independent image is RELOCATED to whichever slot
   * the device writes (relocate-on-write OTA); the slot-B link survives only as
   * the build-time byte-identity check.
   */
#define TOIT_VM_A_ORIGIN  0x00991000
#define TOIT_VM_B_ORIGIN  0x00A51000
#define TOIT_VM_SLOT_SIZE 0x000C0000
/* Neutral link base for the position-independent VM image. The image is LINKED
 * here (a VMA that is NEITHER slot) and RELOCATED to whichever slot it is
 * written to — INCLUDING slot A (LMA below = slot A via AT). Decoupling the link
 * base from the slot flash address means BOTH slots get a non-zero relocation
 * delta, so the slot-A relocation path is exercised for real (not a same-base
 * no-op) and a missed relocation faults on slot-A boot, not only after a B->A
 * OTA. Picked 0x00D00000: above the
 * 3 MB FLASH_AREA (ends 0xB24000) and outside every mapped region (a stray
 * un-relocated pointer faults loudly), yet close enough that EVERY escaping
 * VM->PLAT branch encodes as a direct Thumb-2 BL at link time — the binding
 * constraint is the ITCM-resident hot functions (memcpy & friends at
 * ~0x2600): the farthest branch source (link base + slot size = 0xDC0000)
 * is ~14.4 MB from them, inside BL's +-16.7 MB with margin. At the old
 * 0x01000000 base those branches were ~16.8 MB out, so ld emitted in-slot
 * long-branch veneers in the slot-A link but not the slot-B link, breaking
 * the byte-identity contract the relocation table depends on. To make slot
 * A canonical again, set this to TOIT_VM_A_ORIGIN. */
#define TOIT_VM_LINK_BASE 0x00D00000
#define TOIT_SLOT_MARKER_ORIGIN 0x00B11000
#define TOIT_SLOT_MARKER_SIZE   0x00002000  /* two 4 KB sectors, ping-ponged */
#define TOIT_PLAT_TEXT_LIMIT TOIT_VM_A_ORIGIN

  .text :
  {
    /* The PLAT keep-list (plat_keep.c): nothing references the address
     * table, so --gc-sections would drop it and, with it, the generous
     * PLAT API surface the frozen base guarantees to future slots
     * (__attribute__((used)) does NOT survive section GC). */
    KEEP(*(.rodata.toit_plat_keep))
    EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(.rodata*)
    EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.vfpll_veneer)
    *(.v4_bx)
    /* The bare glob `.init*` matches `.init_array` too, so it would STEAL the
     * VM archives' .init_array from the slot's KEEP below — leaving the slot's
     * __vm_init_array empty and the VM's static constructors unrun (their
     * .init_array pointers also bake VM-slot addresses into this fixed,
     * never-relocated region). Exclude the VM archives here (as for .text/.rodata
     * above) so their .init_array falls through to the .vm_a KEEP and is captured
     * INTO the slot, where run_static_initializers() runs it and the SRL1
     * relocation moves the pointers with the slot. */
    EXCLUDE_FILE (*libtoit_vm.a *libmbedtls.a *libmbedx509.a *libmbedcrypto.a) *(.init*)
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

  /* Linked at the neutral TOIT_VM_LINK_BASE (VMA), loaded into slot A's flash
   * region (LMA, via AT). __vm_link_base/__vm_link_end are the link-domain (VMA)
   * markers gen-slot-reloc relocates FROM; __vm_a_start/__vm_a_end stay the slot
   * flash geometry the device dispatcher and relocate targets use. */
  .vm_a TOIT_VM_LINK_BASE : AT (TOIT_VM_A_ORIGIN)
  {
    __vm_link_base = .;
#ifndef TOIT_VM_SLOT_B
    KEEP(*(.vm_entry))
    /* VM-side C++ static initializers live inside the slot so each slot
     * is self-contained. run_static_initializers() in src/toit_ec618.cc
     * iterates __vm_init_array_*. The VM's writable .data is bracketed
     * separately in .load_dram_shared (__vm_data_start/_end) and carried
     * PER-SLOT: its values are NOT slot-agnostic (the interpreter
     * dispatch_table and *_primitives_ hold in-slot pointers, and the
     * content differs between firmware builds), so each slot ships its own
     * .data init image and the device loads the active slot's copy at boot. */
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
    __vm_link_end = .;
  }
  /* Slot A flash geometry: where the (relocated) slot-A image physically lives.
   * Kept separate from the link base so the flash-address consumers (the slot
   * dispatcher, inactive/active_slot_base, the relocate targets) are unchanged. */
  __vm_a_start = TOIT_VM_A_ORIGIN;
  __vm_a_end   = TOIT_VM_A_ORIGIN + (__vm_link_end - __vm_link_base);

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
    /* Reserve both sectors so the AP binary spans the full marker region;
     * the extension (appended after the binary) therefore starts past
     * sector 1, leaving the ping-pong's second sector free to be written.
     * Fresh contents read as "no valid record" → the dispatcher boots
     * slot A (see slot_marker_read). */
    . = __slot_marker_start + TOIT_SLOT_MARKER_SIZE;
    __slot_marker_end = .;
  } >FLASH_AREA

  ASSERT(__slot_marker_end - __slot_marker_start == TOIT_SLOT_MARKER_SIZE,
         "slot marker region must reserve exactly TOIT_SLOT_MARKER_SIZE")

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