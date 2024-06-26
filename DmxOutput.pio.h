// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// --------- //
// DmxOutput //
// --------- //

#define DmxOutput_wrap_target 3
#define DmxOutput_wrap 6

static const uint16_t DmxOutput_program_instructions[] = {
    0xf035, //  0: set    x, 21           side 0     
    0x0741, //  1: jmp    x--, 1                 [7] 
    0xbf42, //  2: nop                    side 1 [7] 
            //     .wrap_target
    0x9fa0, //  3: pull   block           side 1 [7] 
    0xf327, //  4: set    x, 7            side 0 [3] 
    0x6001, //  5: out    pins, 1                    
    0x0245, //  6: jmp    x--, 5                 [2] 
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program DmxOutput_program = {
    .instructions = DmxOutput_program_instructions,
    .length = 7,
    .origin = -1,
};

static inline pio_sm_config DmxOutput_program_get_default_config( uint offset, uint8_t pin ){
    pio_sm_config cc = pio_get_default_sm_config();
    sm_config_set_wrap(&cc, offset + DmxOutput_wrap_target, offset + DmxOutput_wrap);
    sm_config_set_sideset(&cc, 2, true, false);
    // Setup the side-set pins for the PIO state machine
    sm_config_set_out_pins(&cc, pin, 1);
    sm_config_set_sideset_pins(&cc, pin);
    return cc;
}
#endif

