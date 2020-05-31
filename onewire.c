#include <stdint.h>


typedef enum {
    SUCCESS = 0, 
    ERROR = -1
} Wire_CommStatus;


void wire_pin_setup() {
}


uint8_t wire_presence() {
    return 0;
}

void wire_tx_byte(uint8_t data) {
}

uint8_t wire_read_byte() {
    return 0;
}

uint8_t wire_crc(uint8_t *data, uint32_t length) {
    return 0;
}
