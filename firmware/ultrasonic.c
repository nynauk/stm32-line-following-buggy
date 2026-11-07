#include "ultrasonic.h"
#include "delays.h"
#include "stm32f091xc.h"

uint32_t read_echo(uint32_t timeout)
{
    uint32_t duration = 0;
    while (!(GPIOA->IDR & GPIO_IDR_1))
    {
        duration++;
        delayuS(1);
        if (duration > timeout)
            return 0;
    }
    duration = 0;
    while (GPIOA->IDR & GPIO_IDR_1)
    {
        duration++;
        delayuS(1);
        if (duration > timeout)
            return 0;
    }
    return duration;
}
