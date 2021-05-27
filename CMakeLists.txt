cmake_minimum_required(VERSION 3.17.5)

project(Thermometer C ASM)


set(CMAKE_CROSSCOMPILING 1)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_FLAGS "\
-g -O0 -Wall -nostdlib \
-mcpu=cortex-m3 -mlittle-endian -mthumb -mthumb-interwork \
-fsingle-precision-constant -Wdouble-promotion\
")

# TODO: Add release/debug flags
# set(CMAKE_C_FLAGS_RELEASE "")
# set(CMAKE_C_FLAGS_DEBUG "")

# Fonts generation

set(FONT_GEN_PATH fonts_src)

add_custom_command(
        OUTPUT
            ${PROJECT_BINARY_DIR}/${FONT_GEN_PATH}/bebas_16.c
            ${PROJECT_BINARY_DIR}/${FONT_GEN_PATH}/bebas_16.h
            ${PROJECT_BINARY_DIR}/${FONT_GEN_PATH}/bebas_24.c
            ${PROJECT_BINARY_DIR}/${FONT_GEN_PATH}/bebas_24.h

        USES_TERMINAL

        BYPRODUCTS ${FONT_GEN_PATH}

        COMMAND python3 ${FONTS_GEN_UTIL}
        ${PROJECT_SOURCE_DIR}/res/fonts/AA-BebasNeue.ttf
            -hor
            -fl 1
            -fr 1
            --out_dir ${FONT_GEN_PATH}
            --cname bebas_16
            --glyph_height 16
            --chars А-Я а-я І і Ї ї Ґ ґ Є є 0-9 . , + ! :

        COMMAND python3 ${FONTS_GEN_UTIL}
        ${PROJECT_SOURCE_DIR}/res/fonts/AA-BebasNeue.ttf
            -hor
            -fl 1
            -fr 1
            --out_dir ${FONT_GEN_PATH}
            --cname bebas_24
            --glyph_height 24
            --glyph_width_equal
            --chars 0-9 :
)

set(
        FONT_GEN_SOURCES
            ${PROJECT_BINARY_DIR}/${FONT_GEN_PATH}/bebas_16.c
            ${PROJECT_BINARY_DIR}/${FONT_GEN_PATH}/bebas_24.c
)

# Firmware build process

add_compile_definitions(STM32F103xB)
set(FLASH_ADDRESS 0x08000000)

set(LD_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Device/ST/STM32F1xx/Source/gcc/linker/STM32F103XB_FLASH.ld)
set(CMAKE_EXE_LINKER_FLAGS "-T\"${LD_SCRIPT}\"")

include_directories(src)
include_directories(src/util)
include_directories(src/ui)
include_directories(src/ui/fonts)
include_directories(src/ui/menu)
include_directories(src/ui/mode)
include_directories(src/ui/screens)
include_directories(${PROJECT_BINARY_DIR}/${FONT_GEN_PATH})
include_directories(CMSIS/Core/Include)
include_directories(CMSIS/Device/ST/STM32F1xx/Include)

set(
        SOURCES
        CMSIS/Device/ST/STM32F1xx/Source/gcc/startup_stm32f103xb.s
        CMSIS/Device/ST/STM32F1xx/Source/system_stm32f1xx.c
        src/stm32f1_isr.c
        src/main.c
        src/poll_timer.c
        src/temp_sensor_dispatcher.c
        src/rtc.c
        src/time.c
        src/display.c
        src/flash.c
        src/flash_settings.c
        src/power.c
        src/drivers/ds18b20.c
        src/interfaces/i2c.c
        src/interfaces/onewire_stm32.c
        src/interfaces/uart.c
        src/interfaces/buttons.c
        src/ui/text.c
        src/ui/menu/menu.c
        src/ui/mode/ui_mode_dispatcher.c
        src/ui/screens/scr_common.c
        src/ui/screens/scr_init.c
        src/ui/screens/scr_construction.c
        src/ui/screens/ui_screen_temp.c
        src/ui/screens/scr_menu.c
        src/ui/screens/scr_time.c
        src/ui/screens/scr_set_time.c
        src/ui/screens/scr_settings.c
        src/ui/screens/scr_set_sleep.c
)

list(APPEND SOURCES ${FONT_GEN_SOURCES})

add_executable(${PROJECT_NAME}.elf ${SOURCES})

add_custom_target(${PROJECT_NAME}.bin ALL DEPENDS ${PROJECT_NAME}.elf)
add_custom_command(
        TARGET ${PROJECT_NAME}.bin
        COMMAND ${CMAKE_C_OBJCOPY}
        ARGS -O binary
        ${PROJECT_NAME}.elf
        ${PROJECT_NAME}.bin
)

add_custom_target(
        flash
        DEPENDS ${PROJECT_NAME}.bin
)
add_custom_command(
        TARGET flash
        USES_TERMINAL
        COMMAND st-flash write ${PROJECT_NAME}.bin ${FLASH_ADDRESS}
)
