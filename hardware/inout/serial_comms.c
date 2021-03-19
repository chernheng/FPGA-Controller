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

alt_u8 sev_seg(int num);    // Debug

int main(){
	int i, j=0, k;	// Dummy data to send
    while (1){
        IOWR(LED_BASE, 0, 0b1);	// Waiting for character
    	char data_in = alt_getchar();
    	if (data_in != -1){
    		IOWR(HEX0_BASE, 0, sev_seg(data_in&0xF));
    		IOWR(HEX1_BASE, 0, sev_seg(data_in>>4));
    		// Visually see character on sevseg
        	IOWR(LED_BASE, 0, 0b10);	// Character obtained
            i = (int)data_in;
            j++;
            k = 1;
            alt_printf("{%x %x %x}", i, j, k);
            IOWR(LED_BASE, 0, 0b0);	// Character printed
    	} else {
    		IOWR(LED_BASE, 0, 0b100);
    	}
    }
}

// Helper function turns numbers into 7-segment info
alt_u8 sev_seg(int num){
	switch(num){
		case 0:
			return 0b11000000;
		case 1:
			return 0b11111001;
		case 2:
			return 0b10100100;
		case 3:
			return 0b10110000;
		case 4:
			return 0b10011001;
		case 5:
			return 0b10010010;
		case 6:
			return 0b10000010;
		case 7:
			return 0b11111000;
		case 8:
			return 0b10000000;
		case 9:
			return 0b10010000;
		case 10:
			return 0b10001000;	// 0xA
		case 11:
			return 0b10000011;	// 0xB
		case 12:
			return 0b11000110;	// 0xC
		case 13:
			return 0b10100001;	// 0xD
		case 14:
			return 0b10000110;	// 0xE
		case 15:
			return 0b10001110;	// 0xF
		case 16:
			return 0b11110111;	// 0x10
		case 99:
			return 0b11111111;	// off
		default:
			return 0b10010000;	// lowercase 'e' error
	}
}
