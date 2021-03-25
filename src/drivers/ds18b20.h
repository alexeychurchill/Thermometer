#ifndef THERMOMETER_DS18B20_H
#define THERMOMETER_DS18B20_H

#include <stdint.h>
#include <stdbool.h>
#include "interfaces/onewire.h"
#include "src/util/utils.h"

typedef enum DS18B20Cmd {
    DS18B20_CMD_CONVERT_T           = 0x44,
    DS18B20_CMD_READ_SCRATCHPAD     = 0xBE
} DS18B20Cmd_t;

typedef enum DS18B20Res {
    DS_T_RES_9_BIT      = 0x0,  //  R1 = 0  R0 = 0
    DS_T_RES_10_BIT     = 0x1,  //  R1 = 0  R0 = 1
    DS_T_RES_11_BIT     = 0x2,  //  R1 = 1  R0 = 0
    DS_T_RES_12_BIT     = 0x3   //  R1 = 1  R0 = 1
} DS18B20Res_t;

typedef struct DS18B20Sensor DS18B20Sensor_t;

typedef void (*DS18B20Func_t)(DS18B20Sensor_t*);

struct DS18B20Sensor {
    const OwBusLine_t *ow_bus;

    bool has_data;
    volatile bool busy;

    uint16_t temp;

    uint8_t temp_hi;
    uint8_t temp_lo;
    uint8_t config;

    volatile DS18B20Func_t next_step;
};

void ds18b20_init(const OwBusLine_t *ow_bus, DS18B20Sensor_t *sensor);

FORCE_INLINE bool ds18b20_is_busy(const DS18B20Sensor_t *sensor) {
    return sensor -> busy;
}

void ds18b20_dispatch(DS18B20Sensor_t *sensor);

void ds18b20_convert_t(DS18B20Sensor_t *sensor);

void ds18b20_send_read_scratchpad(DS18B20Sensor_t *sensor);

// Operations for convenient work with DS18B20 temp data

/**
 * Get temp's sign
 * @param temp temperature sent by DS18B20
 * @return false if '+', true if '-'
 */
bool ds18b20_get_temp_sign(const DS18B20Sensor_t *sensor);

/**
 * Gets absolute value of the temperature integer part
 * @param temp temperature sent by DS18B20
 * @return
 */
uint8_t ds18b20_get_temp_abs_int_part(const DS18B20Sensor_t *sensor);

/**
 * Gets absolute value of the temperature fractional part, multiplied by 10000
 * @param temp temperature sent by DS18B20
 * @return
 */
uint8_t ds18b20_get_temp_abs_frac_part(const DS18B20Sensor_t *sensor);

#endif //THERMOMETER_DS18B20_H
