#include "ch32v_timer.h"

// Function pointer for TIMER ISR function body, placeholder declaration
void (*f_body_tim1)(void);
void (*f_body_tim2)(void);
void (*f_body_tim3)(void);
void (*f_body_tim4)(void);
// TIMER ISR, placeholder declaration
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
#if !defined(CH32V20X) && !defined(CH32V10X) && !defined(CH32V30X)
#if defined(CH32X035) || defined(CH32X033)
    void TIM2_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
#else
    void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
#endif
#endif
#if !defined(CH32X035) && !defined(CH32X033)
    void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
#endif

#ifdef GLOBAL_TIMING_USE_SYSYTICK
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
uint64_t global_systick_counter = 0;

/*********************************************************************
 * @fn      systick_init
 *
 * @brief   Initialize Systick global time keeping. Select timebase for tracking. Higher precision means more counter incrementing ISR calls.
 * 
 * @param   iPrecision      Timebase for counting Systicks, can be either SYSTICK_SECONDS, SYSTICK_MILLIS, SYSTICK_MICROS for incrementing in steps of 1 second, millisecond or microsecond respectively.
 *                              You can also specify any other arbitrary non-negative, non-zero number (except 1 and 2) to create a custom time base, it is recommended to let iPrecision be a multiple of 10.
 *                              The counter limit to trigger a Systick increment is calculated by (SystemCoreClock/iPrecision)-1 in that case.
 *
 * @return  None
 */
void systick_init(uint32_t iPrecision)
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
        SysTick->CMP = (SystemCoreClock/iPrecision)-1;   // Default case: use whatever arbitrary non-negative, non-zero number is supplied
        break;
    }
    SysTick->CTLR = 0xF;
    #if defined(CH32X035) || defined(CH32X033)
    NVIC_SetPriority(SysTicK_IRQn, 15);
    #else
    NVIC_SetPriority(SysTicK_IRQn, 1);
    #endif
    NVIC_EnableIRQ(SysTicK_IRQn);
}

/*********************************************************************
 * @fn      SysTick_Handler
 *
 * @brief   Interrupt service routine on timebase counter full event. Increments Systick count.
 *
 * @return  None
 */
void SysTick_Handler(void)
{
    SysTick->SR = 0;
    global_systick_counter++;
}

/*********************************************************************
 * @fn      systick_get
 *
 * @brief   Return the current Systick counter value.
 *
 * @return  Current Systick counter value as 64-Bit unsigned integer
 */
uint64_t systick_get()
{
    return global_systick_counter;
}
#endif

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
int basic_timer_init_base(uint8_t iTimer, uint32_t iF_base, uint8_t iIRQ, void (*func)(void), uint16_t iCount)
{
    // ---------- Initialize ----------
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    // ---------- Initialize Timer ----------
    TIM_TimeBaseInitStructure.TIM_Period = iCount;
	TIM_TimeBaseInitStructure.TIM_Prescaler = SystemCoreClock / iCount / iF_base;   // Rough frequency match, only integer prescaler possible: 96000000 / ( 96000000 / 4 / 1000) / 5
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    switch (iTimer)
    {
        case TCONF_TIM1:    // TIM1 is always an advanced timer, independent of MCU-Series
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
            TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);
            TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);       // Enable self-resetting TRGO-Event when no PWM configured
            if (iIRQ && func != NULL && *func != NULL)
            {
                NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
                NVIC_Init(&NVIC_InitStructure);
                TIM_ClearFlag(TIM1, TIM_FLAG_Update);
                TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
                f_body_tim1 = func;
            }
            TIM_Cmd(TIM1, ENABLE);
            break;
        case TCONF_TIM2:    // TIM2 is an advanced timer on the CH32V03X Series (used for async USB TX by default on other series)
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
            TIM_TimeBaseInit( TIM2, &TIM_TimeBaseInitStructure);
            TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);       // Enable self-resetting TRGO-Event when no PWM configured
            if (iIRQ && func != NULL && *func != NULL)
            {
                #if defined(CH32X035) || defined(CH32X033)
                    NVIC_InitStructure.NVIC_IRQChannel = TIM2_UP_IRQn;
                    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
                    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
                    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
                    NVIC_Init(&NVIC_InitStructure);
                    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
                    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
                #else
                    TIM_ClearFlag( TIM2, TIM_FLAG_Update );
                    TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );
                    NVIC_EnableIRQ( TIM2_IRQn );
                #endif
                f_body_tim2 = func;
            }
            TIM_Cmd(TIM2, ENABLE);
            break;
        case TCONF_TIM3:    // TIM3 is always a regular timer (on the CH32V03X Series used for async USB TX by default)
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
            TIM_TimeBaseInit( TIM3, &TIM_TimeBaseInitStructure);
            TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);       // Enable self-resetting TRGO-Event when no PWM configured
            if (iIRQ && func != NULL && *func != NULL)
            {
                TIM_ClearFlag( TIM3, TIM_FLAG_Update );
                TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE );
                NVIC_EnableIRQ( TIM3_IRQn );
                f_body_tim3 = func;
            }
            TIM_Cmd(TIM3, ENABLE);
            break;
        #if !defined(CH32X035) && !defined(CH32X033)
        case TCONF_TIM4:    // TIM3 is always a regular timer (not available on CH32V03X Series)
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);
            TIM_TimeBaseInit( TIM4, &TIM_TimeBaseInitStructure);
            TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update);       // Enable self-resetting TRGO-Event when no PWM configured
            if (iIRQ && func != NULL && *func != NULL)
            {
                TIM_ClearFlag( TIM4, TIM_FLAG_Update );
                TIM_ITConfig( TIM4, TIM_IT_Update, ENABLE );
                NVIC_EnableIRQ( TIM4_IRQn );
                f_body_tim4 = func;
            }
            TIM_Cmd(TIM4, ENABLE);
            break;
        #endif
    }
    return 0;
}

int var_basic_timer_init(init_timer_args in)
{
    void (*func_out)(void) = in.func ? in.func : NULL;
    uint16_t iCount_out = in.iCount ? in.iCount : 254;
    return basic_timer_init_base(in.iTimer, in.iF_base, in.iIRQ, func_out, iCount_out); // pointer to func_out? (reference)
}

/*********************************************************************
 * @fn      TIM1_UP_IRQHandler
 *
 * @brief   This function handles TIM1 up interrupt.
 *
 * @return  none
 */
void TIM1_UP_IRQHandler( void )
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update))
    {
        (*f_body_tim1)();
    }
    /* clear status */
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}

#if !defined(CH32V20X) && !defined(CH32V10X) && !defined(CH32V30X)
#if defined(CH32X035) || defined(CH32X033)
    /*********************************************************************
     * @fn      TIM2_UP_IRQHandler
     *
     * @brief   This function handles TIM2 up interrupt.
     *
     * @return  none
     */
    void TIM2_UP_IRQHandler( void )
    {
        if (TIM_GetITStatus(TIM2, TIM_IT_Update))
        {
            (*f_body_tim2)();
        }
        /* clear status */
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
#else
    /*********************************************************************
     * @fn      TIM2_IRQHandler
     *
     * @brief   This function handles TIM2 up interrupt.
     *
     * @return  none
     */
    void TIM2_IRQHandler( void )
    {
        (*f_body_tim2)();
        /* clear status */
        TIM2->INTFR = (uint16_t)~TIM_IT_Update;
    }
#endif
#endif

#if !defined(CH32X035) && !defined(CH32X033)
    /*********************************************************************
     * @fn      TIM3_IRQHandler
     *
     * @brief   This function handles TIM3 up interrupt.
     *
     * @return  none
     */
    void TIM3_IRQHandler( void )
    {
        (*f_body_tim3)();
        /* clear status */
        TIM3->INTFR = (uint16_t)~TIM_IT_Update;
    }
#endif

#if !defined(CH32X035) && !defined(CH32X033)
/*********************************************************************
 * @fn      TIM4_IRQHandler
 *
 * @brief   This function handles TIM4 up interrupt.
 *
 * @return  none
 */
void TIM4_IRQHandler( void )
{
    (*f_body_tim4)();
    /* clear status */
    TIM4->INTFR = (uint16_t)~TIM_IT_Update;
}
#endif