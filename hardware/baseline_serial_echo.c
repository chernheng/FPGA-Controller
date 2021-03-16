/* Minimal example for testing serial */

#include "system.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_jtag_uart.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include <stdlib.h>
#include "sys/alt_stdio.h"
#include "alt_types.h"
#include <unistd.h>
#include <stdio.h>

int main(){
    while (1){
        alt_32 xy_dir[2] = {1,2};

        alt_32 nios_to_host_event = alt_getchar();
        // alt_getchar blocks

        alt_u32 send_arr[4] = {
            xy_dir[0],  // x dir {-4,4}
            xy_dir[1],  // y dir {-4,4}
            nios_to_host_event, // enum 0-??
            '\n'        // packet delimiter (helps with readline())
		};

        write(1, send_arr, 13);
    }
}