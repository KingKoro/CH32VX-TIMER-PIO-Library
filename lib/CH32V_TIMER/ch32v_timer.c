#include "ch32v_timer.h"


void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

uint64_t global_systick_counter = 0;

void systick_init(int iPrecision)
{
    SysTick->SR = 0;
    SysTick->CNT = 0;
    switch (iPrecision)
    {
    case SYSTICK_SECONDS:
        SysTick->CMP = (SystemCoreClock/1)-1;
        break;
    case SYSTICK_MILLIS:
        SysTick->CMP = (SystemCoreClock/1000)-1;
        break;
    case SYSTICK_MICROS:
        SysTick->CMP = (SystemCoreClock/1000000)-1;
        break;
    default:
        SysTick->CMP = (SystemCoreClock/1000)-1;   // Default case: use millisecond base
        break;
    }
    SysTick->CTLR = 0xF;

    NVIC_SetPriority(SysTicK_IRQn, 1);
    NVIC_EnableIRQ(SysTicK_IRQn);
}

void SysTick_Handler(void)
{
    SysTick->SR = 0;
    global_systick_counter++;
}

uint64_t systick_get()
{
    return global_systick_counter;
}

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
 * @param   iCount      Base for scaling counter (Defaults to 254 to be compatible with PWM-Library's 8-Bit default iCount of 254, for more precise even frequencies, use even dividers of SystemCoreClock (multiples of 2))
 *
 * @return  Normally returns 0 on exit
 */
int basic_timer_init_base(uint8_t iTimer, uint32_t iF_base, uint8_t iIRQ, uint16_t iCount)
{
    // ---------- Initialize ----------
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};
    // ---------- Initialize Timer ----------
    TIM_TimeBaseInitStructure.TIM_Period = iCount;
	TIM_TimeBaseInitStructure.TIM_Prescaler = SystemCoreClock / iCount / iF_base;   // Rough frequency match, only integer prescaler possible: 96000000 / ( 96000000 / 4 / 1000) / 5
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    switch (iTimer)
    {
        case TCONF_TIM1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
            TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);
            TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);       // Enable self-resetting TRGO-Event when no PWM configured
            if (iIRQ)
            {
                TIM_ClearFlag( TIM1, TIM_FLAG_Update );
                TIM_ITConfig( TIM1, TIM_IT_Update, ENABLE );
                NVIC_EnableIRQ( TIM1_UP_IRQn );
            }
            TIM_Cmd(TIM1, ENABLE);
            break;
        case TCONF_TIM2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
            TIM_TimeBaseInit( TIM2, &TIM_TimeBaseInitStructure);
            TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);       // Enable self-resetting TRGO-Event when no PWM configured
            if (iIRQ)
            {
                TIM_ClearFlag( TIM2, TIM_FLAG_Update );
                TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );
                #if defined(CH32X035) || defined(CH32X033)
                    NVIC_EnableIRQ( TIM2_UP_IRQn );
                #else
                    NVIC_EnableIRQ( TIM2_IRQn );
                #endif
            }
            TIM_Cmd(TIM2, ENABLE);
            break;
        case TCONF_TIM3:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
            TIM_TimeBaseInit( TIM3, &TIM_TimeBaseInitStructure);
            TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);       // Enable self-resetting TRGO-Event when no PWM configured
            if (iIRQ)
            {
                TIM_ClearFlag( TIM3, TIM_FLAG_Update );
                TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE );
                NVIC_EnableIRQ( TIM3_IRQn );
            }
            TIM_Cmd(TIM3, ENABLE);
            break;
        case TCONF_TIM4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);
            TIM_TimeBaseInit( TIM4, &TIM_TimeBaseInitStructure);
            TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update);       // Enable self-resetting TRGO-Event when no PWM configured
            if (iIRQ)
            {
                TIM_ClearFlag( TIM4, TIM_FLAG_Update );
                TIM_ITConfig( TIM4, TIM_IT_Update, ENABLE );
                NVIC_EnableIRQ( TIM4_IRQn );
            }
            TIM_Cmd(TIM4, ENABLE);
            break;
    }
    return 0;
}

int var_basic_timer_init(init_timer_args in)
{
    uint16_t iCount_out = in.iCount ? in.iCount : 254;
    return basic_timer_init_base(in.iTimer, in.iF_base, in.iIRQ, iCount_out);
}