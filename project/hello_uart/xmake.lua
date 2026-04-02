local TARGET_NAME = "hello_uart"
local LIB_DIR = "$(buildir)/" .. TARGET_NAME .. "/"
local LIB_NAME = "lib" .. TARGET_NAME .. ".a "

target(TARGET_NAME)
    set_kind("static")
    set_targetdir(LIB_DIR)

    add_includedirs("./inc", {public = true})
    add_files("./src/*.c|bsp_custom.c", {public = true})

    LIB_USER = LIB_USER .. SDK_TOP .. LIB_DIR .. LIB_NAME .. " "
target_end()
