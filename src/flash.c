#include "flash.h"
#include "stm32f1xx.h"

#define FLASH_WAIT_DONE() while ((FLASH->SR & FLASH_SR_BSY) || !(FLASH->SR & FLASH_SR_EOP))
#define FLASH_WAIT_BUSY() while (FLASH->SR & FLASH_SR_BSY)
#define LSHW(value) ((uint16_t) ((value) & 0x0000FFFFu))
#define MSHW(value) ((uint16_t) (((value) & 0xFFFF0000u) >> 16u))

void flash_unlock() {
    if (!(FLASH->CR & FLASH_CR_LOCK)) {
        return;
    }

    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
}

void flash_lock() {
    FLASH->CR |= FLASH_CR_LOCK;
}

void flash_erase(uint32_t page_address) {
    if (FLASH->CR & FLASH_CR_LOCK) {
        return;
    }

    FLASH_WAIT_BUSY();

    // Reset these flags as they can be present after previous operation!
    // For example, if you just flashed the device.
    FLASH->CR &= ~FLASH_CR_PG;
    FLASH->CR &= ~FLASH_CR_MER;
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = page_address;

    FLASH->CR |= FLASH_CR_STRT;
    FLASH_WAIT_DONE();
}

bool flash_write(uint32_t address, const uint32_t *data, uint32_t size) {
    volatile uint16_t *mem = (uint16_t*) address;

    FLASH->CR &= ~FLASH_CR_MER;
    FLASH->CR &= ~FLASH_CR_PER;
    FLASH->CR |= FLASH_CR_PG;

    for (uint32_t index = 0u; index < size; index++) {
        *(mem + index) = LSHW(data[index]);
        FLASH_WAIT_DONE();

        *(mem + index + 1u) = MSHW(data[index]);
        FLASH_WAIT_DONE();
    }

    for (uint32_t index = 0u; index < size; index++) {
        if (*(mem + index) != LSHW(data[index])) {
            return false;
        }

        if (*(mem + index + 1u) != MSHW(data[index])) {
            return false;
        }
    }

    return true;
}
