#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdbool.h>

typedef enum OwError {
    OW_ERROR_NONE,
    OW_ERROR_NO_DEVICES,
    OW_ERROR_PRESENCE_WRONG_LENGTH
} OwError_t;

typedef enum OwOperation {
    OW_OP_NONE,
    OW_OP_RESET,
    OW_OP_RXTX
} OwOperation_t;

typedef struct OwBusLine {
    bool (*is_busy)();
    OwError_t (*get_error)();
    OwOperation_t (*get_operation)();
    void (*put_tx_buffer)(const uint8_t*, uint32_t, bool);
    void (*get_rx_buffer)(uint8_t*, uint32_t);
    void (*start_rxtx)(bool);
} OwBusLine_t;

#endif //ONEWIRE_H
