# CH32VX-TIMER-PIO-Library
 A library for initializing basic timers and tracking global timing on CH32V/X MCUs.

## Note
- The actual timer frequency might differ from the specified frequency, by various offsets, due to integer division.
    - This is because by default, the counter for the timer ``ìCount``, a parameter of ```basic_timer_init()```, is set to 254 to make it compatible with my [PWM Library's](https://github.com/KingKoro/CH32VX-PWM-PIO-Library)
        default ``ìCount`` of 254 (results in an 8-Bit range of 0 - 255 for the dutycycle). If your application does not require to run a PWM output on the same timer as the one used with ```basic_timer_init()```, or you are fine
        with adapting the dutycycle range of said PWM outputs, then you can choose a different ``ìCount`` that is an even divider of SystemCoreClock and generate more precise clocks.
        An Example for this issue:
        ```C
        // 96000000 / (96000000 / 4 / 1250) / (4 + 1) = 1kHz Output (precisely 1kHz)
        basic_timer_init(TCONF_TIM3, 1250, TCONF_IRQ, my_function, 4);
        // 96000000 / (96000000 / 254 / 1004) / (254 + 1) = ~1kHz Output (actually 1000.0627451 Hz, measured 998,6 Hz)
        basic_timer_init(TCONF_TIM3, 1006, TCONF_IRQ, my_function);
        ```
- When initializing a PWM output on an already initialized timer, the timer prescaler and counter are overwritten, therefore the timer's frequency might change as well.
    An Example for this issue:
    ```C
    // 96000000 / (96000000 / 254 / 1004) / (254 + 1) = ~1kHz Output
    basic_timer_init(TCONF_TIM3, 1004, TCONF_IRQ, my_function);
    // initialize PWM on PA6 (TIM3_CH1) with ~2kHz Base frequency -> This overwrites the 1kHz Base Frequency from the previous Timer 3 Initialization!
    PWM_handle PWM_A6={0};
    init_pwm(&PWM_A6, PWM_TIM3, PWM_CH1, 0xA6, 2000);
    set_pwm_dutycycle(&PWM_A6, 200);              // set  PWM duty cycle to 200/255 initially
    enable_pwm_output(&PWM_A6);                 // start PWM output
    ```
## Warning
If your project is also utilizing my [CH32V/X USB-Serial Library](https://github.com/KingKoro/CH32VX-USB-Serial-PIO-Library), be careful when selecting a timer to configure. Do not use a timer that is already used for asynchronous USB communication (avoid TIM2 on CH32V1xx, V2xx and V3xx or TIM3 on CH32X03x), as this can mess with the USB's TX update rate.

## Installation
### Prequisites
You need to have PlatformIO VSCode Plugin with the [WCH CH32V](https://github.com/Community-PIO-CH32V/platform-ch32v) Platform installed.
## Setup
Simply clone this repository onto your computer and open the folder like a regular PlatformIO project. You can try out the example in ```main.c```, if you have a CH32V203C8T6-EVT-R0 at hand.

Alternativly, you can also copy the library ```lib/CH32V_TIMER``` into any PIO project, modify your ```platformio.ini``` if needed and import the library into your ```main.c``` with: 
```c
#include "ch32v_timer.h"
```
# Usage

Import ```ch32v_timer.h```, then initialize a basic timer with ```basic_timer_init()```. The first argument is the Timer to select (either TCONF_TIM1, TCONF_TIM2, TCONF_TIM3 or TCONF_TIM4), followed by the frequency in Hz, wether to generate an interrupt for the timer (yes = TCONF_IRQ, no = TCONF_NO_IRQ, together with an optional argument ```func```, which points to the function to execute on interrupt) and optionally, specify a custom ``iCount`` for more precise frequencies.

Now you can compile and upload the project.

## Methods for tracking global timing:

### Method 1: Systick Timer

If you want to use the global systick to track timing of your code, initialize the systick counter with ``systick_init()`` and select either SYSTICK_SECONDS, SYSTICK_MILLIS, SYSTICK_MICROS or a custom timebase to count time in seconds, milliseconds, microseconds or arbitrary units respectively. ``systick_get()`` returns the current counter value. This approach enables in the best resolution down to +/- 1us. However, it can be incompatible with certain libraries, especially if they use the ``Delay_Us()`` or ``Delay_Ms()`` functionality, which already utilizes the Systick Timer and can cause ``systick_get()`` to get stuck at a certain value. Due to this incompatibility, systick timing tracking is disabled by default, to enable it, uncomment line 12 ``#define GLOBAL_TIMING_USE_SYSYTICK`` in ``ch32v_timer.h``.

Note: Recalling ``systick_init()`` periodically can maybe help in certain situations.
Note: Using ``#define USB_TX_MODE USB_TX_SYNC`` in the USB serial library can maybe help in certain situations.

Check out the function descriptions for more details. See the example in ``main.c``.

### Method 2: Separate Timer ISR

Using any other general or advanced timer for incrementing a global time counter is also a possible way of tracking timing. Use ``basic_timer_init()`` to initialize a timer for this purpose or reuse an existinig ISR. Depending on the MCU utilization with other interrupts and libraries, the highest possible timer frequency is 100kHz to 200kHz, resulting in a resolution of +/- 5us to 10us.

Check out the function descriptions for more details. See the example in ``main.c``.

### Method 3: Endless While-Loop with delay

If you don't need to use the endless-while loop of your main function to do anything else, you can repeatidly call the ``Delay_Us()`` or ``Delay_Ms()`` method and increment a global time counter. This methods achieves usable time values down to a resolution of +/- 50us to 100us.

Check out the function descriptions for more details. See the example in ``main.c``.

## Overview

The available functions are:
```C++
int basic_timer_init_base(uint8_t iTimer, uint32_t iF_base, uint8_t iIRQ, void (*func)(void) = NULL, uint16_t iCount = 254)    /* Initialize basic timer */

void systick_init(int iPrecision);                                  /* Initialize systick conuter */
uint64_t systick_get();                                             /* Get systick conuter */

```

# Example

The example in this PIO project shows how to configure Timer 3 as a basic timer that counts up, triggers an Interrupt and resets itself again, over and over. When it goes into the ISR, it measures the elapsed time since the last interrupt with the systick counter and prints that value out via UART.

At a frequency of roughly 1kHz, the systick measures roughly 1000 microseconds of elapsed time between the interrupts. When attention to a precise frequency is paid (as described above), timing can be precisly evaluated down to +/- 1us.

## Supported MCUs
This library was only tested on the CH32V203C8T6-EVT-R0, but should work on any CH32V-family or CH32X-family of MCUs. It should be compatible with the NoneOS-SDK and possibly the Arduino Framework as well.

# Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.