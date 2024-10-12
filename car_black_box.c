/*
 * File:   car_black_box.c
 * Author: siddharthsingh
 *
 * Created on February 8, 2023, 9:34 PM
 */


#include <xc.h> 

#pragma config WDTE = OFF //Watchdog timer disabled 

static void init_config(void) {
    //Write your initialization code here 
}

void main(void) {
    init_config(); //Calling initializing function 
    while (1) {
        //Write application code here 
    }

}
