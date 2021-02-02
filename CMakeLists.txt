cmake_minimum_required(VERSION 3.18.2)

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

add_compile_definitions(STM32F103xB)
set(FLASH_ADDRESS 0x08000000)

set(LD_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Device/ST/STM32F1xx/Source/gcc/linker/STM32F103XB_FLASH.ld)
set(CMAKE_EXE_LINKER_FLAGS "-T\"${LD_SCRIPT}\"")

include_directories(ui)
include_directories(ui/fonts)
include_directories(.)
include_directories(CMSIS/Core/Include)
include_directories(CMSIS/Device/ST/STM32F1xx/Include)

set(
        SOURCES
        CMSIS/Device/ST/STM32F1xx/Source/gcc/startup_stm32f103xb.s
        CMSIS/Device/ST/STM32F1xx/Source/system_stm32f1xx.c
        display.c
        ds18b20.c
        i2c.c
        main.c
        onewire_stm32.c
        poll_timer.c
        temp_sensor_dispatcher.c
        uart.c
        buttons.c
        ui/ui_screen_temp.c
)

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
