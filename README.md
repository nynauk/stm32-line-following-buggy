# Line-following buggy (STM32, bare-metal C)

Firmware for an autonomous line-following buggy, built by a team of five (Group 10) for the embedded systems
module of my BEng at Royal Holloway. It runs on an STM32 NUCLEO-F091RC (Arm Cortex-M0 at 48 MHz) and is written
bare-metal: no HAL calls in the application, just the CMSIS register definitions. My part was testing,
debugging and integration.

## What it does

- **Line following.** Two light-dependent resistors read through the ADC (PA4 and PB0, 12-bit). Each reading is
  scaled to 0 to 1 and thresholded at 0.5. One sensor on the line drives only the opposite motor to steer back;
  both on the line keeps turning in the direction of the last sensor that saw white; neither runs both motors
  at half speed.
- **Differential drive.** TIM3 generates PWM on two channels (PB4 and PC7) at 1 kHz (prescaler 47 for 1 µs
  counts, period 1,000), one per motor, with direction pins on PB10 and PB6.
- **Obstacle detection.** An HC-SR04 ultrasonic sensor (trigger PA0, echo PA1) is pinged every loop. When an
  obstacle is close the buggy stops, lights its hazard LEDs, creeps forward slowly until the path clears, then
  resumes.
- **Controls and lights.** The user button on PC13 toggles line-following on a short press and runs a demo
  routine (forward, back, left, right) on a press of 300 ms or more. A brake light comes on whenever the motors
  are stopped.
- **Timing.** `delay_ms` counts SysTick at 48,000 cycles per millisecond; `delayuS` polls the TIM1 update flag
  with TIM1 ticking at 1 MHz.

## Files

```
firmware/main.c          clocks, GPIO, TIM3 PWM, TIM1 timebase, ADC setup and the main control loop
firmware/ultrasonic.c    echo pulse measurement with a timeout
firmware/demo.c          the demo manoeuvre sequence
firmware/delay.c         SysTick millisecond and TIM1 microsecond delays
firmware/config.h        bit-mask and LED macros
firmware/pwmv2.uvprojx   Keil µVision project (with RTE/ device startup files)
```

## Build

Open `firmware/pwmv2.uvprojx` in Keil µVision 5 with the STM32F0 device pack installed, build, and flash the
NUCLEO-F091RC over its on-board ST-LINK.

## Known issues

- The echo time in microseconds divided by 58 gives centimetres, but the comment in `main.c` calls it metres, so
  the 2.5 threshold actually stops the buggy at about 2.5 cm.
- Echo measurement and delays are busy-waits, so the line sensors are not read while the buggy is waiting for
  an echo. Input capture on a timer channel would remove that.
- The steering is bang-bang (one motor on, one off). A proportional controller on the difference between the
  two sensor readings would give a smoother line.
