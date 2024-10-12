/* 
 * File:   main.h
 * Author: siddharthsingh
 *
 * Created on February 8, 2023, 9:21 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#include <xc.h>
#include <string.h>
#include "digital_keypad.h"
#include "clcd.h"
#include "adc.h"
#include "i2c.h"
#include "ds1307.h"
#include "c_b_b.h"
#include "at24c04.h"
#include "timers.h"
#include "uart.h"

#define DASH_BOARD                        0x01;
#define LOGIN_FLAG                        0X02;

#define RESET_PASSWORD                    0x10;
#define RESET_NOTHING                     0x20;

#define RETURN_BACK                       0xA0;
#define LOGIN_SUCCESS                     0xB0;

#define LOGIN_MENU_FLAG                   0xAA;
#define RESET_LOGIN_MENU                  0xBB;

#define VIEW_LOG_FLAG                     0xCC;
#define CLEAR_LOG                         0xDD;
#define DOWNLOAD_LOG                      0xEE;
#define SET_TIME                          0xFF;
#define CHANGE_PASSWORD                   0xA1;


 char extern flag = 1;
 unsigned char extern first_time = 1;
 unsigned char extern blink = 1;

#endif	/* MAIN_H */

