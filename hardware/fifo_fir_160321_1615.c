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
#include "sys/alt_stdio.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>	// Enable nonblocking


/* ####### VARIABLE DEFINITIONS ####### */
#define DEBUG 1					// set to 0 for comms with host PC
#define SEND_TO_PC 1

#define TX_RATE 2				// in hertz, to host PC
const alt_u32 sleep_time = 1000000/TX_RATE;
const alt_u16 FIR_SHIFT = 19;	// number of bits to shift output FIR data
const alt_32 acc_x_offset = 2;
const alt_32 acc_y_offset = -10;
const alt_32 acc_z_offset = -45;	// Offset for the accelerometer readings

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
	GAME_MOVE,
	SWITCH_STATES_TASK,
	PRESS_BUTTON_TASK,
	NUM_TO_BIN_TASK,
    LED_CATCH_TASK,
    Z_AXIS_TASK,
    WINGAME_ROOM,
	LOSEGAME_ROOM
};

// Code to send to the host
// 0 is done, 1 is not done
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
void reset_sev_seg();                          // clears all sevSegs

int setup_fifo();								// sets up the fifos
int setup_accel(alt_up_accelerometer_spi_dev* dev);
void accel_getdata(void* context, alt_u32 id);	// Read data from accelerometer on interrupt
void fifo_x_getdata(void* context, alt_u32 id);	// Get data from FIFO when it's almost full
void fifo_y_getdata(void* context, alt_u32 id);	// Get data from FIFO when it's almost full
void fifo_z_getdata(void* context, alt_u32 id);	// Get data from FIFO when it's almost full

void getDirection(); 	// gets XY data and gives a array of values
void timer_init();

/* GAME STATES */
int rng();
int lobby();
int button_task();
int led_switch_task();
int random_number_task();
int led_button_task();
int z_axis_task();
int wingame();
int losegame();

/* ####### END FUNCTION DECLARATIONS ####### */

int main()
{
	// ## START INIT CODE ## //

	// Ensure that all FIR reset is held LOW
	IOWR(FIR_RST_BASE, 0, 0b000);
//    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	// To enable non-blocking (dangerous! Don't use this!)

	timer_init();	// start the timer

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

	reset_sev_seg();

	// ## END INIT CODE ## //
	/* Event loop never exits. */
	while (1){
		char get_char[1] = { alt_getchar() };
		alt_u8 nios_event = atoi( get_char );

        getDirection();

        IOWR(HEX5_BASE, 0, sev_seg(nios_event));	// visually see what task is being done

		/* Game Logic */
        switch (nios_event){
        case WAITING_ROOM: // Lobby state
            nios_to_host_event = lobby();
            break;
        case GAME_MOVE: // Moving around
            nios_to_host_event = 1;
            break;
        case SWITCH_STATES_TASK:
            nios_to_host_event = led_switch_task();       //match switch to led
            break;
        case PRESS_BUTTON_TASK:
            nios_to_host_event = button_task();          //press as many times as the 7seg
            break;
        case NUM_TO_BIN_TASK:
            nios_to_host_event = random_number_task();  //switch binary of 7seg decimal
            break;
        case LED_CATCH_TASK:
            nios_to_host_event = led_button_task();  //NOTE:changed task
            break;				//top button is +10, bottom is +1, match the numbers
        case Z_AXIS_TASK:
            nios_to_host_event = z_axis_task();  //shake back and forth 10 times
            break;
        case WINGAME_ROOM:
            nios_to_host_event = wingame();
            break;
		case LOSEGAME_ROOM:
			nios_to_host_event = losegame();
			break;
        default:
            alt_printf("Unexpected input %d. Exiting.\n", nios_event)  ;        //wait for task from server again
            return 1;
        }

		alt_u32 send_arr[4] = {
            xy_dir[0],  // x dir {-4,4}
            xy_dir[1],  // y dir {-4,4}
            nios_to_host_event, // enum 0-??
            '\n'        // packet delimiter (helps with readline())
		};

		#ifdef SEND_TO_PC
		alt_printf("{%x %x %x}", send_arr[0], send_arr[1], send_arr[2]);
//		write(1, send_arr, 13);	// Write 3*4+1 bytes = 13 bytes to 1 (stdout)
		#endif

		reset_sev_seg();
//		usleep(sleep_time);	// Don't need to spam the the up/downlink

		#ifndef SEND_TO_PC
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
int lobby(){
	#ifdef DEBUG
	alt_printf  ("Lobby\n") ;
	#endif

	while(1){
		int button_datain =  ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);      //read button
	    button_datain &= (0x01);                                        //top button gives 1

	    if(button_datain) return 1;
	}
}

int button_task(){
	#ifdef DEBUG
    alt_printf  ("button_task\n") ;
    #endif
    int random_value = rng();

    int value=5;
    if(random_value > 1000){value = 20; random_value=0;}
    else if(random_value < 200){random_value=0;}
    while(random_value>=200){value++; random_value -=53;}

    int v1=0, v2=0;
    while(value >= 10){v1++; value -=10;}
    while(value > 0){v2++;  value -=1;}

    int button_datain;

    while(1){
        button_datain = ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);          //read button
        button_datain &= (0b0000000001);                                 //top button gives 1
        int seg1 = sev_seg(v1);
        int seg2 = sev_seg(v2);
        IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, seg1); //loop
        IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, seg2);

        if(button_datain==1){
        	if(v2==0){v2=9; v1--;}
        	else{v2--;}

        	while(button_datain==1){
        	button_datain = ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);          //read button
            button_datain &= (0b0000000001);
        	}
        }
        if(v1==0 && v2==0){return 1;}
    }
}

int led_switch_task(){
	#ifdef DEBUG
    alt_printf  ("led_switch_task\n") ;
	#endif

    int led_value = rng();
    int switch_datain;

    IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, led_value);

    while(1){
    	switch_datain = IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE);
    	if(switch_datain==led_value) return 1;
    }
}

int random_number_task(){
    #ifdef DEBUG
    alt_printf("random_number_task\n");
    #endif

    // Generate random numbers
    int random_value = rng();
    //-(random_value + 1);   


    int d1=0,d2=0;      
    while(random_value>=0){
    	d2++; random_value -= 16;
    	if(d2==10){d2=0; d1++;}
    }

    int led_to_switch = d1*10 + d2;
    int seg1 = sev_seg(d1);
    int seg2 = sev_seg(d2);

    int switch_datain;

    IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, seg1);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, seg2);   


    while(1){
    	switch_datain = IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE);
    	if(switch_datain==led_to_switch){return 1;}
    }
}


int led_button_task(){
	#ifdef DEBUG
    alt_printf  ("led_catch\n") ;
	#endif

    int target= 1;
    int count = 3;
    int button_data;

    while(1){
    	int catch_area = 16+64;
    	int disp =sev_seg(count);
    	IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE,disp);

    	if(target==16 || target==64){ IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, catch_area);}
    	else {IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, catch_area+target);}
    	usleep(50000);

    	button_data = ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);
    	button_data &= (1);
    	if(button_data==1){
    		if(target==32){
    			count-=1;
    			if(count==0){return 1;}
    			int disp = sev_seg(count);
    			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE,disp);
    		}
    		while(button_data==1){
    		    		button_data = ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);
    		    		button_data &= (0b0000000001);
    		}
    	}

    	usleep(10000);
    	target *=2;
    	if(target==1024){target=1;}

    }


    return 1;
}


int z_axis_task(){
    #ifdef DEBUG
    alt_printf  ("z_axis\n") ;
    #endif

    // use acc_data[4] to read unfiltered z-axis accelerometer data
    int shake_count = 10;
    int state = 1;
    while(1){

    	if(state == 1 && acc_data[4] < 30 ){state=0; shake_count--;}
    	if(state == -1 && acc_data[4] > 30 ){state=1; shake_count--;}
    	if(state == 0 ) { state=-1;}

    	if(shake_count==0) { return 1;}
    }
   return 1;
}

int endgame(){ //TODO:idk what we want here
    #ifdef DEBUG
    alt_printf  ("endgame\n") ;
    #endif

    return 1;
}

int wingame(){ //TODO:idk what we want here
    #ifdef DEBUG
    alt_printf  ("wingame\n") ;
    #endif
    return 1;
}

int losegame(){
	#ifdef DEBUG
    alt_printf  ("losegame\n") ;
    #endif

    IOWR_ALTERA_AVALON_PIO_DATA(HEX3_BASE,0b11000111);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX2_BASE,0b11000000);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE,0b10010010);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE,0b10000110);
    usleep(1000000);
    return 1;

}

//MISC

int rng(){
	IOWR(TIMER_BASE,4,1);
	int value = IORD_ALTERA_AVALON_TIMER_SNAPL(TIMER_BASE);
	return value;
}

// clears the sevseg disp and LEDs
void reset_sev_seg(){
    // alt_printf  ("reset\n") ;
    IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX2_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX3_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX4_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX5_BASE, 0xFF);
    return;
}

void timer_init() {

    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0002);  //11
    // continuous, no irq

    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);
    // Clears the TO bit in status register, just in case

    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_BASE, 0x03FF);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_BASE, 0x0000);

    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0007); //111
    // Write 0x7=0b0111 to start the timer

    return;
}