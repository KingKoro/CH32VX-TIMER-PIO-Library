#include "debug.h"
#include "ch32v_timer.h"
#include "stdlib.h"

uint64_t last_count = 0;          // Time of last ISR call in us since init of systick
uint16_t counter_printf = 0;

/*********************************************************************
 * @fn      tim3_interrupt_handler()
 * 
 * @brief   Interrupt Service Routine for TIM3 UP event.
 *
 * @return  none
 */
void tim3_interrupt_handler(void)
{
    // Get elapsed time in us since last ISR call
    uint64_t systick_now = systick_get();
    uint64_t elapsed_time = (systick_now - last_count);    // Cast to uint32_t because printf cannot process 64bits
    last_count = systick_now;
    if (counter_printf > 100)
    {
        counter_printf = 0;
        printf("CLK_CNT: %lu \r\n", (uint32_t)elapsed_time);    // Only print out every 100th time to not cause delays that affect measurment precision
    }
    counter_printf++;
}

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
    // Configure Timer 3 to count upwards and trigger an interrupt at a freqency of roughly 2kHz (0.5ms Timer)
    // 96000000 / (96000000 / 254 / 2000) / (254 + 1) = ~2kHz Output -> ca. 500us between ISRs (actual delay is 502us due to inaccurate actual frequency of 1992.2 Hz)
    //basic_timer_init(TCONF_TIM3, 2000, TCONF_IRQ, tim3_interrupt_handler);
    // Simpel Example: precise 1Khz Timer runs but creates no interrupt (can still be used as source for ADC conversion in example) -> Placeholder for argument func is NULL, iCount = 5
    // 96000000 / (96000000 / 5 / 1200) / (5 + 1) = exactly 1 kHz
    //basic_timer_init(TCONF_TIM3, 1200, TCONF_NO_IRQ, NULL, 5);
    // Additional Example: precise 1Khz Timer runs and creates an interrupt when counter is up, iCount = 5
    // 96000000 / (96000000 / 5 / 1200) / (5 + 1) = exactly 1 kHz -> exactly 1000us between ISRs
    basic_timer_init(TCONF_TIM3, 1200, TCONF_IRQ, tim3_interrupt_handler, 5);
    // Additional Example: precise 1Khz Timer runs and creates an interrupt when counter is up, iCount = 5
    // 96000000 / (96000000 / 10000 / 1) / (10000 + 1) = ~1 Hz -> ca. 1s between ISRs
    //basic_timer_init(TCONF_TIM3, 1, TCONF_IRQ, tim3_interrupt_handler, 10000);

    while(1);
}


