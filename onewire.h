#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdbool.h>

typedef enum OwError {
    OW_ERROR_NONE,
    OW_ERROR_NO_DEVICES,
    OW_ERROR_PRESENCE_WRONG_LENGTH,
    OW_ERROR_ILLEGAL_STATE
} OwError_t;

typedef struct OwBusLine {
    bool (*is_busy)();
    OwError_t (*get_error)();
    void (*put_tx_buffer)(const uint8_t*, uint32_t, bool);
    void (*get_rx_buffer)(uint8_t*, uint32_t);
    void (*start_rxtx)(bool);
} OwBusLine_t;

#endif //ONEWIRE_H
