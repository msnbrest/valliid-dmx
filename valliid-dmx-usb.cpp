/* details :
https://cec-code-lab.aps.edu/robotics/resources/pico-c-api/group__sm__config.html#gaf543422206a8dbdc2efea85818dd650e

by youcode 20230418 edited by msnbrest
*/

#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "DmxOutput.pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "tusb.h"

#define DMX_SM_FREQ 1000000

uint dmx_0_prgm_offset;
uint dmx_0_pin;
uint dmx_0_sm;
PIO  dmx_0_pio;
uint dmx_0_dma;

uint16_t dmx_trame_length = 513;
uint8_t dmx_trame[513] = {0};
uint8_t led_ok= 0;



int dmx_begin(){

    const uint8_t pin= 0;

    // Attempt load DMX PIO assembly program into PIO program memory
    if( !pio_can_add_program(pio0, &DmxOutput_program) ){
        return 2;
    }
    uint prgm_offset = pio_add_program(pio0, &DmxOutput_program);

    // Attempt to claim an unused State Machine into the PIO program memory
    int sm = pio_claim_unused_sm(pio0, false);
    if( sm == -1 ){ return 1; }

    // Set this pin's GPIO function (connect PIO to the pad)
    pio_sm_set_pins_with_mask(pio0, sm, 1u << pin, 1u << pin);
    pio_sm_set_pindirs_with_mask(pio0, sm, 1u << pin, 1u << pin);
    pio_gpio_init(pio0, pin);

    // Generate the default PIO state machine config provided by pioasm
    pio_sm_config sm_conf = DmxOutput_program_get_default_config(prgm_offset,pin);

    // Setup the clock divider to run the state machine at exactly 1MHz
    uint clk_div = clock_get_hz(clk_sys) / DMX_SM_FREQ;
    sm_config_set_clkdiv(&sm_conf, clk_div);

    // Load our configuration, jump to the start of the program and run the State Machine
    pio_sm_init(pio0, sm, prgm_offset, &sm_conf);
    pio_sm_set_enabled(pio0, sm, true);

    // Claim an unused DMA channel.
    // The channel is kept througout the lifetime of the DMX source
    int dma = dma_claim_unused_channel(false);

    if (dma == -1)
        return 3;

    // Get the default DMA config for our claimed channel
    dma_channel_config dma_conf = dma_channel_get_default_config(dma);

    // Set the DMA to move one byte per DREQ signal
    channel_config_set_transfer_data_size(&dma_conf, DMA_SIZE_8);

    // Setup the DREQ so that the DMA only moves data when there
    // is available room in the TXF buffer of our PIO state machine
    channel_config_set_dreq(&dma_conf, pio_get_dreq(pio0, sm, true));

    // Setup the DMA to write to the TXF buffer of the PIO state machine
    dma_channel_set_write_addr(dma, &pio0->txf[sm], false);

    // Apply the config
    dma_channel_set_config(dma, &dma_conf, false);

    // Set member values of C++ class
    dmx_0_prgm_offset = prgm_offset;
    dmx_0_pio = pio0;
    dmx_0_sm = sm;
    dmx_0_pin = pin;
    dmx_0_dma = dma;

    return 0;
}



void dmx_send(){
    // Temporarily disable the PIO state machine
    pio_sm_set_enabled(dmx_0_pio, dmx_0_sm, false);

    // Reset the PIO state machine to a consistent state. Clear the buffers and registers
    pio_sm_restart(dmx_0_pio, dmx_0_sm);

    // Start the DMX PIO program from the beginning
    pio_sm_exec(dmx_0_pio, dmx_0_sm, pio_encode_jmp(dmx_0_prgm_offset));

    // Restart the PIO state machinge
    pio_sm_set_enabled(dmx_0_pio, dmx_0_sm, true);

    // Start the DMA transfer
    dma_channel_transfer_from_buffer_now(dmx_0_dma, dmx_trame, dmx_trame_length);
}



bool dmx_busy(){

    if( dma_channel_is_busy(dmx_0_dma) ){ return true; }
    return !pio_sm_is_tx_fifo_empty(dmx_0_pio, dmx_0_sm);
}



void core1_main() {
    tusb_init();

    uint8_t iv= 0; // dmw_set_val
    uint8_t ih= 1; // dmx_put_H
    uint16_t vv; // dmx_data_v
    uint16_t kk; // dmx_data_k

    while(1){
        // Read characters from USB serial
        int c= tud_cdc_n_read_char(0);
        if(c >= 0){

            if(c==25){ iv= 0; ih= 1; led_ok= 1; continue; }// reset

            if( iv ){ // read value
                    if( ih ){ vv= c*25; }
                    else{ vv+= c; iv= 0; }
            }else{ // read channel
                if( ih ){ kk= c*25; }else{
                    kk+= c;
                    if(kk>511){ // fonction, not dmx
                        if( kk==624 ){ iv= 1; } // next is a value
                    }else if( vv<256 ){ // good dmx value
                        // exemple, j'arrive avec la valeur 200 pour le channel id 511
                        // alors trame= 513 size contenant 0 + 512valeurs, trame[out id 512] = val
                        if( kk>511 ){ return; }
                        if( kk>=dmx_trame_length ){ dmx_trame_length= kk+2; }
                        dmx_trame[kk+1]= vv;
                    }
                }
            }
            ih= 1-ih;
        }
    }
}



int main(){

    stdio_init_all();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while( !stdio_usb_connected() || !stdio_usb_init() ){
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(420);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(80);
    }

    if( dmx_begin()>0 ){ // err
		while(1){
			gpio_put(PICO_DEFAULT_LED_PIN, 1);
			sleep_ms(100);
			gpio_put(PICO_DEFAULT_LED_PIN, 0);
			sleep_ms(40);
		}
    }

    multicore_launch_core1(core1_main);

    while(1){ // rock'n'roll
        if( !dmx_busy() ){
			// ready send
            dmx_send();
			// ok
            gpio_put(PICO_DEFAULT_LED_PIN, led_ok );
			led_ok= 0;

            sleep_ms(22); // 44hz == 22.7ms
        }
    }
}
