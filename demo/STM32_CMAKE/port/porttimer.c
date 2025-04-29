#include "mb.h"
#include "mbport.h"
#include "port_internal.h"
#include "main.h"  // Include for GPIO definitions

extern TIM_HandleTypeDef htim7;

/* Static variables */
static USHORT           timeout     = 0;
static USHORT           downcounter = 0;

BOOL xMBPortTimersInit( USHORT usTim1Timerout50us )
{
#if 0
    TIM_MasterConfigTypeDef sMasterConfig;
  
    /* Store the timeout value for later use */
    timeout = usTim1Timerout50us;

    /* Calculate timer settings for exact 50us ticks 
     * Our goal is to configure the timer to tick precisely every 50us
     * This gives us 20,000 ticks per second (20 kHz)
     */
    uint32_t timerClock;
    
    /* Get the timer clock frequency */
    /* If APB1 prescaler != 1, timer clock = 2 * PCLK1, else timer clock = PCLK1 */
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0) {
        timerClock = HAL_RCC_GetPCLK1Freq() * 2;
    } else {
        timerClock = HAL_RCC_GetPCLK1Freq();
    }
    
    /* Fixed configuration for exactly 50us per tick
     * We need: timerClock / ((prescaler+1) * (period+1)) = 20,000 Hz
     */
    uint32_t prescaler = 0; /* Start with smallest prescaler for maximum precision */
    uint32_t period = timerClock / 20000 - 1;
    
    /* If period is too large, increase prescaler */
    while (period > 0xFFFF) {
        prescaler++;
        period = timerClock / ((prescaler + 1) * 20000) - 1;
    }

    /* Configure the timer */
    htim7.Instance = TIM7;
    htim7.Init.Prescaler = prescaler;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = period;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
 
    if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
    {
        return FALSE;
    }
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
    {
        return FALSE;
    }

#else

    TIM_MasterConfigTypeDef sMasterConfig;
    
    htim7.Instance = TIM7;
    htim7.Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 50 - 1;

    timeout = usTim1Timerout50us;

    if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
    {
    return FALSE;
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
    {
    return FALSE;
    }

#endif

    /* Configure NVIC for timer interrupt */
    HAL_NVIC_SetPriority(TIM7_IRQn, MB_TIM7_IRQ_priority, MB_TIM7_IRQ_subpriority);
    
    /* Setup debug output pin */
    #if MB_TIMER_DEBUG == 1
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Configure debug GPIO pin */
    GPIO_InitStruct.Pin = MB_TIMER_DEBUG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MB_TIMER_DEBUG_PORT, &GPIO_InitStruct);
    
    /* Initial state: low */
    vMBTimerDebugSetLow();
    #endif

    return TRUE;
}

void vMBPortTimersEnable( void )
{
  /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
  downcounter = timeout;
  
  /* Set debug pin high to indicate timer is active */
  vMBTimerDebugSetHigh();
  
  HAL_TIM_Base_Start_IT(&htim7);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);
}

void vMBPortTimersDisable( void )
{
  /* Disable any pending timers. */
  HAL_TIM_Base_Stop_IT(&htim7);
  HAL_NVIC_DisableIRQ(TIM7_IRQn);
  
  /* Set debug pin low to indicate timer is disabled */
  vMBTimerDebugSetLow();
}

/**
 * @brief This function handles TIM7 global interrupt.
 */
void TIM7_IRQHandler( void )
{
    /* Check if update interrupt flag is set */
    if(__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE) != RESET && 
       __HAL_TIM_GET_IT_SOURCE(&htim7, TIM_IT_UPDATE) != RESET) {
        /* Clear update interrupt flag */
        __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);

        /* Decrement down-counter and check if reached zero */
        if (--downcounter == 0) {
            /* Timer expired, call the callback function */
            vMBTimerDebugSetLow();
            pxMBPortCBTimerExpired();
        }
    }

    /* Call the HAL timer IRQ handler */
    HAL_TIM_IRQHandler(&htim7);
}

