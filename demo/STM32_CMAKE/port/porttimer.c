#include "mb.h"
#include "mbport.h"
#include "port_internal.h"

extern TIM_HandleTypeDef htim7;

/* Static variables */
static USHORT usTimerPeriod50us;
static TIM_HandleTypeDef *pxModbusTimer;

/* Function pointer for timer callback */
static void (*pxMBTimerExpiredFuncCB)(void) = NULL;

void vMBSetTimerExpiredCallback(void (*pxTimerExpiredFunc)(void))
{
    pxMBTimerExpiredFuncCB = pxTimerExpiredFunc;
}

void vMBSetTimerHandle(TIM_HandleTypeDef *pxTimHandle)
{
    pxModbusTimer = pxTimHandle;
}

BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    /* Save timer period for later use */
    usTimerPeriod50us = usTim1Timerout50us;
    
    /* The timer is configured in main.c by STM32CubeMX */
    /*
    if(pxModbusTimer == NULL)
    {
        return FALSE;
    }
    */
    return TRUE;
}

void vMBPortTimersEnable(void)
{
#if 0    
    /* Reset timer counter */
    __HAL_TIM_SET_COUNTER(pxModbusTimer, 0);
    
    /* Set auto-reload value for the required timeout */
    __HAL_TIM_SET_AUTORELOAD(pxModbusTimer, usTimerPeriod50us * 50 - 1);
    
    /* Enable TIM interrupt */
    __HAL_TIM_CLEAR_FLAG(pxModbusTimer, TIM_FLAG_UPDATE);
    __HAL_TIM_ENABLE_IT(pxModbusTimer, TIM_IT_UPDATE);
    
    /* Start timer */
    HAL_TIM_Base_Start(pxModbusTimer);
#endif    
}

void vMBPortTimersDisable(void)
{
    /* Disable timer and its interrupt */
    HAL_TIM_Base_Stop(pxModbusTimer);
    __HAL_TIM_DISABLE_IT(pxModbusTimer, TIM_IT_UPDATE);
}

/* Function to be called from timer interrupt handler */
void prvvMBTimerExpiredISR(void)
{
    if(pxMBTimerExpiredFuncCB != NULL)
    {
        pxMBTimerExpiredFuncCB();
    }
}



/**
  * @brief This function handles TIM7 global interrupt.
  */
 void TIM7_IRQHandler(void)
 { 
      
   pxMBPortCBTimerExpired();
   HAL_TIM_IRQHandler(&htim7);
 
 }
 