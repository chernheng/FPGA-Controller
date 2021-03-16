#include "system.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include <stdio.h>
#include <stdlib.h>
//#include "sys/alt_stdio.h"

//TODO: task timeout, resend task request?, adjust delay,
//      filter x,y,z ; random number(get val from timer? or server)  dont need to filter z?
//      input,output from host

//GAME STATES
int lobby(){     //TODO:idk what we want here, button press?
	alt_printf("lobby");
    //printf("lobby incomplete");
    return 1;
}

int x_move(){          //todo: filter, is loop needed?, remove delay?
    alt_printf  ("movement\n") ;
     int x_direc;
     alt_32 x_read;
         alt_up_accelerometer_spi_dev * acc_dev;
         acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
         if (acc_dev == NULL) { // if return 1, check if the spi ip name is "accelerometer_spi"
             return 0;
         }

    for(int k=0; k<10; k++){                          //loop just for testing

    	alt_up_accelerometer_spi_read_x_axis(acc_dev, & x_read);
    	alt_printf("raw data: %x\n", x_read);



        if(x_read<-144){alt_printf("RIGHT, 3\n"); x_direc=3;}
        else if(x_read<-80){alt_printf("RIGHT, 2\n");x_direc=2;}
        else if(x_read<0){alt_printf("RIGHT, 1\n");x_direc=1;}

        if(x_read>144){alt_printf("LEFT, 3\n");x_direc=-3;}
        else if(x_read>80){alt_printf("LEFT, 2\n");x_direc=-2;}
        else if(x_read>0){alt_printf("LEFT, 1\n");x_direc=-1;}  //return x_direc


        for(int k=0; k<1000000;k++){;}   //scuffed delay

    }


    return  x_direc;
}

int y_move(){ return 3; } //TODO:same as x

int led_switch_task(){   //TODO:randomize led_value 1-1023
    alt_printf  ("led_switch_task") ;

    int led_value = 200;
    int switch_datain;

    int led_to_switch = -(led_value+1);
    IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, led_value);

    while(1){
    	switch_datain = ~IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE);
    	alt_printf("switch data: %x, compare_to: %x\n", switch_datain, led_to_switch );

    	if(switch_datain==led_to_switch){return 1;}
    	// for(int k=0; k<1000000;k++){;}
    }


}

int button_task(){      //TODO:1023 random value
    alt_printf  ("button_task\n") ;
   // printf("button_task");
    int random_value = 700;  //try 300 104 99 1023 999

    int value=3;
    if(random_value > 900){value = 9; random_value=0;}
    else if(random_value < 200){value = 3; random_value=0;}
    while(random_value>=200){value++; random_value -=150;}
    //alt_prinf("rand %x\n, value %x\n", random_value, value);
    //for(int k=0; k<1000000;k++){;}

    int button_datain;


    while(1){
        button_datain = ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);          //read button
        button_datain &= (0b0000000001);                                 //top button gives 1
        alt_printf("button data: %x\n", button_datain);

        int seg_disp = digit_to_7seg(value);
        IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, seg_disp); //loop



        if(button_datain==1){
        	value -= 1;
        	while(button_datain==1){
        	button_datain = ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);          //read button
            button_datain &= (0b0000000001);
            alt_printf("button unpress %x\n",button_datain);
           // for(int k=0; k<1000000;k++){;}
        	}
        }

        if(value==0){return 1;}
    }
}

int random_number_task(){   //1 is msb  //TODO:random value
    alt_printf  ("random_number_task") ;

    int random_value = 700;
    int led_to_switch = -(random_value + 1);

    int d1=0,d2=0,d3=0,d4=0;
    while(random_value>=1000){d1++; random_value -= 1000;}
    while(random_value>=100){d2++; random_value -= 100; }
    while(random_value>=10){d3++; random_value -= 10; }
    while(random_value>0){d4++; random_value -= 1; }


    int seg1 = digit_to_7seg(d1);
    int seg2 = digit_to_7seg(d2);
	int seg3 = digit_to_7seg(d3);
	int seg4 = digit_to_7seg(d4);
	alt_printf("d1 %x, d2 %x, d3 %x, d4 %x\n", d1, d2, d3, d4);

    int switch_datain;


    if(d1 == 1){IOWR_ALTERA_AVALON_PIO_DATA(HEX3_BASE, seg1);}
    IOWR_ALTERA_AVALON_PIO_DATA(HEX2_BASE, seg2);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, seg3);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, seg4);

    while(1){
    	switch_datain = ~IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE);
    	alt_printf("switch data: %x, compare_to: %x\n", switch_datain, led_to_switch );

    	if(switch_datain==led_to_switch){return 1;} //alt_printf("succes");
    //	for(int k=0; k<1000000;k++){;}

    }


}

int increment_task(){  //TODO: random value 0 to 99??
    alt_printf  ("increment\n") ;
    //button 1 +1
    // button 2 + 10


    int random_value = 25;
    int desired_value = random_value;
    int curr1=0;
    int curr2=0;
    int d3=0,d4=0;

    while(random_value>=10){d3++; random_value -= 10; }
    while(random_value>0){d4++; random_value -= 1; }

    int seg3 = digit_to_7seg(d3);
    int seg4 = digit_to_7seg(d4);

    IOWR_ALTERA_AVALON_PIO_DATA(HEX3_BASE, seg3);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX2_BASE, seg4);

    IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, 0b1000000);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, 0b1000000);


    int button_datain;
    while(1){
    	button_datain = ~IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE);          //read button
    	button_datain &= (0b0000000011);		//top button gives 01, bottom button gives 10

    	if(button_datain==0b0000000001){
    		curr1 += 1;
    		if(curr1==10){curr1=0;}
    	}

    	else if(button_datain==0b0000000010){
    		curr2 +=1;
    		if(curr2==10){curr2=0; curr1 +=1;}
    	}

    	else{ alt_printf("no/both buttons pressed\n");}

    	int seg1 = digit_to_7seg(curr1);
    	int seg2 = digit_to_7seg(curr2);

    	IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, seg2);
    	IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, seg1);

    	for(int k=0; k<500000;k++){;}  //needed delay

    	int current = curr1*10 + curr2;
    	if(current==desired_value){return 1;}

    	alt_printf("curr1 %x, curr2 %x, current, %x\n",curr1,curr2,current);

    }



   // printf("led_button");
    return 1;
}

int z_axis_task(){  //TODO: change to z-axis //could change/remove?? delay
    alt_printf  ("z_axis") ;


    alt_32 x_read;
    alt_up_accelerometer_spi_dev * acc_dev;
    acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
    if (acc_dev == NULL) { // if return 1, check if the spi ip name is "accelerometer_spi"
         return 0;
    }

    alt_up_accelerometer_spi_read_x_axis(acc_dev, & x_read);
    alt_printf("raw data: %x\n", x_read);

    int shake_count = 10;
    int state = 1;
    while(1){
    	alt_up_accelerometer_spi_read_x_axis(acc_dev, & x_read);
    	if(x_read>30){alt_printf("left: %x\n", x_read);}
    	else if(x_read<-30){alt_printf("right: %x\n", x_read);}
    	else{alt_printf("ignore: %x\n", x_read);}

    	if(state == 1 && x_read<30 ){state=0; shake_count--;}
    	if(state == -1 && x_read>30 ){state=1; shake_count--;}
    	if(state == 0 ){state=-1;}

    	alt_printf("shake_count: %x\n", shake_count);
    	if(shake_count==0){return 1;}



    }




    return 1;


}

int endgame(){ //TODO:idk what we want here
    alt_printf  ("endgame") ;
   // printf("endgame");
    return 5;
}


//MISC
int read_accel_x();   //TODO?: seperate function for filtering??

int rng(); //TODO: is it possible to get timer count value?

void reset_function(){
    alt_printf  ("reset\n") ;
    IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, 0b1111111);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, 0b1111111);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX2_BASE, 0b1111111);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX3_BASE, 0b1111111);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX4_BASE, 0b1111111);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX5_BASE, 0b1111111);
    //printf("reset");
    return;
   // reset led etc   //maybe not needed
}


int digit_to_7seg(int seg){  //TODO: turn off the dot

		int display;
		switch (seg){
			case 0:
				display = 0b1000000 ;
				break;
			case 1:
				display = 0b1111001 ;
				break;
			case 2:
				display = 0b0100100 ;
				break;
			case 3:
				display = 0b0110000 ;
				break;
			case 4:
				display = 0b0011001 ;
				break;
			case 5:
				display = 0b0010010 ;
				break;
			case 6:
				display = 0b0000010 ;
				break;
			case 7:
				display = 0b1111000 ;
				break;
			case 8:
				display = 0b0000000 ;
				break;
			case 9:
				display = 0b0011000 ;
				break;
			default:
				alt_printf("rand to 7seg error");

			}

return display;

}


int main(){



    while(1){
        //if button press, send number_code to host to start connection ?


    	reset_function();   //TODO: diff start setting for 7seg??


        int STATE=6; //TODO: wait until receive state from host

       // alt_printf  ("gimme an input, 7 to exit") ;
        //std::cin >> STATE;

        int complete=0;							//NOTE: should initial values change?
        int x_direc=0, y_direc=0, z_direc=0;   //TODO: we actually dont need z_direc




        switch (STATE){
            case 0:
                complete = lobby();
                break;
            case 1:
                x_direc = x_move();
                y_direc = y_move();
                break;
            case 2:
                complete = led_switch_task();       //match switch to led
                break;
            case 3:
                complete = button_task();          //press as many times as the 7seg
                break;
            case 4:
                complete = random_number_task();  //switch binary of 7seg decimal
                break;
            case 5:
                complete = increment_task();  //NOTE:changed task
                break;				//top button is +10, bottom is +1, match the numbers
            case 6:
                complete = z_axis_task();  //shake back and forth 10 times
                break;
            case 7:
                complete = endgame();
                break;
            default:
                alt_printf("unexpected input")  ;        //wait for task from server again
                break;
            }


        reset_function();


        //TODO: send to host instead of print
        alt_printf("task %x, x_direc %x, y_direc %x, complete? %x\n",STATE, x_direc, y_direc, complete);       //5 is a random succes code   TODO:print accel value


        //TODO: remove return for continuous
        return 1;





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