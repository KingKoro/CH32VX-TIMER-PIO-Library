#include "debug.h"
#include "ch32v_timer.h"
#include "stdlib.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    // Init Global Systick to track timing (in microseconds)
    systick_init(SYSTICK_MICROS);
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    // Configure Timer 3 to count upwards and trigger an interrupt at a freqency of roughly 1kHz (1ms Timer)
    // 96000000 / (96000000 / 254 / 1000) / (254 + 1) = ~1kHz Output
    basic_timer_init(TCONF_TIM3, 1000, TCONF_IRQ);

    while(1);
}