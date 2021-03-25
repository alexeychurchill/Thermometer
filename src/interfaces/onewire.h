#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdbool.h>

typedef enum OwCmd {
    OW_CMD_SEARCH_ROM   = 0xF0,
    OW_CMD_READ_ROM     = 0x33,
    OW_CMD_MATCH_ROM    = 0x55,
    OW_CMD_SKIP_ROM     = 0xCC,
    OW_CMD_ALARM_SEARCH = 0xEC
} OwCmd_t;

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
    void (*send_reset)();
    void (*put_tx_buffer)(const uint8_t*, uint32_t, bool);
    void (*get_rx_buffer)(uint8_t*, uint32_t);
    void (*start_rxtx)();
} OwBusLine_t;

#endif //ONEWIRE_H
