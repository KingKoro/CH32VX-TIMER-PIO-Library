#ifndef __CH32V_TIMER_H
#define __CH32V_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "debug.h"

// ---------- Systick to measure MCU time ----------

// Global System Timebase Precision (increment global systick counter every second, millisecond or microsecond)
// Higher precision means more counter increment ISR calls
enum {
	SYSTICK_SECONDS = 0,
	SYSTICK_MILLIS,
	SYSTICK_MICROS
};

extern void systick_init(uint32_t iPrecision);
extern inline uint64_t systick_get();


// ---------- Basic Timer Initializer (No PWM, only Trigger source for ADC or IRQ) ----------

enum {
	TCONF_TIM1 = 0,
	TCONF_TIM2,
	TCONF_TIM3,
	TCONF_TIM4
};

#define TCONF_IRQ 			1
#define TCONF_NO_IRQ		0

// Initializer function for basic Timer (also let iCount default to 254 if not specified)
int basic_timer_init_base(uint8_t iTimer, uint32_t iF_base, uint8_t iIRQ, void (*func)(void), uint16_t iCount);
// input structure for variadic args
typedef struct 
{
    uint8_t iTimer;
    uint32_t iF_base;
	uint8_t iIRQ;
	void (*func)(void);
    uint16_t iCount;
} init_timer_args;
// placeholder for default args
int var_basic_timer_init(init_timer_args in);
// Variable arguments version of basic_timer_init_base()
/*********************************************************************
 * @fn      basic_timer_init_base
 *
 * @brief   Initialize Timer for basic operation (counting up, no PWM output, trigger source for ADC or IRQ)
 *              Output_Clock = SystemCoreClock / (SystemCoreClock / iCount / iF_base) / (iCount + 1)
 * 
 * @param   iTimer      Timer to setup (TCONF_TIM1, TCONF_TIM2, TCONF_TIM3 or TCONF_TIM4)
 * @param   iF_base     Base frequency for Timer to trigger at when counter is filled up
 *                      WARNIGN: Due to integer divison, the actual frequency only roughly follows the specified one.
 * @param   iIRQ        Wether to generate Interrupt on counter fill up (TCONF_IRQ) or not (TCONF_NO_IRQ)
 * @param   func        Pointer to function which shall be called upon timer up interrupt, must be return type void and parameter void. Declare or define before passing it here as function name.
 * @param   iCount      Base for scaling counter (Defaults to 254 to be compatible with PWM-Library's 8-Bit default iCount of 254, for more precise even frequencies, use even dividers of SystemCoreClock (multiples of 2))
 *
 * @return  Normally returns 0 on exit
 */
#define basic_timer_init(...) var_basic_timer_init((init_timer_args){__VA_ARGS__});

#ifdef __cplusplus
}
#endif

#endif