#include "temp_sensor_dispatcher.h"
#include "config.h"
#include "drivers/ds18b20.h"
#include "poll_timer.h"


typedef enum ThermometerState {
    TS_IDLE,
    TS_TEMP_MEASURE,
    TS_TEMP_READ
} ThermState_t;

static ThermState_t state;
static DS18B20Sensor_t sensor;

static void tsd_handle_state_idle() {
    if (poll_timer_is_running()) {
        return;
    }

    state = TS_TEMP_MEASURE;
    ds18b20_convert_t(&sensor);
}

static void tsd_handle_state_measure() {
    state = TS_TEMP_READ;
    ds18b20_send_read_scratchpad(&sensor);
}

static void tsd_handle_state_read() {
    state = TS_IDLE;
    poll_timer_start(TSD_MEASURE_PERIOD_MS);
}

static void (*therm_state_table[])() = {
        [TS_IDLE]           = tsd_handle_state_idle,
        [TS_TEMP_MEASURE]   = tsd_handle_state_measure,
        [TS_TEMP_READ]      = tsd_handle_state_read
};

void tsd_init(const OwBusLine_t *line) {
    state = TS_IDLE;
    ds18b20_init(line, &sensor);
}

void tsd_dispatch_state() {
    ds18b20_dispatch(&sensor);
    if (ds18b20_is_busy(&sensor)) {
        return;
    }

    therm_state_table[state]();
}

int32_t tsd_get_t() {
    int32_t temp_int = ((int32_t) ds18b20_get_temp_abs_int_part(&sensor));
    bool temp_sign = ds18b20_get_temp_sign(&sensor);
    return temp_sign ? -temp_int : temp_int;
}
