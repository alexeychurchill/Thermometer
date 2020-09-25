CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CMSIS_SRC := $(shell find CMSIS -name '*.c')
CMSIS_SRC_AS := $(shell find CMSIS -name '*.s')
CMSIS_INC := $(addprefix -I, $(dir $(shell find CMSIS -name '*.h')))

INC = -ICMSIS/Device/ST/STM32F1xx/Include
INC += -ICMSIS/Core/Include

LDSCRIPT = CMSIS/Device/ST/STM32F1xx/Source/gcc/linker/STM32F103XB_FLASH.ld

CFLAGS = -g -c -O0 -Wall # -nostdlib
CFLAGS += -mcpu=cortex-m3 -mlittle-endian -mthumb -mthumb-interwork 
CFLAGS += -fsingle-precision-constant -Wdouble-promotion 

LDFLAGS = --script $(LDSCRIPT)

MCUMODEL = -DSTM32F103xB

FLASH_ADDRESS = 0x08000000

DIR_BUILD = ./build
DIR_OBJ = $(DIR_BUILD)/obj

TARGET_NAME = therm_fw
TARGET_ELF = $(DIR_BUILD)/$(TARGET_NAME).elf
TARGET_BIN = $(DIR_BUILD)/$(TARGET_NAME).bin

SRC = $(wildcard *.c) 
SRC += $(CMSIS_SRC)
SRC += onewire_stm32.c
OBJ = $(addprefix $(DIR_OBJ)/, $(SRC:%.c=%.o))
OBJ += $(addprefix $(DIR_OBJ)/, $(CMSIS_SRC_AS:%.s=%.o))


.PHONY: clean all

$(DIR_OBJ)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INC) $(MCUMODEL) $(CFLAGS) $< -o $@

$(DIR_OBJ)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(INC) $(MCUMODEL) $(CFLAGS) $< -o $@

$(TARGET_ELF): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^
	$(SIZE) $@
	
build: $(TARGET_ELF)
	$(OBJCOPY) -O binary $(TARGET_ELF) $(TARGET_BIN)

flash:
	st-flash write $(TARGET_BIN) $(FLASH_ADDRESS)

clean: 
	rm -rf ./build

all: build
