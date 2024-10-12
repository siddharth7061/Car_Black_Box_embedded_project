/*
 * File:   main.c
 * Author: siddharthsingh
 *
 * Created on February 8, 2023, 9:17 PM
 */


#include "main.h"

#pragma config WDTE = OFF //Watchdog timer disabled 

//unsigned char first_time = 1;

static void init_config(void) {
    //Write your initialization code here 
    GIE = 1;
    PEIE = 1;
    init_clcd();
    init_adc();
    init_i2c(100000);
    init_ds1307();
    init_digital_keypad();
    init_timer2();
    init_uart(9600);
    
    
}

void main(void) {
    init_config(); //Calling initializing function 
    unsigned char control_flag = DASH_BOARD ;  // setting control flow to dashboard screen on starting up
    char event[3] = "ON";
    char speed = 0, gr = 0;
    log_car_event(event, speed);
    char *gear[] = {"GN", "GR", "G1", "G2", "G3", "G4"};
    unsigned char key, key_2;
    unsigned char reset_flag = RESET_PASSWORD;
    char var = 0;
    unsigned char wait = 0;
    char menu_entry = 0;
    unsigned char d_flag = 0;
    char ch[] = "NO DATA";
    
    // storing password "1010" st 0x00 address
    eeprom_at24c04_str_write(0x00, "1010");
    while (1) {
        //Write application code here 
        
        // reading digital keypad
        key = read_digital_keypad(STATE);
        // creating delay
        for(int j = 5000;j--;);

        if(control_flag == 0x01) // DASHBOARD
        {
            // acquiring potentiometer reading value from adc and storing it in speed variable
            speed = read_adc() / 10;
            
            // setting speed limit upto 99
            if(speed > 99)
            {
                speed = 99;
            }

            if(key == SW1)
            {
                // Collision -> when sw1 is pressed it records collision in the events
                strcpy(event, "C ");
                log_car_event(event, speed);
                d_flag = 0;
            }
            else if(key == SW2)
            {
                //UP Gearing -> increment gear when sw2 is pressed
                strcpy(event, gear[gr]);
                gr++;
                log_car_event(event, speed);
                d_flag = 0;
            }
            else if(key == SW3)
            {
                // Down Gearing -> decrement gear when sw3 is pressed
                gr--;
                strcpy(event, gear[gr]);
                log_car_event(event, speed);
                d_flag = 0;
            }
            else if((key == SW4 || key == SW5) && control_flag == 0x01 )
            {
                // Login screen -> when sw4 or sw5 is pressed, enter login screen
                clear_screen();
                clcd_print(" ENTER PASSWORD", LINE1(0));
                clcd_write(CURSOR_POS, INST_MODE);
                clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
                __delay_us(100);
                control_flag = LOGIN_FLAG;
                reset_flag = RESET_PASSWORD;
                TMR2ON = 1;

            }
        }
        
        // setting control flow when sw6 is pressed in login menu screen
        if((control_flag == 0xAA) && key == SW6) // LOGIN_MENU_FLAG
        {
            if(wait == 0)
            {
                switch(var)
                {
                    case 0x00:
                        // view log
                        control_flag = VIEW_LOG_FLAG;
                        first_time = 1;
                        break;

                    case 0x01:
                        //Clear log
                        control_flag = CLEAR_LOG;
                        break;

                    case 0x02:
                        //Download log
                        control_flag = DOWNLOAD_LOG;
                        break;

                    case 0x03:
                        // Set time
                        control_flag = SET_TIME;
                        break;
                        
                    case 0x04:
                        // Change Password
                        control_flag = CHANGE_PASSWORD;
                        
                        
                        break;
                }
                clear_screen();
                
                wait = 0;
                key = 0x00;
            }
            
        }
        
        switch(control_flag)
        {
            case 0x01: // Dashboard
                display_dash_board(event, speed);
                break;
                
            case 0x02: // Login Screen
                switch(login(key, reset_flag))
                {
                    case 0xA0 : // RETURN_BACK -> when wrong password is entered
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON = 0x00;
                        control_flag = DASH_BOARD;
                        break;
                    
                    case 0xB0 : //LOGIN SUCCESS -> when password is matched
                        control_flag = LOGIN_MENU_FLAG;
                        reset_flag = RESET_LOGIN_MENU;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON = 0x00;
                        continue; // Just so it doesn't get reset nothing after the switch case
                        break;
                }
                break;
                
            case 0xAA: // LOGIN MENU FLAG
                if(!first_time)
                {
                    clear_screen();
                }
                
                
                var = login_menu(key, reset_flag);
                
                if(key == SW1)
                {
                    // set control flow back to dashboard screen when sw1 is pressed
                    clear_screen();
                    control_flag = 0x01; // dashboard
                }
                // implement go back key
                first_time = 1;
                break;
                
            case 0xCC: // VIEW_LOG
                
                // This function will show the list of events that has been saved in EEPROM
                view_log(key);
                
                if(key == SW6)
                {
                    control_flag = LOGIN_MENU_FLAG;
                }
                break;
                
            case 0xDD: // Clear Log
                clear_log();
                d_flag = 1;
                if(key == SW6)
                {
                    control_flag = LOGIN_MENU_FLAG;
                    clear_screen();
                }
                break;
                
            case 0xEE: // Download Log
                
                if(!d_flag) // if data is available or not yet shared
                {
                    clcd_print("SENDING...", LINE1(2));
                    download_log();
                    d_flag = 1;
                    clear_screen();
                }
                else
                {
                    // when data is sent
                    clcd_print("SENT", LINE1(2));
                }
                
                if(key == SW6)
                {
                    // send control flow to menu when sw6 is pressed
                    control_flag = LOGIN_MENU_FLAG;
                    clear_screen();
                }
                break;
                
        case 0xFF: // Set time
            
            set_time(key);
            
            if(key == SW6)
            {
                control_flag = LOGIN_MENU_FLAG;
                clear_screen();
            }
            break;
            
        case 0xA1:
            
            change_password(key);
            
            
            if(key == SW6)
            {
                control_flag = LOGIN_MENU_FLAG;
                clear_screen();
                first_time = 1;
                TMR2ON = 0;
            }
            break;
        }
        reset_flag = RESET_NOTHING;
    }

}
