/* 
 * File:   c_b_b.h
 * Author: siddharthsingh
 *
 * Created on February 8, 2023, 11:52 PM
 */

#ifndef C_B_B_H
#define	C_B_B_H

void display_dash_board(unsigned char event[], char speed);
void log_car_event(unsigned char *event, char speed);
void clear_screen(void);
unsigned char login(unsigned char key, unsigned char reset_flag);
char login_menu(unsigned char key, unsigned char reset_flag);
void view_log(unsigned char key);
void clear_log(void);
void read_event(unsigned char key);
void download_log(void);
void set_time(unsigned char key);
void change_password(unsigned char key);





#endif	/* C_B_B_H */

