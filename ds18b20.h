#ifndef THERMOMETER_DS18B20_H
#define THERMOMETER_DS18B20_H

#include <stdint.h>
#include <stdbool.h>
#include "onewire.h"

#define DS18B20_CONVERT_T       0x44
#define DS18B20_READ_SCRATCHPAD 0xBE


typedef enum DS18B20Res {
    DS_T_RES_9_BIT      = 0x0,  //  R1 = 0  R0 = 0
    DS_T_RES_10_BIT     = 0x1,  //  R1 = 0  R0 = 1
    DS_T_RES_11_BIT     = 0x2,  //  R1 = 1  R0 = 0
    DS_T_RES_12_BIT     = 0x3   //  R1 = 1  R0 = 1
} DS18B20Res_t;

typedef struct DS18B20Sensor {
    uint16_t temp;

    uint8_t temp_hi;
    uint8_t temp_lo;
    uint8_t config;
} DS18B20Sensor_t;

bool ds18b20_convert_t_send(const OwBusLine_t *line);

bool ds18b20_send_read_scratchpad(const OwBusLine_t *line, DS18B20Sensor_t *sensor);

void ds18b20_parse_scratchpad_data(const OwBusLine_t *line, DS18B20Sensor_t *sensor);

// Operations for convenient work with DS18B20 data and HMI

/**
 * Gets absolute value of the temperature integer part
 * @param temp temperature sent by DS18B20
 * @return
 */
uint8_t ds18b20_hmi_get_temp_int_absolute(uint16_t temp);

/**
 * Get temp's sign
 * @param temp temperature sent by DS18B20
 * @return false if '+', true if '-'
 */
bool ds18b20_hmi_get_temp_sign(uint16_t temp);

/**
 * Gets absolute value of the temperature fractional part, multiplied by 10000
 * @param temp temperature sent by DS18B20
 * @return
 */
uint8_t ds18b20_hmi_get_temp_frac10000(uint16_t temp);

#endif //THERMOMETER_DS18B20_H
