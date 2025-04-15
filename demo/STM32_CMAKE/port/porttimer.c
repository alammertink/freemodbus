#include "mb.h"
#include "mbport.h"
#include "port_internal.h"

/* External variables */
extern TIM_HandleTypeDef htim7;

/* Static variables */
static USHORT usTimerPeriod50us;

BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    /* Save timer period for later use */
    usTimerPeriod50us = usTim1Timerout50us;
    
    /* Configure TIM7 for 50us timing */
    htim7.Instance = TIM7;
    
    /* Calculate prescaler value for 50us timing
     * Timer freq = APB1 timer clock / (prescaler + 1)
     * For 50us period with a 170MHz clock (assuming max G4 speed):
     * 50us = 1/20000Hz, so we need 170MHz/20000 = 8500 counts per period
     * 
     * But we'll use more flexible calculation based on HAL_RCC_GetPCLK1Freq()
     */
    htim7.Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;  /* To get 1MHz timer clock */
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 50 - 1;  /* 50us period with 1MHz timer clock */
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
    {
        return FALSE;
    }
    
    /* Configure TIM7 interrupt priority */
    HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);
    
    return TRUE;
}

void vMBPortTimersEnable(void)
{
    /* Reset timer counter */
    __HAL_TIM_SET_COUNTER(&htim7, 0);
    
    /* Set auto-reload value for the required timeout */
    __HAL_TIM_SET_AUTORELOAD(&htim7, usTimerPeriod50us * 50 - 1);
    
    /* Enable TIM7 interrupt */
    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE);
    
    /* Start timer */
    HAL_TIM_Base_Start(&htim7);
}

void vMBPortTimersDisable(void)
{
    /* Disable timer and its interrupt */
    HAL_TIM_Base_Stop(&htim7);
    __HAL_TIM_DISABLE_IT(&htim7, TIM_IT_UPDATE);
}

/* Timer callback function - to be called from the timer interrupt handler */
void prvvTIMERExpiredISR(void)
{
    (void)pxMBPortCBTimerExpired();
}
