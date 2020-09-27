#include "ds18b20.h"

#include <stddef.h>
#include "utils.h"

#define DS18B20_TEMP_BIT_OFFSET         4
#define DS18B20_TEMP_MASK               (((int16_t) 0x7F) << DS18B20_TEMP_BIT_OFFSET)
#define DS18B20_TEMP_SIGN_BIT_OFFSET    11
#define DS18B20_TEMP_SIGN_MASK          (((int16_t) 0x1F) << DS18B20_TEMP_SIGN_BIT_OFFSET)
#define DS18B20_SCRATCHPAD_LEN          9
#define DS18B20_FRAC_MUL                625u
#define DS18B20_FRAC_MASK               0xFu


static FORCE_INLINE bool ds18b20_lock(DS18B20Sensor_t *sensor) {
    if (sensor -> busy) {
        return false;
    }

    sensor -> busy = true;
    return true;
}

static FORCE_INLINE void ds18b20_unlock(DS18B20Sensor_t *sensor) {
    sensor -> busy = false;
}

static void ds18b20_convert_t_send_wait__(DS18B20Sensor_t *sensor);

static void ds18b20_convert_t_read_wait__(DS18B20Sensor_t *sensor) {
    const OwBusLine_t *line = sensor -> ow_bus;
    if (line -> is_busy()) {
        return;
    }

    uint8_t buf;
    line -> get_rx_buffer(&buf, 1);
    if (buf == 0xFF) { // Done
        sensor -> next_step = NULL;
        ds18b20_unlock(sensor);
        return;
    }

    ds18b20_convert_t_send_wait__(sensor);
}

static void ds18b20_convert_t_send_wait__(DS18B20Sensor_t *sensor) {
    const OwBusLine_t *line = sensor -> ow_bus;
    if (line -> is_busy()) {
        return;
    }

    uint8_t buf = 0xFF;
    line -> put_tx_buffer(&buf, 1, false);
    line -> start_rxtx();

    sensor -> next_step = ds18b20_convert_t_read_wait__;
}

static void ds18b20_convert_t_send_cmd__(DS18B20Sensor_t *sensor) {
    const OwBusLine_t *line = sensor -> ow_bus;
    if (line -> is_busy()) {
        return;
    }

    uint8_t buf[] = { OW_CMD_SKIP_ROM, DS18B20_CMD_CONVERT_T };
    line -> put_tx_buffer(buf, 2, false);
    line -> start_rxtx();

    sensor -> next_step = ds18b20_convert_t_send_wait__;
}

static void ds18b20_convert_t_reset__(DS18B20Sensor_t *sensor) {
    const OwBusLine_t *line = sensor -> ow_bus;
    if (line -> is_busy()) {
        return;
    }

    line -> send_reset();

    sensor -> next_step = ds18b20_convert_t_send_cmd__;
}

static void ds18b20_read_scratchpad_parse_data__(DS18B20Sensor_t *sensor) {
    if ((sensor -> ow_bus) -> is_busy()) {
        return;
    }

    uint8_t rx_buf[11];
    (sensor -> ow_bus) -> get_rx_buffer(rx_buf, 11);

    sensor -> temp = (rx_buf[2] << 8u) | (rx_buf[3]);
    sensor -> temp_hi = rx_buf[4];
    sensor -> temp_lo = rx_buf[5];
    sensor -> config = rx_buf[6];

    sensor -> next_step = NULL;
    ds18b20_unlock(sensor);
}

static void ds18b20_read_scratchpad_send_cmd__(DS18B20Sensor_t *sensor) {
    if ((sensor -> ow_bus) -> is_busy()) {
        return;
    }

    const OwBusLine_t *line = sensor -> ow_bus;
    uint8_t tx_buf[] = { OW_CMD_SKIP_ROM, DS18B20_CMD_READ_SCRATCHPAD };
    line -> put_tx_buffer(tx_buf, 2, false);
    line -> put_tx_buffer(NULL, DS18B20_SCRATCHPAD_LEN, true);
    line -> start_rxtx();
    sensor -> next_step = ds18b20_read_scratchpad_parse_data__;
}

static void ds18b20_read_scratchpad_reset__(DS18B20Sensor_t *sensor) {
    const OwBusLine_t *line = sensor -> ow_bus;
    if (line -> is_busy()) {
        return;
    }

    sensor -> next_step = ds18b20_read_scratchpad_send_cmd__;
    line -> send_reset();
}

void ds18b20_init(const OwBusLine_t *ow_bus, DS18B20Sensor_t *sensor) {
    sensor -> ow_bus = ow_bus;

    sensor -> has_data = false;
    sensor -> busy = false;

    sensor -> temp = 0;
    sensor -> temp_lo = 0;
    sensor -> temp_hi = 0;
    sensor -> config = 0;

    sensor -> next_step = NULL;
}

void ds18b20_dispatch(DS18B20Sensor_t *sensor) {
    DS18B20Func_t step = sensor -> next_step;
    if (step == NULL) {
        return;
    }

    step(sensor);
}

void ds18b20_convert_t(DS18B20Sensor_t *sensor) {
    if (!ds18b20_lock(sensor)) {
        return;
    }

    if ((sensor -> ow_bus) -> is_busy()) {
        return;
    }

    ds18b20_convert_t_reset__(sensor);
}

void ds18b20_send_read_scratchpad(DS18B20Sensor_t *sensor) {
    if (!ds18b20_lock(sensor)) {
        return;
    }

    if ((sensor -> ow_bus) -> is_busy()) {
        return;
    }

    ds18b20_read_scratchpad_reset__(sensor);
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
