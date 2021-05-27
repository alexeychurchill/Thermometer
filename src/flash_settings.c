#include "flash_settings.h"
#include "flash.h"

#define FLASH_SETTINGS_PAGE                         0x0801FC00u
#define FLASH_SETTINGS_SIZE                         0x100u
#define FLASH_SETTINGS_INDEX_SLEEP                  0u
#define FLASH_SETTINGS_INDEX_MEASURE_PERIOD          0u

typedef uint32_t setting_t;

static setting_t s_buffer[FLASH_SETTINGS_SIZE];

static inline uint32_t setting_value(uint32_t index) {
    return *((volatile setting_t*) (FLASH_SETTINGS_PAGE + index * sizeof(setting_t)));
}

static void read_to_buffer() {
    for (uint32_t index = 0x0u; index < FLASH_SETTINGS_SIZE; index++) {
        s_buffer[index] = setting_value(index);
    }
}

static bool flash_write_value(uint32_t index, uint32_t value) {
    read_to_buffer();
    s_buffer[index] = value;

    flash_unlock();
    flash_erase(FLASH_SETTINGS_PAGE);
    bool result = flash_write(FLASH_SETTINGS_PAGE, s_buffer, FLASH_SETTINGS_SIZE);
    flash_lock();

    return result;
}

void settings_init() {
    for (uint32_t index = 0u; index < FLASH_SETTINGS_SIZE; index++) {
        s_buffer[index] = 0xFF;
    }
}

uint32_t settings_get_sleep() {
    return setting_value(FLASH_SETTINGS_INDEX_SLEEP);
}

bool settings_set_sleep(uint32_t value) {
    return flash_write_value(FLASH_SETTINGS_INDEX_SLEEP, value);
}

uint32_t settings_get_measure_period() {
    return setting_value(FLASH_SETTINGS_INDEX_MEASURE_PERIOD);
}

bool settings_set_measure_period(uint32_t value) {
    return flash_write_value(FLASH_SETTINGS_INDEX_MEASURE_PERIOD, value);
}
