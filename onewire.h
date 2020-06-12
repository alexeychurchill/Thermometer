#ifndef ONEWIRE_H
#define ONEWIRE_H

typedef enum one_wire_status {
    OW_STS_IDLE, 

    OW_STS_RESET_IN_PROGRESS, 
    OW_STS_RESET_DONE, 

    OW_STS_SEND_IN_PROGRESS,
    OW_STS_SEND_DONE, 

    OW_STS_READ_IN_PROGRESS,  
    OW_STS_READ_DONE
} one_wire_status_t;

typedef enum one_wire_error {
    OW_ERR_NONE = 0, 
    OW_ERR_PRESENCE_TIMEOUT = 100, 
    OW_ERR_PRESENCE_WRONG_TIMING = 200
} one_wire_error_t;

one_wire_status_t ow_status_get();

one_wire_error_t ow_error_get();

void ow_error_reset();

void one_wire_init();

void one_wire_reset();

// void one_wire_put_write_data();

// uint8_t one_wire_get_read_data();

// void one_wire_proceed_read(); 

// void one_wire_proceed_write();

#endif
