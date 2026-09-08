#ifndef _BLINKY_H_
#define  _BLINKY_H_

#include <stdint.h>
#include <stdbool.h>

struct blinky_cfg {
    uint32_t dout_idx;
};

// Main functions
int32_t blinky_cfg_get_cfg(struct blinky_cfg* cfg);
int32_t blinky_init(struct blinky_cfg* cfg);
int32_t blinky_start(void);


// Change status directly
int32_t blinky_set_state(int32_t on);

#endif