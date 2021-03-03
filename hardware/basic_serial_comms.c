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

/* FUNCTION DECLARATIONS */

//alt_u8 sev_seg(char in);
alt_u8 sev_seg(int num);

/* END FUNCTION DECLARATIONS */
/* VARIABLE DEFINITIONS */

// Convenient way to get current state
enum dev_state{
		FILT_ON,
		FILT_OFF,
		COEFF_UPDATE
};

alt_u8 dev_state = FILT_OFF;
alt_u32 sleep_time = 10000;

/* END VARIABLE DEFINITIONS */

int main()
{ 
//	alt_32 x_read,y_read,z_read;	// For some reason the driver expects a 32-bit value
	alt_32 send_arr[4] = {0,0,0, '\n'};
	char newline = '\n';
	alt_up_accelerometer_spi_dev * acc_dev;
	acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
	if (acc_dev == NULL) { // if return 1, check if the spi ip name is "accelerometer_spi"
		alt_printf("[FATAL] No accelerometer device available\n");
		return 1;
	}

	alt_printf("[INFO] Starting filtering.\n");

  /* Event loop never exits. */
  while (1){
//	  switch (filt_state) {
//			case FILT_OFF:
//				break;
//		}
	  alt_u8 rstatusx = alt_up_accelerometer_spi_read_x_axis(acc_dev, &send_arr[0]);
	  alt_u8 rstatusy = alt_up_accelerometer_spi_read_y_axis(acc_dev, &send_arr[1]);
	  alt_u8 rstatusz = alt_up_accelerometer_spi_read_z_axis(acc_dev, &send_arr[2]);
	  alt_u8 status = 0;
	  if (~rstatusx && ~rstatusy && ~rstatusz){
		  status = write( 1, send_arr, 13 );	// Writing bytes to host is here
	  }
	  IOWR(HEX0_BASE, 0, sev_seg(status));
	  IOWR(HEX1_BASE, 0, sev_seg(99));	// Not currently in use, shows blank
	  IOWR(HEX2_BASE, 0, sev_seg(99));	// Not currently in use
	  // If these don't show '0' there is an error
	  IOWR(HEX3_BASE, 0, sev_seg(rstatusx));
	  IOWR(HEX4_BASE, 0, sev_seg(rstatusy));
	  IOWR(HEX5_BASE, 0, sev_seg(rstatusz));

	  usleep(sleep_time);	// Don't print so fast!!
  }

  return 0;
}

// Helper function turns numbers into 7-segment info
alt_u8 sev_seg(int num){
	switch(num){
		case 0:
			return 0b1000000;
		case 1:
			return 0b1111001;
		case 2:
			return 0b0100100;
		case 3:
			return 0b0110000;
		case 4:
			return 0b0011001;
		case 5:
			return 0b0010010;
		case 6:
			return 0b0000010;
		case 7:
			return 0b1111000;
		case 8:
			return 0b0000000;
		case 9:
			return 0b0010000;
		case 10:
			return 0b0001000;	// 0xA
		case 11:
			return 0b0000011;	// 0xB
		case 12:
			return 0b1000110;	// 0xC
		case 13:
			return 0b0100001;	// 0xD
		case 14:
			return 0b0000110;	// 0xE
		case 15:
			return 0b0001110;	// 0xF
		case 99:
			return 0b1111111;	// off
		default:
			return 0b0010000;	// lowercase 'e' error
	}
}

 /*
  * Todo:
  * Modify the state
  * add hardware FIR filtering
  */