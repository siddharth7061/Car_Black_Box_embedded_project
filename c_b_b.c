#include "main.h"

unsigned char clock_reg[3];
char time[7];
unsigned char log[11];
unsigned char data[11];
char pos = -1;
char sec;
char return_time = 5;
unsigned char extern control_flag;
unsigned char c_log[] = "          ";
char c_pos = 0;
unsigned char clear_flag = 0;
char d_pos = 0;
char read_pos = 0;
unsigned char d_data[16];

//unsigned char extern first_time;

static void display_time()
{
    //BCD Format 
    //clock_reg[0] = HH
    //clock_reg[1] = MM
    //clock_reg[2] = SS
   
    
    
    clcd_putch(time[0], LINE2(0)); // HH:MM:SS
    clcd_putch(time[1], LINE2(1)); // HH:MM:SS 
    clcd_putch(':', LINE2(2)); // HH:MM:SS 
    clcd_putch(time[2], LINE2(3)); // HH:MM:SS 
    clcd_putch(time[3], LINE2(4)); // HH:MM:SS 
    clcd_putch(':', LINE2(5)); // HH:MM:SS 
    clcd_putch(time[4], LINE2(6)); // HH:MM:SS 
    clcd_putch(time[5], LINE2(7)); // HH:MM:SS 
}

static void get_time()
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 
    
    /* To store the time in HH:MM:SS format */
      // "HH:MM:SS"
    // HH -> 
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    
    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
}

void display_dash_board(unsigned char event[], char speed)
{
    clcd_print("  TIME     EV SP", LINE1(0));
    get_time();
    display_time();
    clcd_print(event, LINE2(11));
    
    if(strcmp(event, "ON") == 0)
    {
        speed = 0;
    }
    clcd_putch(speed / 10 + '0', LINE2(14));    
    clcd_putch(speed % 10 + '0', LINE2(15));

}

void log_event(void)
{
    clear_flag = 0;
    char addr = 0x05;
    pos++;
    if(pos == 10)
    {
        pos = 0;
    }
    addr = pos * 10 + 5;
    eeprom_at24c04_str_write(addr, log); // 0x05 -> HHMMSS0N00, 
}

void log_car_event(unsigned char *event, char speed)
{
    //time[] = "HH,,SS\0"
    //log[] = HHMMSSEVSP
    get_time();
    strncpy(log, time, 6); // HHMMSS
    strncpy(&log[6], event, 2); // HHMMSSON
    log[8] = speed/10 + '0';
    log[9] = speed%10 + '0';
    log[10] = '\0'; // HHMMSSON00
    log_event();
}

void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}

unsigned char login(unsigned char key, unsigned char reset_flag)
{
    static int attempt = 3;
    char npassword[4];
    static char i = 0;
    char spassword[4];
    if(reset_flag == 0x10) // RESET PASSWORD
    {
        attempt = 3;
        npassword[0] = '\0';
        npassword[1] = '\0';
        npassword[2] = '\0';
        npassword[3] = '\0';
        key = 0xFF;
        return_time = 5;
    }
    
    if(key == SW4 && i < 4)
    {
        npassword[i] = '1';
        clcd_putch('*', LINE2(6 + i));
        i++;
        return_time = 5;
    }
    else if(key == SW5 && i < 4)
    {
        npassword[i] = '0';
        clcd_putch('*', LINE2(6 + i));
        i++;
        return_time = 5;
    }
    if(i == 4) // All 4 keys are read
    {
        for(int j = 0; j < 4; j++)
        {
            spassword[j] = eeprom_at24c04_random_read(j);
        }
        if(strncmp(spassword, npassword, 4) == 0)
        {
            // Menu  Screen
            return LOGIN_SUCCESS;
        }
        else // When wrong password is entered
        {
            attempt--;
            if(attempt == 0) // when 0 attempts are left
            {
                // Lock for 1 minute
                TMR2ON = 1; // Turn on timer
                sec = 60;
                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("You are locked", LINE1(0));
                clcd_print("Wait for...60sec", LINE2(0));
                
                
                while(sec) // Display seconds remaining until 0 seconds are left
                {
                    clcd_putch(sec / 10 + '0', LINE2(11));
                    clcd_putch(sec % 10 + '0', LINE2(12));
                }
            }
            else // when you have attempts left
            {
                // Re-enter password
                clear_screen();
                clcd_print("Wrong Password", LINE1(0));
                clcd_putch(attempt + '0', LINE2(0));
                clcd_print("Attempt Remains", LINE2(2));
                __delay_ms(3000);
                
            }
            clear_screen();
            clcd_print("ENTER PASSWORD", LINE1(0));
            i = 0; // reset cursor 
            return_time = 5;
            clcd_write(CURSOR_POS, INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
        }
    }
    
    if(return_time == 0)
    {
        return RETURN_BACK; // returning this macro when user is taking longer than 5 seconds to enter passsword
    }
    
    return 0xFF; // Just return some value, nothing specific
}

char login_menu(unsigned char key, unsigned char reset_flag)
{
    char *menu[] = {"View log", "Clear log", "Download log", "Set time", "Change pwd"};
    static char menu_pos = 0;
    if(reset_flag == 0xBB) // RESET_LOGIN_MENU
    {
        menu_pos = 0;
        clear_screen();
    }
    if(key == SW5 && menu_pos <= 3)
    {
        menu_pos++;
        clear_screen();
    }
    else if(key == SW4 && menu_pos > 0)
    {
        menu_pos--;
        clear_screen();
    }
    if(menu_pos < 4)
    {
        clcd_putch('*', LINE1(0));
        clcd_print(menu[menu_pos], LINE1(2));
        clcd_print(menu[menu_pos + 1], LINE2(2));
    }
    if(menu_pos == 4)
    {
        clcd_print(menu[menu_pos - 1], LINE1(2));
        clcd_print(menu[menu_pos], LINE2(2));
        clcd_putch('*', LINE2(0));
    }
    
    return menu_pos;
}

void read_event(unsigned char key)
{
    unsigned char addr = 0x05;

    
    if(read_pos > 0 && key == SW4)
    {
        read_pos--;
    }
    if(key == SW5 && read_pos < pos)
    {
        read_pos++;
    }

    addr = read_pos * 10 + 5;
    data[0] = eeprom_at24c04_random_read(addr++); // 0x05 -> HHMMSS0N00,
    data[1] = eeprom_at24c04_random_read(addr++);
    data[2] = eeprom_at24c04_random_read(addr++);
    data[3] = eeprom_at24c04_random_read(addr++);
    data[4] = eeprom_at24c04_random_read(addr++);
    data[5] = eeprom_at24c04_random_read(addr++);
    data[6] = eeprom_at24c04_random_read(addr++);
    data[7] = eeprom_at24c04_random_read(addr++);
    data[8] = eeprom_at24c04_random_read(addr++);
    data[9] = eeprom_at24c04_random_read(addr++);
    data[10] = '\0';
    addr = read_pos * 10 + 5;

    /*if(read_pos == pos)
    {
        read_pos = -1;
    }*/
}

void view_log(unsigned char key)
{
    //static char flag = 1;
    
    if(first_time)
    {
        clear_screen();
        first_time = 0;
    }
    
    if(clear_flag)
    {
        clcd_print("NO DATA", LINE2(5));
    }
    else
    {
        read_event(key);
        clcd_print("  TIME     EV SP", LINE1(0));
        clcd_putch(data[0], LINE2(0));
        clcd_putch(data[1], LINE2(1));
        clcd_putch(':', LINE2(2));
        clcd_putch(data[2], LINE2(3));
        clcd_putch(data[3], LINE2(4));
        clcd_putch(':', LINE2(5));
        clcd_putch(data[4], LINE2(6));
        clcd_putch(data[5], LINE2(7));

        clcd_putch(data[6], LINE2(11));
        clcd_putch(data[7], LINE2(12));

        clcd_putch(data[8], LINE2(14));
        clcd_putch(data[9], LINE2(15)); 
    }
    
    
    
}

void clear_log(void)
{
    char addr = 0x05;
    
    for(; c_pos < pos; c_pos++)
    {
       addr = c_pos * 10 + 5;
       eeprom_at24c04_str_write(addr, c_log); // 0x05 -> HHMMSS0N00,  
    }
    
    if(c_pos == pos)
    {
        c_pos = 0;
        pos = 0;
    }
    
    clear_flag = 1;
    
    clcd_print("LOG CLEARED", LINE1(2));
}

void download_log(void)
{
    
    puts("  TIME       EV  SP\n");
    for(unsigned char addr = 0x05; d_pos <= pos; d_pos++)
    {
        addr = d_pos * 10 + 5;
        
        data[0] = eeprom_at24c04_random_read(addr++); // 0x05 -> HHMMSS0N00,
        data[1] = eeprom_at24c04_random_read(addr++);
        data[2] = ':';
        data[3] = eeprom_at24c04_random_read(addr++);
        data[4] = eeprom_at24c04_random_read(addr++);
        data[5] = ':';
        data[6] = eeprom_at24c04_random_read(addr++);
        data[7] = eeprom_at24c04_random_read(addr++);
        data[8] = ' ';
        data[9] = ' ';
        data[10] = ' ';
        data[11] = eeprom_at24c04_random_read(addr++);
        data[12] = eeprom_at24c04_random_read(addr++);
        data[13] = ' ';
        data[14] = eeprom_at24c04_random_read(addr++);
        data[15] = eeprom_at24c04_random_read(addr++);
        data[16] = '\0';
        
        //puts(data[1])
        puts(data);
        puts("\n");
    }
    
}

void set_time(unsigned char key)
{
    static unsigned char run = 1;
    static unsigned char b_pos = 0; // variable to track bar position
    char bar[2], change_flag = 0;
    bar[0] = 207 + '0';
    bar[1] = '\0';
    
    
    if(key == SW1)
    {
        run = !run;
        clear_screen();
    }
    
    if(run)
    {
        //RUN Mode
        clcd_print("SET TIME->RUN_MD", LINE1(0));
        if(change_flag)
        {
            // send data to RTC 
            clock_reg[0] = 0; // HH -> BCD 
            clock_reg[1] = 0; // MM -> BCD 
            clock_reg[2] = 0; // SS -> BCD 
            
            clock_reg[0] = (((clock_reg[0] | (time[0] - '0')) << 4) | (time[1] - '0'));
            clock_reg[1] = (((clock_reg[1] | (time[2] - '0')) << 4) | (time[3] - '0'));
            clock_reg[2] = (((clock_reg[2] | (time[4] - '0')) << 4) | (time[5] - '0'));
            
            write_ds1307(HOUR_ADDR, clock_reg[0]);
            write_ds1307(MIN_ADDR, clock_reg[1]);
            write_ds1307(SEC_ADDR, clock_reg[2]);
        }
        get_time();
        display_time();
        TMR2ON = 0;
        b_pos = 0;
        
        
        change_flag = 0;
    }
    else
    {
        // Edit Mode
        TMR2ON = 1;
        clcd_print("EDIT MODE", LINE1(3));
        
        if(key == SW2) // BACKWARD CURSOR SHIFT
        {
            if(b_pos > 0)
            {
                if(b_pos == 3 || b_pos == 6)
                {
                    // decrement by 2 position to avoid ':'
                    b_pos = b_pos - 2;
                }
                else
                {
                    // decrement cursor by 1 position
                    b_pos--;
                }
                
            }
        }
        if(key == SW3) // FORWARD CURSOR SHIFT
        {
            // Move cursor ahead
            if(b_pos < 7)
            {
                if(b_pos == 1 || b_pos == 4)
                {
                    // incrementing cursor position by 2 positions
                    b_pos = b_pos + 2;
                }
                else
                {
                    // incrementing cursor position by 1 position
                    b_pos++;
                }
            }
        }
        
        if(key == SW4)
        {
            // Increment time value
            if(b_pos < 2) // HOURS
            {
                if(b_pos == 0) // b_pos = 0 -> H0
                {
                    if(time[0] < 50) // increment upto 2
                    {
                        time[0]++;
                        change_flag = 1;
                    }
                }
                else // b_pos = 1 -> 0H
                {
                    if(time[0] < 50) // 0H or 1H
                    {
                        if(time[1] < 57)
                        {
                            time[1]++; // increment upto 9 when 10ths place hour is 1
                            change_flag = 1;
                        }
                    }
                    else // 2H
                    {
                        if(time[1] < 51)
                        {
                            time[1]++; // increment upto 3 when 10ths place hour is 2
                            change_flag = 1;
                        }
                    }
                }
            }
            
            if(b_pos == 3) // MINUTES
            {
                if(time[2] < 53) // H0 -> Increment upto 5
                {
                    time[2]++; // position 1 less than the original
                    change_flag = 1;
                }
            }
            
            if(b_pos == 4) // MINUTES
            {
                if(time[3] < 57) // 0H -> Increment upto 9
                {
                    time[3]++; // position 1 less than the original
                    change_flag = 1;
                }
            }
            
            if(b_pos == 6) // SECONDS
            {
                if(time[4] < 53) // S0 -> Increment upto 5
                {
                    time[4]++;
                    change_flag = 1;
                }
            }
            
            if(b_pos == 7) // SECONDS
            {
                if(time[5] < 57) // 0S -> Increment upto 9
                {
                    time[5]++;
                    change_flag = 1;
                }
            }
        }
        
        if(key == SW5)
        {
            // Decrement time value
            if(b_pos < 2) // HOURS
            {
                /*if(b_pos == 0) // H0
                {
                    if(time[b_pos] > 0)
                    {
                        time[b_pos]--;
                    }
                }
                else // 0H
                {
                    
                }*/
                if(time[b_pos] > 48)
                {
                    time[b_pos]--;
                    change_flag = 1;
                }
            }
            
            if(b_pos == 3 || b_pos == 4) // MINUTES
            {
                if(time[b_pos - 1] > 48)
                {
                    time[b_pos - 1]--;
                    change_flag = 1;
                }
            }
            
            if(b_pos > 5) // SECONDS
            {
                if(time[b_pos - 2] > 48)
                {
                    time[b_pos - 2]--;
                    change_flag = 1;
                }
            }
        }
        
        if(blink)
        {
            // Display bar at that location -> b_pos
            clcd_putch(bar[0], LINE2(b_pos));
            
        }
        else
        {
            // display number at that location
            clcd_putch(time[0], LINE2(0)); // HH:MM:SS
            clcd_putch(time[1], LINE2(1)); // HH:MM:SS 
            clcd_putch(':', LINE2(2)); // HH:MM:SS 
            clcd_putch(time[2], LINE2(3)); // HH:MM:SS 
            clcd_putch(time[3], LINE2(4)); // HH:MM:SS 
            clcd_putch(':', LINE2(5)); // HH:MM:SS 
            clcd_putch(time[4], LINE2(6)); // HH:MM:SS 
            clcd_putch(time[5], LINE2(7)); // HH:MM:SS
            
            clcd_putch(time[0], LINE2(9)); // HH:MM:SS
            clcd_putch(time[1], LINE2(10)); // HH:MM:SS 
            //clcd_putch(':', LINE2(2)); // HH:MM:SS 
            clcd_putch(time[2], LINE2(11)); // HH:MM:SS 
            clcd_putch(time[3], LINE2(12)); // HH:MM:SS 
            //clcd_putch(':', LINE2(5)); // HH:MM:SS 
            clcd_putch(time[4], LINE2(13)); // HH:MM:SS 
            clcd_putch(time[5], LINE2(14)); // HH:MM:SS
        }
        
        
    }
}

void change_password(unsigned char key)
{
    char npassword[4], bar[2];
    static char i = 0;
    unsigned static char display = 1;
    bar[0] = 207 + '0';
    bar[1] = '\0';
    
    if(first_time)
    {
        clcd_print("  NEW PASSWORD", LINE1(0));
        npassword[0] = ' ';
        npassword[1] = '\0';
        npassword[2] = '\0';
        npassword[3] = '\0';
        first_time = 0;
        display = 1;
        TMR2ON = 1;
    }
    
    if(i == 4)
    {
        /*for(int j = 0; j < 4; j++)
        {
            npassword[j] = eeprom_at24c04_random_read(j);
        }*/
        eeprom_at24c04_str_write(0x00, npassword);
        i = 0;
        clear_screen();
        clcd_print("PASSWORD CHANGED", LINE1(0));
        display = 0;
        TMR2ON = 0;
    }
    
    if(key == SW4 && i < 4)
    {
        npassword[i] = '1';
        clcd_putch('*', LINE2(6 + i));
        i++;
    }
    
    if(key == SW5 && i < 4)
    {
        npassword[i] = '0';
        clcd_putch('*', LINE2(6 + i));
        i++;
    }
    
    if(display)
    {
        if(blink)
        {
            clcd_putch(bar[0], LINE2(6 + i));
        }
        else
        {
            clcd_putch(npassword[i], LINE2(6 + i));
        }
    }
    
    
}