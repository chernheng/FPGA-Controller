/*
Fixed version of the code interfaces with FIR filter in an expected way now.
Beforehand there was a bug in sending data to the FIR filter through the FIFOs due
to sign-extension.
*/

#include "altera_avalon_fifo_regs.h"
#include "altera_avalon_fifo_util.h"
#include "altera_avalon_fifo.h"	// For the FIFO (talking to hardware components)

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


/* ####### VARIABLE DEFINITIONS ####### */
#define DEBUG 1					// set to 0 for comms with host PC
#define TX_RATE 2				// in hertz, to host PC
const alt_u32 sleep_time = 1000000/TX_RATE;
const alt_u16 FIR_SHIFT = 19;	// number of bits to shift output FIR data
const alt_32 acc_x_offset = 2;
const alt_32 acc_y_offset = -10;
const alt_32 acc_z_offset = -45;	// Offset for the accelerometer readings

volatile alt_u32* uartDataRegPtr  = (alt_u32*)JTAG_UART_BASE;		// UART Data register
volatile alt_u32* uartCntrlRegPtr = ((alt_u32*)JTAG_UART_BASE+1);	// UART Control register

// Convenient way to get current state
enum dev_state{
		FILT_ON,
		FILT_OFF,
		COEFF_UPDATE
};

enum accel_rate_settings {
	ACCEL_0_78HZ = 0x3,
	ACCEL_1_56HZ = 0x4,
	ACCEL_3_13HZ = 0x5,
	ACCEL_6_25HZ = 0x6,
	ACCEL_12_5HZ = 0x7,
	ACCEL_25HZ = 0x8,
	ACCEL_50HZ = 0x9,
	ACCEL_100HZ = 0xA,
	ACCEL_200HZ = 0xB,
	ACCEL_400HZ = 0xC,
	ACCEL_800HZ = 0xD,
	ACCEL_1600HZ = 0xE,
	ACCEL_3200HZ = 0xF
};

// Codes recieved from the host
enum recieved_event {
	WAITING_ROOM,
	GAME_START,
	GAME_MOVE,
	TASK1,
	TASK2,
	TASK3
};

// Code to send to the host
enum transmitted_event {
	NONE,
	NORMAL,
	TASK_DONE,
	TASK_IN_PROGRESS
};

volatile alt_u32 nios_to_host_event = NONE;			// Var for FPGA to talk to host
volatile alt_u8 host_to_nios_event = 0;				// Var for host to talk to FPGA

alt_u8 accel_samplerate = ACCEL_200HZ;

alt_u8 out_fifo_ienable = 0;	// No input interrupts
alt_u8 in_fifo_ienable = ALTERA_AVALON_FIFO_IENABLE_AF_MSK | ALTERA_AVALON_FIFO_IENABLE_F_MSK;
// almost full and full interrupts only, look at altera_avalon_fifo_regs.h for the rest
alt_u8 in_fifo_af_level = 8;	// Almost-full level

//alt_irq_context irq_context;						// IRQ context handler
volatile alt_up_accelerometer_spi_dev* acc_dev;	// accelerometer file handler
volatile alt_16 acc_data[6];						// 0,1:X, 2,3:Y, 4,5:Z
volatile alt_u8 acc_first = 0;
volatile alt_32 fifo_data[3];						// Data from FIFO (passed through)
alt_8 xy_dir[2];									//

/* ####### END VARIABLE DEFINITIONS ####### */

/* ####### FUNCTION DECLARATIONS ####### */

alt_u8 sev_seg(int num);
int setup_fifo();								// sets up the fifos
int setup_accel(alt_up_accelerometer_spi_dev* dev);
void accel_getdata(void* context, alt_u32 id);	// Read data from accelerometer on interrupt
void fifo_x_getdata(void* context, alt_u32 id);	// Get data from FIFO when it's almost full
void fifo_y_getdata(void* context, alt_u32 id);	// Get data from FIFO when it's almost full
void fifo_z_getdata(void* context, alt_u32 id);	// Get data from FIFO when it's almost full
void uart_getdata(void* context, alt_u32 id);	// Get data from UART when something is sent

void getDirection(); 	// gets XY data and gives a array of values

/* ####### END FUNCTION DECLARATIONS ####### */

int main()
{
	// ## START INIT CODE ## //
	#ifdef DEBUG
	for(int i=0; i<16; i++){
		IOWR(HEX0_BASE, 0, sev_seg(i));
		usleep(10000);	// debug startup
	}
	#endif

	// Ensure that all FIR reset is held LOW
	IOWR(FIR_RST_BASE, 0, 0b000);

	// Activate interrupts on UART
	IOWR(uartCntrlRegPtr, 0, 0x1);	// Enable JTAG IRQ

	acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
	if (setup_accel((alt_up_accelerometer_spi_dev*)acc_dev)!=0) { while(1); }
	else {
		#ifdef DEBUG
		alt_printf("[INFO] Accel setup success.\n");
		#endif
	}	// Initialise Accelerometer

	if (setup_fifo()!=0) { while(1); }
	else {
		#ifdef DEBUG
		alt_printf("[INFO] FIFO setup success.\n");
		#endif
	}	// Initialise FIFOs

	int init_status;
	void* acc_dev_int_ptr = (void*) acc_dev;
	init_status = alt_irq_register(ACCELEROMETER_SPI_IRQ, acc_dev_int_ptr, accel_getdata);	// pass the acc handle
	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] Accel IRQ initialize error, code %x\n", init_status);
		#endif
		while(1);
	}

	init_status = alt_irq_register(INPUT_FIFO_X_IN_CSR_IRQ, NULL, fifo_x_getdata);	// pass the acc handle
	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] Input X FIFO IRQ initialize error, code %x\n", init_status);
		#endif
		while(1);
	}

	init_status = alt_irq_register(INPUT_FIFO_Y_IN_CSR_IRQ, NULL, fifo_y_getdata);	// pass the acc handle
	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] Input Y FIFO IRQ initialize error, code %x\n", init_status);
		#endif
		while(1);
	}

	init_status = alt_irq_register(INPUT_FIFO_Z_IN_CSR_IRQ, NULL, fifo_z_getdata);	// pass the acc handle
	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] Input Z FIFO IRQ initialize error, code %x\n", init_status);
		#endif
		while(1);
	}

//	init_status = alt_irq_register(JTAG_UART_IRQ, NULL, uart_getdata);	// pass the acc handle
//	if (init_status!=0){
//		#ifdef DEBUG
//		alt_printf("[FATAL] JTAG_UART IRQ initialize error, code %x\n", init_status);
//		#endif
//		while(1);
//	}

	// ## END INIT CODE ## //
	/* Event loop never exits. */
	while (1){
		uart_getdata(NULL, 0);	// nonblocking function
		getDirection();

		/*
		 * PUT GAME LOGIC HERE
		 */

		alt_u32 send_arr[4] = {
				xy_dir[0],
				xy_dir[1],
				nios_to_host_event,
				'\n'
		};

		#ifndef DEBUG
		write( 1, send_arr, 13);	// Write 3*4+1 bytes = 13 bytes to 1 (stdout)
		#endif

		usleep(sleep_time);	// Don't need to spam the the up/downlink

		#ifdef DEBUG
		// See FIFO status on 7seg
		alt_u8 x_fill = altera_avalon_fifo_read_level(INPUT_FIFO_X_IN_CSR_BASE);
		alt_u8 y_fill = altera_avalon_fifo_read_level(INPUT_FIFO_Y_IN_CSR_BASE);
		alt_u8 z_fill = altera_avalon_fifo_read_level(INPUT_FIFO_Z_IN_CSR_BASE);

		alt_u8 x_status = altera_avalon_fifo_read_status(INPUT_FIFO_X_IN_CSR_BASE, ALTERA_AVALON_FIFO_STATUS_ALL);
		alt_u8 y_status = altera_avalon_fifo_read_status(INPUT_FIFO_Y_IN_CSR_BASE, ALTERA_AVALON_FIFO_STATUS_ALL);
		alt_u8 z_status = altera_avalon_fifo_read_status(INPUT_FIFO_Z_IN_CSR_BASE, ALTERA_AVALON_FIFO_STATUS_ALL);

		IOWR(HEX0_BASE, 0, sev_seg(z_fill));
		IOWR(HEX1_BASE, 0, sev_seg(z_status));
		IOWR(HEX2_BASE, 0, sev_seg(y_fill));
		IOWR(HEX3_BASE, 0, sev_seg(y_status));
		IOWR(HEX4_BASE, 0, sev_seg(x_fill));
		IOWR(HEX5_BASE, 0, sev_seg(x_status));

		printf("%ld, ", xy_dir[0]);
		printf("%ld, ", fifo_data[0] >> FIR_SHIFT);
		printf("(%d | %d)\n", acc_data[0], acc_data[1]);
		printf("%ld, ", xy_dir[1]);
		printf("%ld, ", fifo_data[1] >> FIR_SHIFT);
		printf("(%d | %d)\n", acc_data[2], acc_data[3]);

		printf("%ld, ", fifo_data[2] >> FIR_SHIFT);
		printf("(%d | %d)\n\n", acc_data[4], acc_data[5]);

		#endif
	}

  return 0;
}

void accel_getdata(void* context, alt_u32 id){
	IOWR(LED_BASE, 0, 0x200);

	// contextify acc_dev pointer
	alt_up_accelerometer_spi_dev* acc_dev_ptr = (alt_up_accelerometer_spi_dev *)context;

	alt_irq_context irq_context = alt_irq_disable_all();

	alt_u8 acc_status;
	alt_up_accelerometer_spi_read(acc_dev_ptr, 0x30, &acc_status);
	// Ensure that incoming interrupt is the correct one
	if (acc_status & 0x80) {
		alt_32 readdata;
		if (acc_first==0){
			alt_up_accelerometer_spi_read_x_axis(acc_dev_ptr, &readdata);	// X
			acc_data[0] = readdata - acc_x_offset;
			alt_up_accelerometer_spi_read_y_axis(acc_dev_ptr, &readdata);	// Y
			acc_data[2] = readdata - acc_y_offset;
			alt_up_accelerometer_spi_read_z_axis(acc_dev_ptr, &readdata);	// Z
			acc_data[4] = readdata - acc_z_offset;
			acc_first = 1;
		} else {
			alt_up_accelerometer_spi_read_x_axis(acc_dev_ptr, &readdata);	// X
			acc_data[1] = readdata - acc_x_offset;
			alt_up_accelerometer_spi_read_y_axis(acc_dev_ptr, &readdata);	// Y
			acc_data[3] = readdata - acc_y_offset;
			alt_up_accelerometer_spi_read_z_axis(acc_dev_ptr, &readdata);	// Z
			acc_data[5] = readdata - acc_z_offset;

			// Convert to something ready to send (pack two symbols into one beat)
			// Also remember to remove sign extension for the LSB
			alt_u32 tosend;
			tosend = ( (acc_data[0]) << 16 | (acc_data[1]&0x0000FFFF) );
			altera_avalon_fifo_write_fifo(OUTPUT_FIFO_X_IN_BASE, OUTPUT_FIFO_X_IN_CSR_BASE, tosend );

			tosend = ( (acc_data[2]) << 16 | (acc_data[3]&0x0000FFFF) );
			altera_avalon_fifo_write_fifo(OUTPUT_FIFO_Y_IN_BASE, OUTPUT_FIFO_Y_IN_CSR_BASE, tosend );

			tosend = ( (acc_data[4]) << 16 | (acc_data[5]&0x0000FFFF) );
			altera_avalon_fifo_write_fifo(OUTPUT_FIFO_Z_IN_BASE, OUTPUT_FIFO_Z_IN_CSR_BASE, tosend );

			acc_first = 0;
		}
	}
	alt_irq_enable_all(irq_context);	// Re enable interrupts afterwards

	IOWR(LED_BASE, 0, 0x00);
}

/* Easy event access
 * #define ALTERA_AVALON_FIFO_EVENT_F_MSK    (0x01)
 * #define ALTERA_AVALON_FIFO_EVENT_E_MSK    (0x02)
 * #define ALTERA_AVALON_FIFO_EVENT_AF_MSK   (0x04)
 * #define ALTERA_AVALON_FIFO_EVENT_AE_MSK   (0x08)
 * #define ALTERA_AVALON_FIFO_EVENT_OVF_MSK  (0x10)
 * #define ALTERA_AVALON_FIFO_EVENT_UDF_MSK  (0x20)
 * #define ALTERA_AVALON_FIFO_EVENT_ALL  (0x3F)
 */

// Read data from FIFO. In this case we dont need to see which interrupt it was, as there is only one.
// Read twice, as input data rate is twice of output data rate
void fifo_x_getdata(void* context, alt_u32 id){
	alt_irq_context irq_context = alt_irq_disable_all();

	fifo_data[0] = altera_avalon_fifo_read_fifo(INPUT_FIFO_X_OUT_BASE, INPUT_FIFO_X_IN_CSR_BASE);
	altera_avalon_fifo_clear_event(INPUT_FIFO_X_IN_CSR_BASE, ALTERA_AVALON_FIFO_EVENT_ALL);	// Clear FIFO event

	alt_irq_enable_all(irq_context);	// Re enable interrupts afterwards
}

void fifo_y_getdata(void* context, alt_u32 id){
	alt_irq_context irq_context = alt_irq_disable_all();

	fifo_data[1] = altera_avalon_fifo_read_fifo(INPUT_FIFO_Y_OUT_BASE, INPUT_FIFO_Y_IN_CSR_BASE);
	altera_avalon_fifo_clear_event(INPUT_FIFO_Y_IN_CSR_BASE, ALTERA_AVALON_FIFO_EVENT_ALL); // Clear FIFO event

	alt_irq_enable_all(irq_context);	// Re enable interrupts afterwards
}

void fifo_z_getdata(void* context, alt_u32 id){
	alt_irq_context irq_context = alt_irq_disable_all();

	fifo_data[2] = altera_avalon_fifo_read_fifo(INPUT_FIFO_Z_OUT_BASE, INPUT_FIFO_Z_IN_CSR_BASE);
	altera_avalon_fifo_clear_event(INPUT_FIFO_Z_IN_CSR_BASE, ALTERA_AVALON_FIFO_EVENT_ALL); // Clear FIFO event

	alt_irq_enable_all(irq_context);	// Re enable interrupts afterwards
}

// Interrupt handler for JTAG-UART events
void uart_getdata(void* context, alt_u32 id){
	alt_irq_context irq_context = alt_irq_disable_all();
	IOWR(LED_BASE, 0, 0b1);

	alt_u32 jtag_status = *uartDataRegPtr;
	host_to_nios_event  = jtag_status & 0xFF;		// UART Data register

//	Interrupt is not cleared after this!
	IOWR(LED_BASE, 0, 0b0);
	alt_irq_enable_all(irq_context);	// Re enable interrupts afterwards
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

int setup_fifo(){
	int init_status;
	// Init Output X FIFO
	init_status = altera_avalon_fifo_init(OUTPUT_FIFO_X_IN_CSR_BASE, out_fifo_ienable,
									// OUT buffer has no interrupts
									3,	// Almost empty level
									9	// Almost full level
									);
	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] X Output FIFO initialize error, code %x\n", init_status);
		#endif
		while(1);
	} else {
		#ifdef DEBUG
		alt_printf("[INFO] X Output FIFO initialized.\n");
		alt_printf("ALMOSTEMPTY = %x\n",
					altera_avalon_fifo_read_almostempty(OUTPUT_FIFO_X_IN_CSR_BASE) );
		alt_printf("ALMOSTFULL = %x\n",
					altera_avalon_fifo_read_almostfull(OUTPUT_FIFO_X_IN_CSR_BASE) );
		alt_printf("iENABLE = %x\n\n",
					altera_avalon_fifo_read_ienable(OUTPUT_FIFO_X_IN_CSR_BASE,ALTERA_AVALON_FIFO_IENABLE_ALL) );
		#endif
	}

	// Init Input X FIFO
	init_status = altera_avalon_fifo_init(INPUT_FIFO_X_IN_CSR_BASE, in_fifo_ienable,
								// interrupts for ALMOST_FULL(0x8) ALMOST EMPTY (0x4) and FULL(0x2).
								3,	// Almost empty level
								in_fifo_af_level	// Almost full level
								);

	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] X Input FIFO initialize error, code %x\n", init_status);
		#endif
		while(1);
	} else {
		#ifdef DEBUG
		alt_printf("[INFO] X Input FIFO initialized.\n");
		alt_printf("ALMOSTEMPTY = %x\n",
					altera_avalon_fifo_read_almostempty(INPUT_FIFO_X_IN_CSR_BASE) );
		alt_printf("ALMOSTFULL = %x\n",
					altera_avalon_fifo_read_almostfull(INPUT_FIFO_X_IN_CSR_BASE) );
		alt_printf("iENABLE = %x\n\n",
					altera_avalon_fifo_read_ienable(INPUT_FIFO_X_IN_CSR_BASE,ALTERA_AVALON_FIFO_IENABLE_ALL) );
		#endif
	}

	// Init Output Y FIFO
	init_status = altera_avalon_fifo_init(OUTPUT_FIFO_Y_IN_CSR_BASE, out_fifo_ienable,
									// OUT buffer has no interrupts
									3,	// Almost empty level
									9	// Almost full level
									);
	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] Y Output FIFO initialize error, code %x\n", init_status);
		#endif
		while(1);
	} else {
		#ifdef DEBUG
		alt_printf("[INFO] Y Output FIFO initialized.\n");
		alt_printf("ALMOSTEMPTY = %x\n",
					altera_avalon_fifo_read_almostempty(OUTPUT_FIFO_Y_IN_CSR_BASE) );
		alt_printf("ALMOSTFULL = %x\n",
					altera_avalon_fifo_read_almostfull(OUTPUT_FIFO_Y_IN_CSR_BASE) );
		alt_printf("iENABLE = %x\n\n",
					altera_avalon_fifo_read_ienable(OUTPUT_FIFO_Y_IN_CSR_BASE,ALTERA_AVALON_FIFO_IENABLE_ALL) );
		#endif
	}

	// Init Input Y FIFO
	init_status = altera_avalon_fifo_init(INPUT_FIFO_Y_IN_CSR_BASE, in_fifo_ienable,
								// interrupts for ALMOST_FULL(0x8) ALMOST EMPTY (0x4) and FULL(0x2).
								3,	// Almost empty level
								in_fifo_af_level	// Almost full level
								);

	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] Y Input FIFO initialize error, code %x\n", init_status);
		#endif
		while(1);
	} else {
		#ifdef DEBUG
		alt_printf("[INFO] Y Input FIFO initialized.\n");
		alt_printf("ALMOSTEMPTY = %x\n",
					altera_avalon_fifo_read_almostempty(INPUT_FIFO_Y_IN_CSR_BASE) );
		alt_printf("ALMOSTFULL = %x\n",
					altera_avalon_fifo_read_almostfull(INPUT_FIFO_Y_IN_CSR_BASE) );
		alt_printf("iENABLE = %x\n\n",
					altera_avalon_fifo_read_ienable(INPUT_FIFO_Y_IN_CSR_BASE,ALTERA_AVALON_FIFO_IENABLE_ALL) );
		#endif
	}

	// Init Output Z FIFO
	init_status = altera_avalon_fifo_init(OUTPUT_FIFO_Z_IN_CSR_BASE, out_fifo_ienable,
									// OUT buffer has no interrupts
									3,	// Almost empty level
									9	// Almost full level
									);
	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] Z Output FIFO initialize error, code %x\n", init_status);
		#endif
		while(1);
	} else {
		#ifdef DEBUG
		alt_printf("[INFO] Z Output FIFO initialized.\n");
		alt_printf("ALMOSTEMPTY = %x\n",
					altera_avalon_fifo_read_almostempty(OUTPUT_FIFO_Z_IN_CSR_BASE) );
		alt_printf("ALMOSTFULL = %x\n",
					altera_avalon_fifo_read_almostfull(OUTPUT_FIFO_Z_IN_CSR_BASE) );
		alt_printf("iENABLE = %x\n\n",
					altera_avalon_fifo_read_ienable(OUTPUT_FIFO_Z_IN_CSR_BASE,ALTERA_AVALON_FIFO_IENABLE_ALL) );
		#endif
		}

	// Init Input Z FIFO
	init_status = altera_avalon_fifo_init(INPUT_FIFO_Z_IN_CSR_BASE, in_fifo_ienable,
								// interrupts for ALMOST_FULL(0x8) ALMOST EMPTY (0x4) and FULL(0x2).
								3,	// Almost empty level
								in_fifo_af_level	// Almost full level
								);

	if (init_status!=0){
		#ifdef DEBUG
		alt_printf("[FATAL] Z Input FIFO initialize error, code %x\n", init_status);
		#endif
		while(1);
	} else {
		#ifdef DEBUG
		alt_printf("[INFO] Z Input FIFO initialized.\n");
		alt_printf("ALMOSTEMPTY = %x\n",
					altera_avalon_fifo_read_almostempty(INPUT_FIFO_Z_IN_CSR_BASE) );
		alt_printf("ALMOSTFULL = %x\n",
					altera_avalon_fifo_read_almostfull(INPUT_FIFO_Z_IN_CSR_BASE) );
		alt_printf("iENABLE = %x\n\n",
					altera_avalon_fifo_read_ienable(INPUT_FIFO_Z_IN_CSR_BASE,ALTERA_AVALON_FIFO_IENABLE_ALL) );
		#endif
	}

	return 0;
}

int setup_accel(alt_up_accelerometer_spi_dev* acc_dev){

	if (acc_dev == NULL) { // if return 1, check if the spi ip name is "accelerometer_spi"
		#ifdef DEBUG
		alt_printf("[FATAL] No accelerometer device available\n");
		#endif
		while(1);
	}
	alt_u8 acc_status;
	// Set sensitivity here
	alt_up_accelerometer_spi_read((alt_up_accelerometer_spi_dev *)acc_dev, 0x31, &acc_status);
//	alt_printf("Resolution acc_status %x.\n", acc_status);
	acc_status &= 0xFC;	// set to +-2G sensitivity, write back
	alt_up_accelerometer_spi_write((alt_up_accelerometer_spi_dev *)acc_dev, 0x31, acc_status); // set to +-2G sensitivity
	// Set interrupts here
	alt_up_accelerometer_spi_read((alt_up_accelerometer_spi_dev *)acc_dev, 0x2E, &acc_status);
//	alt_printf("Interrupt acc_status %x.\n", acc_status);
	acc_status |= 0x80;	// set enable DATA_READY interrupts, write back
	alt_up_accelerometer_spi_write((alt_up_accelerometer_spi_dev *)acc_dev, 0x2E, acc_status); // Enable DATA_READY interrupts
	alt_up_accelerometer_spi_read((alt_up_accelerometer_spi_dev *)acc_dev, 0x2C, &acc_status);
//	alt_printf("Rate acc_status %x.\n", acc_status);
	acc_status = accel_samplerate;	// Rate
	alt_up_accelerometer_spi_write((alt_up_accelerometer_spi_dev *)acc_dev, 0x2C, acc_status); // Enable DATA_READY interrupts

	return 0;
}

// if dir > 3, return 3
// if dir < -3, return -3 (bounds the return data)
// there is a "deadzone" so that the controller isn't too sensitive
// (implemented by the first 'if')
void getDirection(){
	alt_8 dir;
	dir = fifo_data[0] >> FIR_SHIFT;
	if (dir <= 1 && dir >= -1){
		dir = 0;
	} else if(dir > 4){
		dir = 4;
	} else if (dir < -4) {
		dir = -4;
	} else if (dir > 0){
		dir --;
	} else if (dir < 0){
		dir ++;
	}
	xy_dir[0] = dir;

	dir = fifo_data[1] >> FIR_SHIFT;
	if (dir <= 1 && dir >= -1){
		dir = 0;
	} else if(dir > 4){
		dir = 4;
	} else if (dir < -4) {
		dir = -4;
	} else if (dir > 0){
		dir --;
	} else if (dir < 0){
		dir ++;
	}
	xy_dir[1] = dir;
}
