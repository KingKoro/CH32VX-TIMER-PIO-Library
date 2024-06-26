/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v20x_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v20x_it.h"
#include "ch32v_timer.h"

uint64_t last_count = 0;          // Time of last ISR call in us since init of systick
uint16_t counter_printf = 0;

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
}
/*********************************************************************
 * @fn      TIM3_IRQHandler
 *
 * @brief   This function handles TIM3 exception.
 *
 * @return  none
 */
void TIM3_IRQHandler( void )
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
  /* clear status */
  TIM3->INTFR = (uint16_t)~TIM_IT_Update;
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}


