#include "demo.h"
#include "stm32f091xc.h"
#include "config.h"
#include "delays.h"

void runDemoMode(void)
{
    uint32_t demoSpeed = 840;
    uint32_t maxSpeed = 1000;
    
    // Drive Forward
    GPIOB->ODR &= ~MASK(10);
    GPIOB->ODR &= ~MASK(6);
    TIM3->CCR1 = demoSpeed;
    TIM3->CCR2 = demoSpeed;
    delay_ms(300);
    
    // Drive Backward
    GPIOB->ODR |= MASK(10);
    GPIOB->ODR |= MASK(6);
    TIM3->CCR1 = demoSpeed;
    TIM3->CCR2 = demoSpeed;
    delay_ms(300);
    
    // Turn Left (only right motor on)
    GPIOB->ODR &= ~MASK(10);
    GPIOB->ODR &= ~MASK(6);
    TIM3->CCR1 = 0;
    TIM3->CCR2 = demoSpeed;
    delay_ms(300);
    
    // Turn Right (only left motor on)
    GPIOB->ODR &= ~MASK(10);
    GPIOB->ODR &= ~MASK(6);
    TIM3->CCR1 = demoSpeed;
    TIM3->CCR2 = 0;
    delay_ms(300);
    
    // Spin Clockwise (left forward, right reverse)
    GPIOB->ODR &= ~MASK(10);
    GPIOB->ODR |= MASK(6);
    TIM3->CCR1 = demoSpeed;
    TIM3->CCR2 = maxSpeed;
    delay_ms(300);
    
    // Spin Anticlockwise (left reverse, right forward)
    GPIOB->ODR |= MASK(10);
    GPIOB->ODR &= ~MASK(6);
    TIM3->CCR1 = maxSpeed;
    TIM3->CCR2 = demoSpeed;
    delay_ms(300);
    
    // Stop motors after demo sequence
    TIM3->CCR1 = 0;
    TIM3->CCR2 = 0;
    GPIOB->ODR &= ~MASK(10);
    GPIOB->ODR &= ~MASK(6);
}
