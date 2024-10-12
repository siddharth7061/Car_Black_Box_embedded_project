#include <xc.h>



void init_timer2(void)
{
    /* Selecting the scale as 1:1 */
    T2CKPS0 = 0;
    T2CKPS1 = 0;

    /* Loading the Pre Load register with 250 */
    PR2 = 250;
    
    /* The timer interrupt is enabled */
    TMR2IE = 1;
       
    /* Switching on the Timer2 */
    //TMR2ON = 1;
}