//NOTE: only state 1 and 3 does stuff right now, need to add jtag_uart stuff

#include "system.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include <stdio.h>
#include <stdlib.h>
//#include "sys/alt_stdio.h"


//GAME STATES
int lobby(){
	alt_printf("lobby");
    return 1;
}

int movement(){          //todo: delay?, forloop for fir?(diff inputs), adjust xread values, return vector , seperate function x,y             //for loop so enough (diff time inputs??)
    alt_printf  ("movement\n") ;
     int x_direc, y_direc;
     alt_32 x_read;
         alt_up_accelerometer_spi_dev * acc_dev;
         acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
         if (acc_dev == NULL) { // if return 1, check if the spi ip name is "accelerometer_spi"
             return 1;
         }

    for(int k=0; k<10; k++){                          //loop just for testing
    alt_up_accelerometer_spi_read_x_axis(acc_dev, & x_read);
    alt_printf("raw data: %x\n", x_read);



        if(x_read<-90){alt_printf("RIGHT, 3\n"); x_direc=3;}
        else if(x_read<-15){alt_printf("RIGHT, 2\n");x_direc=2;}
        else if(x_read<0){alt_printf("RIGHT, 1\n");x_direc=1;}

        if(x_read>90){alt_printf("LEFT, 3\n");x_direc=-3;}
        else if(x_read>15){alt_printf("LEFT, 2\n");x_direc=-2;}
        else if(x_read>0){alt_printf("LEFT, 1\n");x_direc=-1;}  //return x_direc

        for(int k=0; k<1000000;k++){;}   //scuffed delay

    }



    return 5;
}

int led_switch_task(){
    alt_printf  ("led_switch_task") ;

    return 1;
}

int button_task(){      //todo argument, led countdown, button cannot hold?, scuffed delay
    alt_printf  ("button_task\n") ;

    int amount = 6;
    //int led_indicator = 32;
    //int led_value = 63;
    int led_indicator = 1;
    int led_value = 1;
    int switch_datain;


    while(1){
        switch_datain = ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);          //read button
        switch_datain &= (0b0000000001);                                 //top button gives 1
        alt_printf("switch data: %x\n", switch_datain);


        if(switch_datain==1){
        		amount -= 1;
        		IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, led_value); //loop
        		led_indicator=led_indicator*2;
        		led_value = led_value + led_indicator;

        }

        for(int k=0; k<1000000;k++){;}   //scuffed delay
        if(amount==0){return 5;}
    }
}

int random_number_task(){
    alt_printf  ("random_number_task") ;

    return 1;
}

int led_button_task(){
    alt_printf  ("led_button") ;

    return 1;
}

int z_axis_task(){
    alt_printf  ("z_axis") ;

    return 1;
}

int endgame(){
    alt_printf  ("endgame") ;

    return 5;
}


//MISC
int read_accel_x();

void reset_function(){       //TODO?
    alt_printf  ("reset\n") ;
    IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0);
    return;
}


int main(){



    while(1){
        //TODO:connection/ready

        int STATE=1;    

        //TODO: read state from host
        int x;                            


        switch (STATE){          //todo specify x type
            case 0:
                x=lobby();                     
                break;
            case 1:           
                x = movement();   //vector
                break;
            case 2:
                x = led_switch_task();
                break;
            case 3:
                x = button_task();       //look up random ip core, or random from server
                break;
            case 4:
                x = random_number_task();
                break;
            case 5:
                x = led_button_task();
                break;
            case 6:
                x = z_axis_task();
                break;
            case 7:
                x=endgame();
                break;
            default:
                alt_printf("unexpected input")  ;
            }


        reset_function();


        if(x==5){alt_printf("SUCCESS-task %x\n",STATE);}       //5 is a random succes code   TODO:print accel value

        return 1;



    //TODO : send to host instead of print





    }
}

//use timer for rng??
//https://www.intel.com/content/www/us/en/programmable/documentation/dmi1455632999173.html - doesnt work for de10?

/* Send data over the JTAG_UART connection
 * Format:
* 0: BTN[0]{MSB} SWITCH[9:0] {LSB}
* 1: ACC_X [31:0]
* 2: ACC_Y [31:0]
* 3: ACC_Z [31:0]
* 4: OTHER[31:0]
* 5: '\n' delimiter [reserved]
*/

//add \n