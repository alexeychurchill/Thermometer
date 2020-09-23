#include "ds18b20.h"

#include <stddef.h>
#include "onewire_rom_cmd.h"
#include "utils.h"

#define DS18B20_TEMP_BIT_OFFSET         4
#define DS18B20_TEMP_MASK               (((int16_t) 0x7F) << DS18B20_TEMP_BIT_OFFSET)
#define DS18B20_TEMP_SIGN_BIT_OFFSET    11
#define DS18B20_TEMP_SIGN_MASK          (((int16_t) 0x1F) << DS18B20_TEMP_SIGN_BIT_OFFSET)
#define DS18B20_SCRATCHPAD_LEN          9
#define DS18B20_FRAC_MUL                625u
#define DS18B20_FRAC_MASK               0xFu


bool ds18b20_convert_t_send(const OwBusLine_t *line) {
    if (line -> is_busy()) {
        return false;
    }

    uint8_t tx_buf[] = { OW_SKIP_ROM, DS18B20_CONVERT_T };
    line -> put_tx_buffer(tx_buf, 2, false);
    line -> start_rxtx(true);
    return true;
}

bool ds18b20_send_read_scratchpad(const OwBusLine_t *line, DS18B20Sensor_t *sensor) {
    // TODO: Implement ROM handling
    if (line -> is_busy()) {
        return false;
    }

    uint8_t tx_buf[] = { OW_SKIP_ROM, DS18B20_READ_SCRATCHPAD };
    line -> put_tx_buffer(tx_buf, 2, false);
    line -> put_tx_buffer(NULL, DS18B20_SCRATCHPAD_LEN, true);
    line -> start_rxtx(false);
    return true;
}

void ds18b20_parse_scratchpad_data(const OwBusLine_t *line, DS18B20Sensor_t *sensor) {
    uint8_t rx_buf[11];
    line -> get_rx_buffer(rx_buf, 11);

    sensor -> temp = (rx_buf[2] << 8u) | (rx_buf[3]);
    sensor -> temp_hi = rx_buf[4];
    sensor -> temp_lo = rx_buf[5];
    sensor -> config = rx_buf[6];
}

static FORCE_INLINE uint16_t ds18b20_hmi_temp_abs(uint16_t temp) {
    int16_t t_signed = (int16_t) temp;
    return (uint16_t) (t_signed < 0 ? -t_signed : t_signed);
}

uint8_t ds18b20_hmi_get_temp_int_absolute(uint16_t temp) {
    uint16_t temp_abs = ds18b20_hmi_temp_abs(temp);
    return (temp_abs & DS18B20_TEMP_MASK) >> DS18B20_TEMP_BIT_OFFSET;
}

bool ds18b20_hmi_get_temp_sign(uint16_t temp) {
    return (temp & DS18B20_TEMP_SIGN_MASK) > 0;
}

uint8_t ds18b20_hmi_get_temp_frac10000(uint16_t temp) {
    uint16_t temp_abs = ds18b20_hmi_temp_abs(temp);
    uint16_t temp_frac = (temp_abs & DS18B20_FRAC_MASK);
    return temp_frac * DS18B20_FRAC_MUL;
}
