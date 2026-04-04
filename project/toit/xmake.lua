local TARGET_NAME = "toit"
local LIB_DIR = "$(buildir)/" .. TARGET_NAME .. "/"
local LIB_NAME = "lib" .. TARGET_NAME .. ".a "

-- Path to the Toit VM build output (built by cmake/ninja separately).
local TOIT_BUILD = SDK_TOP .. "/../../build/ec618"

target(TARGET_NAME)
    set_kind("static")
    set_targetdir(LIB_DIR)

    add_includedirs("./inc", {public = true})
    add_includedirs(SDK_TOP .. "/PLAT/os/freertos/portable/mem/cmpctmalloc", {public = true})
    add_files("./src/*.c|bsp_custom.c|sys_ro_override.c", {public = true})

    -- Link the project's own library.
    LIB_USER = LIB_USER .. SDK_TOP .. "/" .. LIB_DIR .. LIB_NAME .. " "

    -- Link the Toit VM library and mbedTLS libraries.
    LIB_USER = LIB_USER .. TOIT_BUILD .. "/src/libtoit_vm.a "
    LIB_USER = LIB_USER .. TOIT_BUILD .. "/mbedtls/library/libmbedtls.a "
    LIB_USER = LIB_USER .. TOIT_BUILD .. "/mbedtls/library/libmbedx509.a "
    LIB_USER = LIB_USER .. TOIT_BUILD .. "/mbedtls/library/libmbedcrypto.a "
target_end()
