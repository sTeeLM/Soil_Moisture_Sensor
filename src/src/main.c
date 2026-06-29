/**
 * CPU: 89C52
 * Freq: 12MHz
*/

#include <stc8g.h>
#include <stdio.h>

#include "system.h"

void delay_100ms(void) //@12MHz
{
    unsigned char i, j;

    i = 195;
    j = 138;

    do
    {
        while (--j)
            ;
    } while (--i);
}

void main(void)
{
    void system_init(void);

    while (1)
    {
        P12 = !P12;
        delay_100ms();
    }
}
