set_project("EC618")
set_xmakever("2.7.2")
set_version("0.0.2", {build = "%Y%m%d%H%M"})
add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

local VM_64BIT = nil
SDK_TOP = "."
local SDK_PATH
local USER_PROJECT_NAME = "example"
local USER_PROJECT_NAME_VERSION
USER_PROJECT_DIR  = ""
local LUAT_SCRIPT_SIZE
local LUAT_SCRIPT_OTA_SIZE
local script_addr = nil
local full_addr = nil

package("gnu_rm")
	set_kind("toolchain")
	set_homepage("https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm")
	set_description("GNU Arm Embedded Toolchain")
	local version_map = {
		["2021.10"] = "10.3-2021.10"
	}
	if is_host("windows") then
		set_urls("http://cdndownload.openluat.com/xmake/toolchains/gcc-arm/gcc-arm-none-eabi-$(version)-win32.zip", {version = function (version)
			return version_map[tostring(version)]
		end})
		add_versions("2021.10", "d287439b3090843f3f4e29c7c41f81d958a5323aecefcf705c203bfd8ae3f2e7")
	elseif is_host("linux") then
		set_urls("http://cdndownload.openluat.com/xmake/toolchains/gcc-arm/gcc-arm-none-eabi-$(version)-x86_64-linux.tar.bz2", {version = function (version)
			return version_map[tostring(version)]
		end})
		add_versions("2021.10", "97dbb4f019ad1650b732faffcc881689cedc14e2b7ee863d390e0a41ef16c9a3")
    elseif is_host("macosx") then
        set_urls("https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-$(version)-mac.tar.bz2", {version = function (version)
			return version_map[tostring(version)]
		end})
		add_versions("2021.10", "fb613dacb25149f140f73fe9ff6c380bb43328e6bf813473986e9127e2bc283b")
	end
	on_install("@windows", "@linux", "@macosx", function (package)
		os.vcp("*", package:installdir())
	end)
package_end()

if os.getenv("GCC_PATH") then
	toolchain("arm_toolchain")
	    set_kind("standalone")
	    set_sdkdir(os.getenv("GCC_PATH"))
	toolchain_end()
	set_toolchains("arm_toolchain")
else
	add_requires("gnu_rm 2021.10")
	set_toolchains("gnu-rm@gnu_rm")
end

-- 获取项目名称
if os.getenv("PROJECT_NAME") then
	USER_PROJECT_NAME = os.getenv("PROJECT_NAME")
end

-- 是否为rndis csdk
if os.getenv("EC618_RNDIS") == "enable" then
    is_rndis = true
    add_defines("LUAT_EC618_RNDIS_ENABLED=1")
else
    is_rndis = false
end

-- 是否启用低速模式, 内存更大, 但与rndis不兼容
if is_rndis == false and os.getenv("LSPD_MODE") == "enable" then
    is_lspd = true
else
    is_lspd = false
end

-- 若启用is_lspd, 加上额外的宏
if is_lspd == true then
    add_defines("LOW_SPEED_SERVICE_ONLY")
end

if os.getenv("ROOT_PATH") then
	SDK_TOP = os.getenv("ROOT_PATH")
else
	SDK_TOP = os.curdir()
end
SDK_TOP = SDK_TOP .. "/"
SDK_PATH = SDK_TOP

if os.getenv("PROJECT_DIR") then
    USER_PROJECT_DIR = os.getenv("PROJECT_DIR")
else
    USER_PROJECT_DIR = SDK_TOP .. "/project/" .. USER_PROJECT_NAME
end

set_plat("cross")
set_arch("arm")
set_languages("gnu99", "cxx11")
set_warnings("everything")

-- ==============================
-- === defines =====
add_defines("__EC618",
            "CHIP_EC618",
            "CORE_IS_AP",
            "SDK_REL_BUILD",
            "EC_ASSERT_FLAG",
            "PM_FEATURE_ENABLE",
            "UINILOG_FEATURE_ENABLE",
            "FEATURE_OS_ENABLE",
            "configUSE_NEWLIB_REENTRANT=1",
            "ARM_MATH_CM3",
            "FEATURE_YRCOMPRESS_ENABLE",
            "FEATURE_CCIO_ENABLE",
            "DHCPD_ENABLE_DEFINE=1",
            "LWIP_CONFIG_FILE=\"lwip_config_ec6180h00.h\"",
            "FEATURE_MBEDTLS_ENABLE",
            "LFS_NAME_MAX=63",
            "LFS_DEBUG_TRACE",
            "WDT_FEATURE_ENABLE=1",
            "FEATURE_UART_HELP_DUMP_ENABLE",
            "DEBUG_LOG_HEADER_FILE=\"debug_log_ap.h\"",
            "TRACE_LEVEL=5",
            "SOFTPACK_VERSION=\"\"",
            "HAVE_STRUCT_TIMESPEC",
            "HTTPS_WITH_CA",
            "FEATURE_HTTPC_ENABLE",
            -- "LITE_FEATURE_MODE",
            -- "RTE_RNDIS_EN=0", "RTE_ETHER_EN=0",
            "RTE_USB_EN=1",
            "RTE_PPP_EN=0",
            "RTE_OPAQ_EN=0",
            "RTE_ONE_UART_AT=0",
            "RTE_TWO_UART_AT=0",
            "__USER_CODE__",
            "__PRINT_ALIGNED_32BIT__",
            "_REENT_SMALL",
            "_REENT_GLOBAL_ATEXIT"
)

if is_rndis then
    
else
    add_defines("LITE_FEATURE_MODE")
end

set_optimize("smallest")
add_cxflags("-g3",
            "-mcpu=cortex-m3",
            "-mthumb",
            "-std=gnu99",
            "-nostartfiles",
            "-mapcs-frame",
            "-ffunction-sections",
            "-fdata-sections",
            "-fno-isolate-erroneous-paths-dereference",
            "-freorder-blocks-algorithm=stc",
            "-Wall",
            "-Wno-format",
            "-gdwarf-2",
            "-fno-inline",
            "-mslow-flash-data",
            "-fstack-usage",
            "-Wstack-usage=4096",
{force=true})

add_cxflags("-Werror=maybe-uninitialized")

add_ldflags(" -Wl,--wrap=clock ",{force = true})
add_ldflags(" -Wl,--wrap=localtime ",{force = true})
add_ldflags(" -Wl,--wrap=gmtime ",{force = true})
add_ldflags(" -Wl,--wrap=time ",{force = true})
add_ldflags(" -Wl,--wrap=SetUnilogUart", {force=true})
-- PLAT jump-table prototype: route every PLAT symbol the VM calls
-- through g_plat_jt[]. The block between the markers below is
-- regenerated by `tools/gen_plat_jt.py` from
-- `tools/plat_jt_ldflags.lua`; do not edit it by hand.
-- BEGIN PLAT_JT_LDFLAGS
add_ldflags(" -Wl,--wrap=BSP_QSPI_Erase_Safe ", {force = true})
add_ldflags(" -Wl,--wrap=BSP_QSPI_Read_Safe ", {force = true})
add_ldflags(" -Wl,--wrap=BSP_QSPI_Write_Safe ", {force = true})
add_ldflags(" -Wl,--wrap=BSP_SetPlatConfigItemValue ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_Config ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_IomuxEC618 ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_Output ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_PullConfig ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_clearInterruptFlags ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_getInterruptFlags ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_interruptConfig ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_pinConfig ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_pinRead ", {force = true})
add_ldflags(" -Wl,--wrap=GPIO_pinWrite ", {force = true})
add_ldflags(" -Wl,--wrap=OsaSystemTimeReadRamUtc ", {force = true})
add_ldflags(" -Wl,--wrap=OsaTimerSync ", {force = true})
add_ldflags(" -Wl,--wrap=ResetStateGet ", {force = true})
add_ldflags(" -Wl,--wrap=Uart_BaseInitEx ", {force = true})
add_ldflags(" -Wl,--wrap=Uart_ChangeBR ", {force = true})
add_ldflags(" -Wl,--wrap=Uart_DeInit ", {force = true})
add_ldflags(" -Wl,--wrap=Uart_IsTSREmpty ", {force = true})
add_ldflags(" -Wl,--wrap=Uart_RxBufferClear ", {force = true})
add_ldflags(" -Wl,--wrap=Uart_RxBufferRead ", {force = true})
add_ldflags(" -Wl,--wrap=Uart_TxTaskSafe ", {force = true})
add_ldflags(" -Wl,--wrap=XIC_EnableIRQ ", {force = true})
add_ldflags(" -Wl,--wrap=XIC_SetVector ", {force = true})
add_ldflags(" -Wl,--wrap=_ZNSt13random_device7_M_finiEv ", {force = true})
add_ldflags(" -Wl,--wrap=_ZNSt13random_device7_M_initERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE ", {force = true})
add_ldflags(" -Wl,--wrap=_ZNSt13random_device9_M_getvalEv ", {force = true})
add_ldflags(" -Wl,--wrap=_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv ", {force = true})
add_ldflags(" -Wl,--wrap=_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE13_S_copy_charsEPcPKcS7_ ", {force = true})
add_ldflags(" -Wl,--wrap=_ZSt25__throw_bad_function_callv ", {force = true})
add_ldflags(" -Wl,--wrap=_ZdaPv ", {force = true})
add_ldflags(" -Wl,--wrap=_ZdlPv ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_atexit ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_d2f ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_d2iz ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_d2lz ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dadd ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dcmpeq ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dcmpge ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dcmpgt ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dcmple ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dcmplt ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dcmpun ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_ddiv ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dmul ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_dsub ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_f2d ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_i2d ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_l2d ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_ldivmod ", {force = true})
add_ldflags(" -Wl,--wrap=__aeabi_uldivmod ", {force = true})
add_ldflags(" -Wl,--wrap=__assert_func ", {force = true})
add_ldflags(" -Wl,--wrap=__cxa_thread_atexit ", {force = true})
add_ldflags(" -Wl,--wrap=__emutls_get_address ", {force = true})
add_ldflags(" -Wl,--wrap=__popcountdi2 ", {force = true})
add_ldflags(" -Wl,--wrap=__popcountsi2 ", {force = true})
add_ldflags(" -Wl,--wrap=__wrap_time ", {force = true})
add_ldflags(" -Wl,--wrap=abort ", {force = true})
add_ldflags(" -Wl,--wrap=acos ", {force = true})
add_ldflags(" -Wl,--wrap=aligned_alloc ", {force = true})
add_ldflags(" -Wl,--wrap=apmuSetDeepestSleepMode ", {force = true})
add_ldflags(" -Wl,--wrap=appGetECBCInfoSync ", {force = true})
add_ldflags(" -Wl,--wrap=appSetCFUN ", {force = true})
add_ldflags(" -Wl,--wrap=asin ", {force = true})
add_ldflags(" -Wl,--wrap=atan ", {force = true})
add_ldflags(" -Wl,--wrap=atan2 ", {force = true})
add_ldflags(" -Wl,--wrap=calloc ", {force = true})
add_ldflags(" -Wl,--wrap=ceil ", {force = true})
add_ldflags(" -Wl,--wrap=cos ", {force = true})
add_ldflags(" -Wl,--wrap=cosh ", {force = true})
add_ldflags(" -Wl,--wrap=deregisterPSEventCallback ", {force = true})
add_ldflags(" -Wl,--wrap=exp ", {force = true})
add_ldflags(" -Wl,--wrap=fclose ", {force = true})
add_ldflags(" -Wl,--wrap=feof ", {force = true})
add_ldflags(" -Wl,--wrap=fflush ", {force = true})
add_ldflags(" -Wl,--wrap=floor ", {force = true})
add_ldflags(" -Wl,--wrap=fmod ", {force = true})
add_ldflags(" -Wl,--wrap=fopen ", {force = true})
add_ldflags(" -Wl,--wrap=fotaNvmNfsPeInit ", {force = true})
add_ldflags(" -Wl,--wrap=fputc ", {force = true})
add_ldflags(" -Wl,--wrap=fputs ", {force = true})
add_ldflags(" -Wl,--wrap=fread ", {force = true})
add_ldflags(" -Wl,--wrap=free ", {force = true})
add_ldflags(" -Wl,--wrap=fseek ", {force = true})
add_ldflags(" -Wl,--wrap=fwrite ", {force = true})
add_ldflags(" -Wl,--wrap=gmtime_r ", {force = true})
add_ldflags(" -Wl,--wrap=isspace ", {force = true})
add_ldflags(" -Wl,--wrap=localtime_r ", {force = true})
add_ldflags(" -Wl,--wrap=log ", {force = true})
add_ldflags(" -Wl,--wrap=malloc ", {force = true})
add_ldflags(" -Wl,--wrap=memchr ", {force = true})
add_ldflags(" -Wl,--wrap=memcmp ", {force = true})
add_ldflags(" -Wl,--wrap=memcpy ", {force = true})
add_ldflags(" -Wl,--wrap=memmove ", {force = true})
add_ldflags(" -Wl,--wrap=memset ", {force = true})
add_ldflags(" -Wl,--wrap=mktime ", {force = true})
add_ldflags(" -Wl,--wrap=osDelay ", {force = true})
add_ldflags(" -Wl,--wrap=osKernelGetTickCount ", {force = true})
add_ldflags(" -Wl,--wrap=pbuf_alloc ", {force = true})
add_ldflags(" -Wl,--wrap=pbuf_cat ", {force = true})
add_ldflags(" -Wl,--wrap=pbuf_free ", {force = true})
add_ldflags(" -Wl,--wrap=pbuf_ref ", {force = true})
add_ldflags(" -Wl,--wrap=pow ", {force = true})
add_ldflags(" -Wl,--wrap=printf ", {force = true})
add_ldflags(" -Wl,--wrap=psSetCdgcont ", {force = true})
add_ldflags(" -Wl,--wrap=putchar ", {force = true})
add_ldflags(" -Wl,--wrap=putenv ", {force = true})
add_ldflags(" -Wl,--wrap=puts ", {force = true})
add_ldflags(" -Wl,--wrap=realloc ", {force = true})
add_ldflags(" -Wl,--wrap=registerPSEventCallback ", {force = true})
add_ldflags(" -Wl,--wrap=rngGenRandom ", {force = true})
add_ldflags(" -Wl,--wrap=round ", {force = true})
add_ldflags(" -Wl,--wrap=sin ", {force = true})
add_ldflags(" -Wl,--wrap=sinh ", {force = true})
add_ldflags(" -Wl,--wrap=slot_marker_read ", {force = true})
add_ldflags(" -Wl,--wrap=slot_marker_write ", {force = true})
add_ldflags(" -Wl,--wrap=slpManApplyPlatVoteHandle ", {force = true})
add_ldflags(" -Wl,--wrap=slpManDeepSlpTimerRegisterExpCb ", {force = true})
add_ldflags(" -Wl,--wrap=slpManDeepSlpTimerStart ", {force = true})
add_ldflags(" -Wl,--wrap=slpManPlatVoteDisableSleep ", {force = true})
add_ldflags(" -Wl,--wrap=slpManPlatVoteEnableSleep ", {force = true})
add_ldflags(" -Wl,--wrap=slpManSetPmuSleepMode ", {force = true})
add_ldflags(" -Wl,--wrap=snprintf ", {force = true})
add_ldflags(" -Wl,--wrap=soc_power_mode ", {force = true})
add_ldflags(" -Wl,--wrap=sprintf ", {force = true})
add_ldflags(" -Wl,--wrap=sqrt ", {force = true})
add_ldflags(" -Wl,--wrap=strchr ", {force = true})
add_ldflags(" -Wl,--wrap=strcmp ", {force = true})
add_ldflags(" -Wl,--wrap=strcpy ", {force = true})
add_ldflags(" -Wl,--wrap=strdup ", {force = true})
add_ldflags(" -Wl,--wrap=strerror ", {force = true})
add_ldflags(" -Wl,--wrap=strlen ", {force = true})
add_ldflags(" -Wl,--wrap=strncmp ", {force = true})
add_ldflags(" -Wl,--wrap=strncpy ", {force = true})
add_ldflags(" -Wl,--wrap=strnlen ", {force = true})
add_ldflags(" -Wl,--wrap=strstr ", {force = true})
add_ldflags(" -Wl,--wrap=strtod ", {force = true})
add_ldflags(" -Wl,--wrap=tan ", {force = true})
add_ldflags(" -Wl,--wrap=tanh ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_accept ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_arg ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_bind ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_close ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_connect ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_err ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_listen_with_backlog ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_new ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_output ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_recv ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_recved ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_sent ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_shutdown ", {force = true})
add_ldflags(" -Wl,--wrap=tcp_write ", {force = true})
add_ldflags(" -Wl,--wrap=tcpip_callback_with_block ", {force = true})
add_ldflags(" -Wl,--wrap=trunc ", {force = true})
add_ldflags(" -Wl,--wrap=tzset ", {force = true})
add_ldflags(" -Wl,--wrap=udp_bind ", {force = true})
add_ldflags(" -Wl,--wrap=udp_connect ", {force = true})
add_ldflags(" -Wl,--wrap=udp_new ", {force = true})
add_ldflags(" -Wl,--wrap=udp_recv ", {force = true})
add_ldflags(" -Wl,--wrap=udp_remove ", {force = true})
add_ldflags(" -Wl,--wrap=udp_send ", {force = true})
add_ldflags(" -Wl,--wrap=udp_sendto ", {force = true})
add_ldflags(" -Wl,--wrap=vPortGetHeapStats ", {force = true})
add_ldflags(" -Wl,--wrap=vPortGetHeapTag ", {force = true})
add_ldflags(" -Wl,--wrap=vPortIterateAllocations ", {force = true})
add_ldflags(" -Wl,--wrap=vPortSetHeapTag ", {force = true})
add_ldflags(" -Wl,--wrap=vQueueDelete ", {force = true})
add_ldflags(" -Wl,--wrap=vTaskDelete ", {force = true})
add_ldflags(" -Wl,--wrap=vfprintf ", {force = true})
add_ldflags(" -Wl,--wrap=xQueueCreateMutex ", {force = true})
add_ldflags(" -Wl,--wrap=xQueueGenericCreate ", {force = true})
add_ldflags(" -Wl,--wrap=xQueueGenericReceive ", {force = true})
add_ldflags(" -Wl,--wrap=xQueueGenericSend ", {force = true})
add_ldflags(" -Wl,--wrap=xQueueGenericSendFromISR ", {force = true})
add_ldflags(" -Wl,--wrap=xQueueGetMutexHolder ", {force = true})
add_ldflags(" -Wl,--wrap=xTaskCreate ", {force = true})
add_ldflags(" -Wl,--wrap=xTaskGenericNotify ", {force = true})
add_ldflags(" -Wl,--wrap=xTaskGetCurrentTaskHandle ", {force = true})
add_ldflags(" -Wl,--wrap=xTaskNotifyWait ", {force = true})
-- END PLAT_JT_LDFLAGS

add_ldflags("--specs=nano.specs", {force=true})
add_asflags("-Wl,--cref -Wl,--check-sections -Wl,--gc-sections -lm -Wl,--print-memory-usage -Wl,--wrap=_malloc_r -Wl,--wrap=_free_r -Wl,--wrap=_realloc_r -Wl,--wrap=_memalign_r -mcpu=cortex-m3 -mthumb -DTRACE_LEVEL=5 -DSOFTPACK_VERSION=\"\" -DHAVE_STRUCT_TIMESPEC")

add_defines("sprintf=sprintf_")
add_defines("snprintf=snprintf_")
add_defines("vsnprintf=vsnprintf_")

-- ==============================
-- === includes =====

add_includedirs(
                SDK_TOP .. "/PLAT/device/target/board/common/ARMCM3/inc",
                SDK_TOP .. "/PLAT/device/target/board/ec618_0h00/common/inc",
                SDK_TOP .. "/PLAT/device/target/board/ec618_0h00/ap/gcc",
                SDK_TOP .. "/PLAT/device/target/board/ec618_0h00/ap/inc",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/audio",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/camera",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/camera/bf30a2",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/camera/gc6153",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/camera/gc032A",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/camera/gc6123",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/camera/sp0A39",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/camera/sp0821",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/eeprom",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/lcd",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/lcd/ST7571",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/lcd/ST7789V2",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/inc/ntc",
                SDK_TOP .. "/PLAT/driver/chip/ec618/ap/inc",
                SDK_TOP .. "/PLAT/driver/chip/ec618/ap/inc_cmsis",
                SDK_TOP .. "/PLAT/driver/hal/common/inc",
                SDK_TOP .. "/PLAT/driver/hal/ec618/ap/inc",
                SDK_TOP .. "/PLAT/os/freertos/inc",
                SDK_TOP .. "/PLAT/os/freertos/CMSIS/common/inc",
                SDK_TOP .. "/PLAT/os/freertos/CMSIS/ap/inc",
                SDK_TOP .. "/PLAT/os/freertos/portable/mem/tlsf",
                SDK_TOP .. "/PLAT/os/freertos/portable/gcc",
                SDK_TOP .. "/PLAT/middleware/developed/debug/inc",
                SDK_TOP .. "/PLAT/middleware/developed/nvram/inc",
                SDK_TOP .. "/PLAT/middleware/developed/cms/psdial/inc",
                SDK_TOP .. "/PLAT/middleware/developed/cms/cms/inc",
                SDK_TOP .. "/PLAT/middleware/developed/cms/psil/inc",
                SDK_TOP .. "/PLAT/middleware/developed/cms/psstk/inc",
                SDK_TOP .. "/PLAT/middleware/developed/cms/sockmgr/inc",
                SDK_TOP .. "/PLAT/middleware/developed/cms/cmsnetlight/inc",
                SDK_TOP .. "/PLAT/middleware/developed/ecapi/appmwapi/inc",
                SDK_TOP .. "/PLAT/middleware/developed/ecapi/psapi/inc",
                SDK_TOP .. "/PLAT/middleware/developed/common/inc",
                SDK_TOP .. "/PLAT/middleware/developed/psnv/inc",
                SDK_TOP .. "/PLAT/os/freertos/portable/gcc",
                SDK_TOP .. "/PLAT/middleware/developed/tcpipmgr/app/inc",
                SDK_TOP .. "/PLAT/middleware/developed/tcpipmgr/common/inc",
                SDK_TOP .. "/PLAT/os/freertos/inc",
                SDK_TOP .. "/PLAT/middleware/developed/yrcompress",
                SDK_TOP .. "/PLAT/os/freertos/portable/gcc",
                SDK_TOP .. "/PLAT/prebuild/PS/inc",
                SDK_TOP .. "/PLAT/middleware/thirdparty/lwip/src/include",
                SDK_TOP .. "/PLAT/middleware/thirdparty/lwip/src/include/lwip",
                SDK_TOP .. "/PLAT/middleware/developed/ccio/pub",
                SDK_TOP .. "/PLAT/middleware/developed/ccio/device/inc",
                SDK_TOP .. "/PLAT/middleware/developed/ccio/service/inc",
                SDK_TOP .. "/PLAT/middleware/developed/ccio/custom/inc",
                SDK_TOP .. "/PLAT/middleware/developed/fota/pub",
                SDK_TOP .. "/PLAT/middleware/developed/fota/custom/inc",
                SDK_TOP .. "/PLAT/middleware/developed/at/atdecoder/inc",
                SDK_TOP .. "/PLAT/middleware/developed/at/atps/inc",
                SDK_TOP .. "/PLAT/middleware/developed/at/atps/inc/cnfind",
                SDK_TOP .. "/PLAT/middleware/developed/at/atcust/inc",
                SDK_TOP .. "/PLAT/middleware/developed/at/atcust/inc/cnfind",
                SDK_TOP .. "/PLAT/middleware/developed/at/atentity/inc",
                SDK_TOP .. "/PLAT/middleware/developed/at/atreply/inc",
                SDK_TOP .. "/PLAT/middleware/developed/at/atref/inc",
                SDK_TOP .. "/PLAT/middleware/developed/at/atref/inc/cnfind",
                SDK_TOP .. "/PLAT/os/freertos/portable/gcc",
                SDK_TOP .. "/thirdparty/httpclient",
                SDK_TOP .. "/PLAT/os/freertos/portable/gcc",
                SDK_TOP .. "/PLAT/middleware/thirdparty/lwip/src/include",
                SDK_TOP .. "/PLAT/middleware/thirdparty/lwip/src/include/posix",
                SDK_TOP .. "/PLAT/os/freertos/inc",
                SDK_TOP .. "/PLAT/os/freertos/portable/gcc",
                SDK_TOP .. "/thirdparty/littlefs",
                SDK_TOP .. "/thirdparty/littlefs/port",
                SDK_TOP .. "/PLAT/os/freertos/portable/gcc",
                SDK_TOP .. "/PLAT/prebuild/PS/inc",
                SDK_TOP .. "/PLAT/prebuild/PLAT/inc",
                SDK_TOP .. "/PLAT/core/common/include",
                SDK_TOP .. "/PLAT/core/tts/include",
                SDK_TOP .. "/PLAT/core/multimedia/include",
                SDK_TOP .. "/PLAT/core/driver/include",
                SDK_TOP .. "/thirdparty/linksdk",
                SDK_TOP .. "/thirdparty/printf",
{public = true})

if USER_PROJECT_NAME ~= 'luatos' then
    add_defines("MBEDTLS_CONFIG_FILE=\"config_user_ssl.h\"")
    add_includedirs(SDK_TOP .. "/interface/include", 
                    SDK_TOP .. "/interface/base_include", 
                    SDK_TOP .. "/interface/private_include", 
                    SDK_TOP .. "/thirdparty/mbedtls/include",
                    SDK_TOP .. "/thirdparty/mbedtls/include/mbedtls",
                    SDK_TOP .. "/thirdparty/mbedtls/configs",
                    SDK_TOP .. "/thirdparty/fal/inc",
                    SDK_TOP .. "/thirdparty/flashdb/inc",
                    {public = true})
else
    if os.getenv("LUAT_EC618_LITE_MODE") == "1" then
        add_defines("LUAT_EC618_LITE_MODE", "LUAT_SCRIPT_SIZE=448", "LUAT_SCRIPT_OTA_SIZE=284")
    end
    if os.getenv("LUAT_USE_TTS") == "1" then
        add_defines("LUAT_USE_TTS")
    end
    if os.getenv("LUAT_USE_TTS_ONCHIP") == "1" then
        add_defines("LUAT_USE_TTS_ONCHIP")
    end
    add_defines("__LUATOS__","LWIP_NUM_SOCKETS=8")
    add_defines("MBEDTLS_CONFIG_FILE=\"mbedtls_ec618_config.h\"")
end
--linkflags
local LD_BASE_FLAGS = "-Wl,--cref -Wl,--check-sections -Wl,--gc-sections -lm -Wl,--print-memory-usage"
LD_BASE_FLAGS = LD_BASE_FLAGS .. " -L" .. SDK_TOP .. "/PLAT/device/target/board/ec618_0h00/ap/gcc/"
--LD_BASE_FLAGS = LD_BASE_FLAGS .. " -T" .. SDK_TOP .. "/PLAT/device/target/board/ec618_0h00/ap/gcc/ec618_0h00_flash.ld -Wl,-Map,$(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME.."_$(mode).map "
LD_BASE_FLAGS = LD_BASE_FLAGS .. " -T" .. SDK_TOP .. "/PLAT/core/ld/ec618_0h00_flash.ld -Wl,-Map,$(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME.."_$(mode).map "
LD_BASE_FLAGS = LD_BASE_FLAGS .. " -Wl,--wrap=_malloc_r -Wl,--wrap=_free_r -Wl,--wrap=_realloc_r -Wl,--wrap=_memalign_r -mcpu=cortex-m3 -mthumb -DTRACE_LEVEL=5 -DSOFTPACK_VERSION=\"\" -DHAVE_STRUCT_TIMESPEC"
local LIB_BASE = SDK_TOP .. "/PLAT/libs/libstartup.a "
LIB_BASE = LIB_BASE .. SDK_TOP .. "/PLAT/libs/libcore_airm2m.a "
LIB_BASE = LIB_BASE .. SDK_TOP .. "/PLAT/libs/libfreertos.a "
LIB_BASE = LIB_BASE .. SDK_TOP .. "/PLAT/libs/libpsnv.a "
LIB_BASE = LIB_BASE .. SDK_TOP .. "/PLAT/libs/libtcpipmgr.a "
LIB_BASE = LIB_BASE .. SDK_TOP .. "/PLAT/libs/libyrcompress.a "
LIB_BASE = LIB_BASE .. SDK_TOP .. "/PLAT/libs/libmiddleware_ec.a "
LIB_BASE = LIB_BASE .. SDK_TOP .. "/PLAT/libs/liblwip.a "
if os.getenv("LUAT_FAST_ADD_USER_LIB") == "1" then
    LIB_BASE = LIB_BASE .. SDK_TOP .. os.getenv("USER_LIB") .. " "
end

if is_rndis then
    LIB_PS_PRE = SDK_TOP .. "/PLAT/prebuild/PS/lib/gcc"
    LIB_PLAT_PRE = SDK_TOP .. "/PLAT/prebuild/PLAT/lib/gcc"
else
    LIB_PS_PRE = SDK_TOP .. "/PLAT/prebuild/PS/lib/gcc/lite"
    LIB_PLAT_PRE = SDK_TOP .. "/PLAT/prebuild/PLAT/lib/gcc/lite"
end
LIB_BASE = LIB_BASE .. LIB_PS_PRE .. "/libps.a "
LIB_BASE = LIB_BASE .. LIB_PS_PRE .. "/libpsl1.a "
LIB_BASE = LIB_BASE .. LIB_PS_PRE .. "/libpsif.a "
LIB_BASE = LIB_BASE .. LIB_PLAT_PRE .. "/libosa.a "
LIB_BASE = LIB_BASE .. LIB_PLAT_PRE .. "/libmiddleware_ec_private.a "
LIB_BASE = LIB_BASE .. LIB_PLAT_PRE .. "/libccio.a "
LIB_BASE = LIB_BASE .. LIB_PLAT_PRE .. "/libdeltapatch.a "
LIB_BASE = LIB_BASE .. LIB_PLAT_PRE .. "/libfota.a "
LIB_BASE = LIB_BASE .. LIB_PLAT_PRE .. "/libdriver_private.a "
LIB_BASE = LIB_BASE .. LIB_PLAT_PRE .. "/libusb_private.a "
LIB_USER = ""

after_load(function (target)
    for _, sourcebatch in pairs(target:sourcebatches()) do
        if sourcebatch.sourcekind == "as" then -- only asm files
            for idx, objectfile in ipairs(sourcebatch.objectfiles) do
                sourcebatch.objectfiles[idx] = objectfile:gsub("%.S%.o", ".o")
            end
        end
        if sourcebatch.sourcekind == "cc" then -- only c files
            for idx, objectfile in ipairs(sourcebatch.objectfiles) do
                sourcebatch.objectfiles[idx] = objectfile:gsub("%.c%.o", ".o")
            end
        end
    end
end)

target("driver")
    set_kind("static")
    add_deps(USER_PROJECT_NAME)
	--driver
	add_files(SDK_TOP .. "/PLAT/driver/board/ec618_0h00/src/**.c",
                SDK_TOP .. "/PLAT/driver/chip/ec618/ap/**.c",
                SDK_TOP .. "/PLAT/driver/chip/ec618/common/gcc/memcpy-armv7m.S",
                SDK_TOP .. "/PLAT/driver/hal/**.c",
                SDK_TOP .. "/PLAT/core/speed/*.c"
    )
	
	remove_files(SDK_TOP .. "/PLAT/driver/board/ec618_0h00/src/camera/camAT.c",
                SDK_TOP .. "/PLAT/driver/board/ec618_0h00/src/exstorage/*.c",
				SDK_TOP.."/PLAT/driver/chip/ec618/ap/src/usb/usb_device/usb_bl_test.c",
				SDK_TOP.."/PLAT/driver/chip/ec618/ap/src_cmsis/bsp_lpusart_stub.c",
				SDK_TOP.."/PLAT/driver/chip/ec618/ap/src/tls.c",
                SDK_TOP.."/PLAT/driver/chip/ec618/ap/src_cmsis/bsp_spi.c"
	)

    set_targetdir("$(buildir)/libdriver_" .. USER_PROJECT_NAME)
target_end()

includes(USER_PROJECT_DIR)

target(USER_PROJECT_NAME..".elf")
	set_kind("binary")
    -- add_deps(USER_PROJECT_NAME)
    set_targetdir("$(buildir)/"..USER_PROJECT_NAME)
    add_deps("driver")
	-- if os.getenv("GCC_PATH") then
	-- 	LD_BASE_FLAGS = " --specs=nano.specs " .. LD_BASE_FLAGS
	-- end

    if USER_PROJECT_NAME ~= 'luatos' and USER_PROJECT_NAME ~= 'toit' then
        add_files(SDK_TOP .. "/interface/private_src/*.c",{public = true})
        add_files(SDK_TOP .. "/thirdparty/mbedtls/library/*.c",{public = true})
        add_files(SDK_TOP .. "/thirdparty/printf/*.c",{public = true})
		add_files(SDK_TOP.."/thirdparty/fal/src/*.c",{public = true})
        add_files(SDK_TOP.."/thirdparty/flashdb/src/*.c",{public = true})
		add_files(SDK_TOP .. "/interface/src/*.c",{public = true})
		add_files(SDK_TOP .. "/thirdparty/littlefs/**.c",{public = true})
    elseif USER_PROJECT_NAME == 'toit' then
        -- Toit brings its own mbedTLS (from the esp-idf copy with Toit patches).
        -- Still need printf, littlefs, and interface sources.
        add_files(SDK_TOP .. "/interface/private_src/*.c",{public = true})
        add_files(SDK_TOP .. "/thirdparty/printf/*.c",{public = true})
        add_files(SDK_TOP .. "/thirdparty/littlefs/**.c",{public = true})
        add_files(SDK_TOP .. "/interface/src/*.c",{public = true})
        -- cmpctmalloc heap replaces heap_6 from libfreertos.a.
        add_includedirs(SDK_TOP .. "/PLAT/os/freertos/portable/mem/cmpctmalloc",{public = true})
        add_files(SDK_TOP .. "/PLAT/os/freertos/portable/mem/cmpctmalloc/cmpctmalloc.c",{public = true})
        add_files(SDK_TOP .. "/PLAT/os/freertos/src/heap_7.c",{public = true})
        -- bsp_custom.c overrides the default BSP_CustomInit from libcore_airm2m.a.
        add_files(SDK_TOP .. "/project/" .. USER_PROJECT_NAME .. "/src/bsp_custom.c",{public = true})
        -- sys_ro_override.c overrides sysROSpaceCheck from libstartup.a, and
        -- exports the toit_ap_image_modify_{start,end} window the OTA commit
        -- step uses to write into the AP image area.
        add_files(SDK_TOP .. "/project/" .. USER_PROJECT_NAME .. "/src/sys_ro_override.c",{public = true})
        add_ldflags("-Wl,--allow-multiple-definition", {force = true})
        -- Dual-slot OTA: retain input relocations in toit.elf so
        -- tools/ec618/gen-slot-reloc.toit can extract the slot's
        -- relocation table (R_ARM_ABS32 data pointers + the __wrap_time
        -- branch) for relocate-on-write. objcopy -O binary drops the
        -- relocation sections, so ap.bin is unaffected.
        add_ldflags("-Wl,--emit-relocs", {force = true})
    else
        remove_files(SDK_TOP .. "/interface/src/luat_kv_ec618.c")
    end
    

	add_ldflags(LD_BASE_FLAGS .. " -Wl,--whole-archive -Wl,--start-group " .. LIB_BASE .. LIB_USER .. " -Wl,--end-group -Wl,--no-whole-archive -Wl,--no-undefined -Wl,--no-print-map-discarded  -ldriver", {force=true})
	
    on_load(function (target)
        if USER_PROJECT_NAME == 'luatos' then
            local conf_data = io.readfile("$(projectdir)/project/luatos/inc/luat_conf_bsp.h")
            USER_PROJECT_NAME_VERSION = conf_data:match("#define LUAT_BSP_VERSION \"(%w+)\"")
            VM_64BIT = conf_data:find("\r#define LUAT_CONF_VM_64bit") or conf_data:find("\n#define LUAT_CONF_VM_64bit")
            local TTS_ONCHIP = conf_data:find("\r#define LUAT_USE_TTS_ONCHIP") or conf_data:find("\n#define LUAT_USE_TTS_ONCHIP")
            local TLS_DISABLE = conf_data:find("\r#define LUAT_USE_TLS_DISABLE") or conf_data:find("\n#define LUAT_USE_TLS_DISABLE")

            local mem_map_data = io.readfile("$(projectdir)/PLAT/device/target/board/ec618_0h00/common/inc/mem_map.h")
            FLASH_FOTA_REGION_START = tonumber(mem_map_data:match("#define FLASH_FOTA_REGION_START%s+%((%g+)%)"))
            if (TTS_ONCHIP or os.getenv("LUAT_USE_TTS_ONCHIP") == "1") and not TLS_DISABLE then
                LUAT_SCRIPT_SIZE = 64
                LUAT_SCRIPT_OTA_SIZE = 48
            elseif os.getenv("LUAT_EC618_LITE_MODE") == "1" then
                LUAT_SCRIPT_SIZE = 448
                LUAT_SCRIPT_OTA_SIZE = 284
            else
                LUAT_SCRIPT_SIZE = tonumber(conf_data:match("\r#define LUAT_SCRIPT_SIZE (%d+)") or conf_data:match("\n#define LUAT_SCRIPT_SIZE (%d+)"))
                LUAT_SCRIPT_OTA_SIZE = tonumber(conf_data:match("\r#define LUAT_SCRIPT_OTA_SIZE (%d+)") or conf_data:match("\n#define LUAT_SCRIPT_OTA_SIZE (%d+)"))
            end
            print(string.format("script zone %d ota %d", LUAT_SCRIPT_SIZE, LUAT_SCRIPT_OTA_SIZE))
            LUA_SCRIPT_ADDR = FLASH_FOTA_REGION_START - (LUAT_SCRIPT_SIZE + LUAT_SCRIPT_OTA_SIZE) * 1024
            LUA_SCRIPT_OTA_ADDR = FLASH_FOTA_REGION_START - LUAT_SCRIPT_OTA_SIZE * 1024
            script_addr = string.format("%X", LUA_SCRIPT_ADDR)
            full_addr = string.format("%X", LUA_SCRIPT_OTA_ADDR)
            -- print(FLASH_FOTA_REGION_START,LUAT_SCRIPT_SIZE,LUAT_SCRIPT_OTA_SIZE)
            -- print(script_addr,full_addr)
            
        end
    end)
    before_build(function(target)
        if os.getenv("GCC_PATH") then
            GCC_DIR = os.getenv("GCC_PATH").."/"
        else
            GCC_DIR = target:toolchains()[1]:sdkdir().."/"
        end
        if os.getenv("LSPD_MODE") == "enable" then
            FLAGS = "-DLOW_SPEED_SERVICE_ONLY"
        else
            FLAGS = ""
        end
        if USER_PROJECT_NAME == "luatos" then
            FLAGS = FLAGS .. " -D__LUATOS__ -DFLASH_AREA_SIZE=" .. string.format("%dK", 2944 - LUAT_SCRIPT_SIZE - LUAT_SCRIPT_OTA_SIZE)
        end
        -- Toit dual-slot OTA: TOIT_VM_SLOT_B=1 produces the slot-B link
        -- pass that places the VM at .vm_b instead of .vm_a.
        if os.getenv("TOIT_VM_SLOT_B") == "1" then
            FLAGS = FLAGS .. " -DTOIT_VM_SLOT_B"
        end
        os.exec(GCC_DIR .. "bin/arm-none-eabi-gcc -E " .. FLAGS .. " -I " .. SDK_PATH .. "/PLAT/device/target/board/ec618_0h00/common/inc" .. " -P " .. SDK_PATH .. "/PLAT/core/ld/ec618_0h00_flash.c" ..  " -o " .. SDK_PATH .. "/PLAT/core/ld/ec618_0h00_flash.ld")
        
    end)
	after_build(function(target)
		if os.getenv("GCC_PATH") then
			GCC_DIR = os.getenv("GCC_PATH").."/"
		else
			GCC_DIR = target:toolchains()[1]:sdkdir().."/"
		end
		OUT_PATH = SDK_PATH .. "/out/" ..USER_PROJECT_NAME
		if not os.exists(OUT_PATH) then
			os.mkdir(OUT_PATH)
		end
		os.exec(GCC_DIR .. "bin/arm-none-eabi-objcopy -O binary $(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".elf $(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".bin")
		-- io.writefile("$(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".list", os.iorun(GCC_DIR .. "bin/arm-none-eabi-objdump -h -S $(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".elf"))
		io.writefile("$(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".size", os.iorun(GCC_DIR .. "bin/arm-none-eabi-size $(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".elf"))
		-- io.cat("$(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".size")
		os.exec(GCC_DIR .. "bin/arm-none-eabi-objcopy -O binary $(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".elf $(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".bin")
		os.exec(GCC_DIR .."bin/arm-none-eabi-size $(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".elf")
        os.cp("$(buildir)/"..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".bin", "$(buildir)/"..USER_PROJECT_NAME.."/ap.bin")
        
        os.cp("$(buildir)/"..USER_PROJECT_NAME.."/*.bin", OUT_PATH)
		os.cp("$(buildir)/"..USER_PROJECT_NAME.."/*.map", OUT_PATH)
		os.cp("$(buildir)/"..USER_PROJECT_NAME.."/*.elf", OUT_PATH)
		os.cp("./PLAT/comdb.txt", OUT_PATH)

        ---------------------------------------------------------
        -------------- 这部分尚不能跨平台
        local cmd = "-M -input ./PLAT/tools/ap_bootloader.bin -addrname  BL_IMG_MERGE_ADDR -flashsize BOOTLOADER_FLASH_LOAD_SIZE -input $(buildir)/"..USER_PROJECT_NAME.."/ap.bin -addrname  AP_IMG_MERGE_ADDR -flashsize AP_FLASH_LOAD_SIZE -input ./PLAT/prebuild/FW/lib/cp-demo-flash.bin -addrname CP_IMG_MERGE_ADDR -flashsize CP_FLASH_LOAD_SIZE -def ./PLAT/device/target/board/ec618_0h00/common/inc/mem_map.h "
        if os.getenv("BINPKG_CROSS") then
            -- 准备自定义打包程序
            cmd = os.getenv("BINPKG_CROSS") .. cmd
        else
            if is_plat("windows") then
                cmd = "./PLAT/tools/fcelf.exe " .. cmd
            elseif is_plat("macosx") then
                cmd = "bash ./fcelf-docker.sh " .. cmd
            else
                cmd = "./fcelf " .. cmd
            end
        end
        cmd = cmd .. " -outfile " .. "./out/" ..USER_PROJECT_NAME.."/"..USER_PROJECT_NAME..".binpkg"
        -- 如果所在平台没有fcelf, 可注释掉下面的行, 没有binpkg生成. 
        -- 仍可使用其他工具继续刷机
        print("fcelf CMD --> ", cmd)
        os.exec(cmd)
        ---------------------------------------------------------

        import("lib.detect.find_file")
        local path7z = nil
        if is_plat("windows") then
            path7z = "\"$(programdir)/winenv/bin/7z.exe\""
        elseif is_plat("linux") or is_plat("macosx") then
            path7z = find_file("7z", { "/usr/bin/", "/usr/local/bin/" })
            if not path7z then
                path7z = find_file("7zr", { "/usr/bin/"})
            end
        end
        if path7z == nil then
            print("7z not find")
            return
        end
        if USER_PROJECT_NAME == 'luatos' then
            os.cp("$(projectdir)/project/luatos/pack", OUT_PATH)
            import("core.base.json")
            local info_table = json.loadfile(OUT_PATH.."/pack/info.json")
            if VM_64BIT then
                info_table["script"]["bitw"] = 64
            end
            if script_addr then
                info_table["download"]["script_addr"] = script_addr
                info_table["rom"]["fs"]["script"]["size"] = LUAT_SCRIPT_SIZE
                io.gsub(OUT_PATH.."/pack/config_ec618_usb.ini", "filepath = .\\script.bin\nburnaddr = 0x(%g+)", "filepath = .\\script.bin\nburnaddr = 0x"..script_addr)
            end
            if full_addr then
                info_table["fota"]["full_addr"] = full_addr
            end
            json.savefile(OUT_PATH.."/pack/info.json", info_table)
            os.cp(OUT_PATH.."/luatos.binpkg", OUT_PATH.."/pack")
            os.cp(OUT_PATH.."/luatos.elf", OUT_PATH.."/pack")
            os.cp("./PLAT/comdb.txt", OUT_PATH.."/pack")
            os.cp("./PLAT/device/target/board/ec618_0h00/common/inc/mem_map.h", OUT_PATH .. "/pack")
            os.cp("$(projectdir)/project/luatos/inc/luat_conf_bsp.h", OUT_PATH.."/pack")
            os.exec(path7z.." a -mx9 LuatOS-SoC_"..USER_PROJECT_NAME_VERSION.."_EC618.7z "..OUT_PATH.."/pack/* -r")
            local ver = "_FULL"
            if os.getenv("LUAT_EC618_LITE_MODE") == "1" then
                ver = ""
            end
            if os.getenv("LUAT_USE_TTS") == "1" then
                ver = "_TTS"
                if os.getenv("LUAT_USE_TTS_ONCHIP") == "1" then
                    ver = "_TTS_ONCHIP"
                end
            end
            os.mv("LuatOS-SoC_"..USER_PROJECT_NAME_VERSION.."_EC618.7z", OUT_PATH.."/LuatOS-SoC_"..USER_PROJECT_NAME_VERSION.."_EC618"..ver..".soc")
            os.rm(OUT_PATH.."/pack")
        else 
            os.cp("$(projectdir)/project/luatos/pack/info.json", OUT_PATH)
            os.cp("./PLAT/device/target/board/ec618_0h00/common/inc/mem_map.h", OUT_PATH)
            os.exec(path7z.." a -mx9 "..USER_PROJECT_NAME.."_ec618.7z "..OUT_PATH.."/* -r")
            os.mv(USER_PROJECT_NAME.."_ec618.7z", OUT_PATH.."/"..USER_PROJECT_NAME.."_ec618.soc")
        end

        -- 计算差分包大小, 需要把老的binpkg放在根目录,且命名为 $项目名称.binpkg
        if os.exists(USER_PROJECT_NAME .. ".binpkg") then
            os.cp("./PLAT/tools/fcelf.exe", "tools/dtools/dep/fcelf.exe")
            os.cp(OUT_PATH.."/"..USER_PROJECT_NAME..".binpkg", "tools/dtools/new.binpkg")
            os.cp(USER_PROJECT_NAME .. ".binpkg", "tools/dtools/old.binpkg")
            os.exec("tools\\dtools\\run.bat BINPKG delta.par " .. USER_PROJECT_NAME .. ".binpkg new.binpkg")
        end
	end)
target_end()
