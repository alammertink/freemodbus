#include "mb.h"
#include "mbport.h"
#include "port_internal.h"

/* STM32 Timer handle - customize based on your project */
static TIM_HandleTypeDef *pxMBTimerHandle;

BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
  /* Configure your timer based on usTim1Timerout50us */
  /* Timer should be configured for 20kHz (50μs) */
  
  /* Store timer reload value for 50µs intervals */
  return TRUE;
}

void vMBPortTimersEnable()
{
  /* Start timer with stored timeout value */
  HAL_TIM_Base_Start_IT(pxMBTimerHandle);
}

void vMBPortTimersDisable()
{
  /* Stop timer */
  HAL_TIM_Base_Stop_IT(pxMBTimerHandle);
}

/* Timer callback - to be called from timer interrupt handler */
void prvvMBTimerExpiredISR()
{
  (void)pxMBPortCBTimerExpired();
}

/* Implement this in your main stm32g4xx_it.c file */
void TIMx_IRQHandler(void)
{
  HAL_TIM_IRQHandler(pxMBTimerHandle);
  /* Call the Modbus timer expired function in the ISR */
  prvvMBTimerExpiredISR();
}
