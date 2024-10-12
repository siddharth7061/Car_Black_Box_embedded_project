#include <xc.h>
#include "main.h"
extern char sec;
extern char return_time;
void __interrupt() isr(void)
{
    static unsigned long int count = 0;
    
    if (TMR2IF == 1)
    {
        if(count == 10000)
        {
            blink = !blink;
        }
        
        if (++count == 20000)
        {
            blink = !blink;
            count = 0;
            if(sec > 0)
            {
                sec--;
            }
            
            if(sec == 0 && return_time > 0)
            {
                return_time--;
            }
        }
        
        TMR2IF = 0;
    }
}