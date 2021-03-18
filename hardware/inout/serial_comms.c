/* Example for testing Serial
- '{' is the starting character.
- '}' is the delimiting character.
- In between, we have hex representations of:
    > i (echo back recieved character)
    > j (number of successful loops)
    > k (constant 1)
 */
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
	int i, j=0, k;	// Dummy data to send
    while (1){
    	printf("{");
        IOWR(LED_BASE, 0, 0b1);
        // 1st LED lights up when Waiting for character
    	
        char data_in = alt_getchar();
    	if (data_in != -1){
        	IOWR(LED_BASE, 0, 0b10);
            // 2nd LED lights up when Character obtained
            i = (int)data_in;
            j++;
            k = 1;
            printf("%x %x %x}", i, j, k);
            IOWR(LED_BASE, 0, 0b0);	    // Character printed, clear LED flag for now
    	} else {
    		IOWR(LED_BASE, 0, 0b100);
            // 3rd LED lights up when EOF character obtained from stream (err)
    	}
    }
}
