#include "stm32f091xc.h"
#include "config.h"
#include "delays.h"
#include "ultrasonic.h"
#include "demo.h"
#include <stdint.h>

// Global Variables
int toggle = 0;           // 1 = line-following mode active
int lastWhiteSensor = 0;  // For LDR sensor history in line-following logic
int hazardActive = 0;     // 1 = hazard state active

int main(void)
{
    // --- 1. Enable Clocks ---
    RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN);
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;    // For PWM generation
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;      // For microsecond timing

    // --- 2. Configure Motor PWM and I/O ---
    // Motor1 PWM on PB4 (AF1 for TIM3_CH1)
    GPIOB->MODER &= ~(0x3 << (4 * 2));
    GPIOB->MODER |=  (0x2 << (4 * 2));
    GPIOB->AFR[0] &= ~(0xF << (4 * 4));
    GPIOB->AFR[0] |=  (0x1 << (4 * 4));
    // Motor2 PWM on PC7 (assumed AF0 for TIM3_CH2)
    GPIOC->MODER &= ~(0x3 << (7 * 2));
    GPIOC->MODER |=  (0x2 << (7 * 2));
    GPIOC->AFR[0] &= ~(0xF << (7 * 4));
    
    // Configure motor direction pins as outputs:
    // Motor1 reverse control on PB10 (active low for forward)
    GPIOB->MODER &= ~(0x3 << (10 * 2));
    GPIOB->MODER |=  (0x1 << (10 * 2));
    // Motor2 reverse control on PB6 (active low for forward)
    GPIOB->MODER &= ~(0x3 << (6 * 2));
    GPIOB->MODER |=  (0x1 << (6 * 2));
    
    // Motor status LED on PA5 as output
    GPIOA->MODER &= ~(0x3 << (5 * 2));
    GPIOA->MODER |=  (0x1 << (5 * 2));
    
    // Configure push button PC13 as input
    GPIOC->MODER &= ~(0x3 << (13 * 2));
    
    // --- 3. Configure Ultrasonic Sensor I/O ---
    // PA0 as output (Trigger) and PA1 as input (Echo)
    GPIOA->MODER &= ~((3 << (0 * 2)) | (3 << (1 * 2)));
    GPIOA->MODER |=  (1 << (0 * 2));
    
    // --- 4. Configure Sensor LEDs ---
    // LED D2 (brake/hazard light) on PA10
    GPIOA->MODER &= ~(3 << (10 * 2));
    GPIOA->MODER |=  (1 << (10 * 2));
    // LED D3 on PB3:
    GPIOB->MODER &= ~(3 << (3 * 2));
    GPIOB->MODER |=  (1 << (3 * 2));
    // LED D4 on PB5:
    GPIOB->MODER &= ~(3 << (5 * 2));
    GPIOB->MODER |=  (1 << (5 * 2));
    
    // --- 5. Configure TIM3 for PWM ---
    TIM3->PSC = 47;       // 48MHz/48 = 1µs per count
    TIM3->ARR = 1000;     // PWM period: 1000 counts
    TIM3->CCR1 = 0;
    TIM3->CCR2 = 0;
    // PWM mode 1 for Channel 1 (PB4)
    TIM3->CCMR1 &= ~(0x7 << 4);
    TIM3->CCMR1 |=  ((6 << 4) | (1 << 3));
    // PWM mode 1 for Channel 2 (PC7)
    TIM3->CCMR1 &= ~(0x7 << 12);
    TIM3->CCMR1 |=  ((6 << 12) | (1 << 11));
    // Enable PWM outputs:
    TIM3->CCER |= 1;
    TIM3->CCER |= (1 << 4);
    TIM3->CR1 |= (1 << 7);
    TIM3->CR1 |= 1;
    // Set motor directions to forward (active low)
    GPIOB->ODR &= ~MASK(10);
    GPIOB->ODR &= ~MASK(6);
    
    // --- 6. Configure TIM1 for microsecond timing ---
    TIM1->PSC = 48 - 1;
    TIM1->ARR = 1;
    TIM1->CNT = 0;
    TIM1->CR1 = TIM_CR1_CEN;
    
    // --- 7. Configure ADC for LDR Sensors (for line following) ---
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN;
    GPIOA->MODER |= (3 << (4 * 2));   // PA4 for LDR1
    GPIOB->MODER |= (3 << (0 * 2));   // PB0 for LDR2
    delay_ms(1);
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}
    
    // --- Button Detection Variables ---
    uint32_t buttonHoldTime = 0;
    uint8_t buttonPrevState = 0;
    uint8_t demoModeActive = 0;
    
    // --- Main Loop ---
    while (1)
    {
        // --- Button Handling ---
        // PC13: short press toggles line-following mode; long press (>=300ms) starts demo mode.
        if ((GPIOC->IDR & MASK(13)) == 0)  // Button pressed (active low)
        {
            if (!buttonPrevState)
            {
                buttonHoldTime = 0;
                buttonPrevState = 1;
            }
            else
            {
                buttonHoldTime += 10;
                if (buttonHoldTime >= 300 && !demoModeActive)
                {
                    demoModeActive = 1;
                    runDemoMode();
                    // After demo mode, return to stationary mode.
                    toggle = 0;
                    buttonHoldTime = 0;
                    buttonPrevState = 0;
                }
            }
        }
        else
        {
            // On short press, toggle line-following mode.
            if (buttonPrevState && buttonHoldTime < 300 && !demoModeActive)
            {
                toggle = !toggle;
                delay_ms(50);
            }
            buttonPrevState = 0;
            buttonHoldTime = 0;
            demoModeActive = 0;
        }
        
        // --- Process Motion: Only execute when line-following mode is active ---
        if (toggle)
        {
            // --- Hazard Detection (Ultrasonic Sensor) ---
            TRIG_LOW;
            delayuS(2);
            TRIG_HIGH;
            delayuS(10);
            TRIG_LOW;
            uint32_t sensorDuration = read_echo(400000);
            float distance = sensorDuration / 58.0f;  // Distance in meters
            
            if (distance <= 2.5f)
            {
                // Hazard detected: mark hazard active and turn on all LEDs.
                hazardActive = 1;
                LED_D2_ON;
                LED_D3_ON;
                LED_D4_ON;
                // Stop the motors.
                TIM3->CCR1 = 0;
                TIM3->CCR2 = 0;
                delay_ms(500);
                
                // Drive slowly until hazard clears.
                uint32_t slowSpeed = 300;  // Adjust slow forward speed as needed
                while (1)
                {
                    TRIG_LOW;
                    delayuS(2);
                    TRIG_HIGH;
                    delayuS(10);
                    TRIG_LOW;
                    sensorDuration = read_echo(400000);
                    float currentDistance = sensorDuration / 58.0f;
                    if (currentDistance > 2.5f)
                        break;  // Hazard cleared
                    
                    TIM3->CCR1 = slowSpeed;
                    TIM3->CCR2 = slowSpeed;
                    delay_ms(50);
                }
                // Hazard cleared: stop motors and turn off hazard indicators.
                TIM3->CCR1 = 0;
                TIM3->CCR2 = 0;
                LED_D3_OFF;
                LED_D4_OFF;
                hazardActive = 0;
            }
            
            // --- Line Following Logic using LDR Sensors ---
            uint16_t ldr1_val, ldr2_val;
            // Read LDR1 from PA4 (ADC channel 4)
            ADC1->CHSELR = (1 << 4);
            ADC1->CR |= ADC_CR_ADSTART;
            while (!(ADC1->ISR & ADC_ISR_EOC)) { }
            ldr1_val = ADC1->DR;
            
            // Read LDR2 from PB0 (ADC channel 8)
            ADC1->CHSELR = (1 << 8);
            ADC1->CR |= ADC_CR_ADSTART;
            while (!(ADC1->ISR & ADC_ISR_EOC)) { }
            ldr2_val = ADC1->DR;
            
            float ldr1_percentage = ldr1_val / 4095.0f;
            float ldr2_percentage = ldr2_val / 4095.0f;
            float threshold = 0.5f;
            uint8_t ldr1_digital = (ldr1_percentage >= threshold) ? 1 : 0;
            uint8_t ldr2_digital = (ldr2_percentage >= threshold) ? 1 : 0;
            
            if (ldr1_digital == 1 && ldr2_digital == 0)
                lastWhiteSensor = 1;
            else if (ldr1_digital == 0 && ldr2_digital == 1)
                lastWhiteSensor = 2;
            else if (ldr1_digital == 1 && ldr2_digital == 1)
                if (lastWhiteSensor == 0) lastWhiteSensor = 1;
            
            uint32_t baseMotorSpeed = 840;
            uint32_t finalMotorSpeed1, finalMotorSpeed2;
            
            if (ldr1_digital == 1 && ldr2_digital == 0)
            {
                finalMotorSpeed1 = baseMotorSpeed;
                finalMotorSpeed2 = 0;
            }
            else if (ldr1_digital == 0 && ldr2_digital == 1)
            {
                finalMotorSpeed1 = 0;
                finalMotorSpeed2 = baseMotorSpeed;
            }
            else if (ldr1_digital == 1 && ldr2_digital == 1)
            {
                if (lastWhiteSensor == 1)
                {
                    finalMotorSpeed1 = baseMotorSpeed;
                    finalMotorSpeed2 = 0;
                }
                else
                {
                    finalMotorSpeed1 = 0;
                    finalMotorSpeed2 = baseMotorSpeed;
                }
            }
            else
            {
                finalMotorSpeed1 = baseMotorSpeed / 2;
                finalMotorSpeed2 = baseMotorSpeed / 2;
            }
            
            TIM3->CCR1 = finalMotorSpeed1;
            TIM3->CCR2 = finalMotorSpeed2;
            GPIOA->ODR |= MASK(5);  // Motor status LED ON (line-following mode)
        }
        else
        {
            // Not in line-following mode: ensure motors and hazard indicators are off.
            TIM3->CCR1 = 0;
            TIM3->CCR2 = 0;
            LED_D3_OFF;
            LED_D4_OFF;
            GPIOA->ODR &= ~MASK(5);
        }
        
        // --- Brake Lights Control (LED D2) ---
        // LED D2 turns on when either a hazard is active or the motors are stopped.
        if (hazardActive || (TIM3->CCR1 == 0 && TIM3->CCR2 == 0))
            LED_D2_ON;
        else
            LED_D2_OFF;
        
        delay_ms(10);
    }
    
    return 0;
}
