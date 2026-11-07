#include "delays.h"
#include "stm32f091xc.h"

void delay_ms(uint32_t ms)
{
    SysTick->LOAD = 48000 - 1;  // 48MHz: 48000 cycles per ~1ms
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    for (uint32_t i = 0; i < ms; i++)
    {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) { }
    }
    SysTick->CTRL = 0;
}

void delayuS(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++)
    {
        while (!(TIM1->SR & TIM_SR_UIF)) { }
        TIM1->SR &= ~TIM_SR_UIF;
    }
}
