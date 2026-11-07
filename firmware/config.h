#ifndef CONFIG_H
#define CONFIG_H

#include "stm32f091xc.h"

// --- Macros ---
// Bit masking macro
#define MASK(x) (1UL << (x))

// Ultrasonic Sensor Macros
#define TRIG_HIGH    (GPIOA->BSRR = GPIO_BSRR_BS_0)
#define TRIG_LOW     (GPIOA->BSRR = GPIO_BSRR_BR_0)

// LED Macros
// LED D2 acts as the brake/hazard light, LED D3 and LED D4 indicate hazard status.
#define LED_D2_ON    (GPIOA->BSRR = GPIO_BSRR_BS_10)
#define LED_D2_OFF   (GPIOA->BSRR = GPIO_BSRR_BR_10)
#define LED_D3_ON    (GPIOB->BSRR = GPIO_BSRR_BS_3)
#define LED_D3_OFF   (GPIOB->BSRR = GPIO_BSRR_BR_3)
#define LED_D4_ON    (GPIOB->BSRR = GPIO_BSRR_BS_5)
#define LED_D4_OFF   (GPIOB->BSRR = GPIO_BSRR_BR_5)

#endif // CONFIG_H
