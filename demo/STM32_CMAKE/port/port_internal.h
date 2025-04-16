#ifndef _PORT_INTERNAL_H
#define _PORT_INTERNAL_H

#include "port.h"

/* Detect STM32 family and include appropriate HAL */
#if defined(STM32F3xx)
  #include "stm32f3xx_hal.h"
#elif defined(STM32G4xx) || defined(STM32G431xx)
  #include "stm32g4xx_hal.h"
#elif defined(STM32F1xx)
  #include "stm32f1xx_hal.h"
/* Add other STM32 families as needed */
#else
  #error "No STM32 device family defined!"
#endif

/* Functions to set callback handlers from demo.c */
void vMBSetTimerExpiredCallback(void (*pxTimerExpiredFunc)(void));
void vMBSetFrameRxCallback(void (*pxRxFunc)(void));
void vMBSetFrameTxCallback(void (*pxTxFunc)(void));

/* Functions to set peripheral handles */
void vMBSetTimerHandle(TIM_HandleTypeDef *pxTimHandle);
void vMBSetUARTHandle(UART_HandleTypeDef *pxUartHandle);

/* Functions to be called from IRQ handlers */
void prvvUARTRxISR(void);
void prvvUARTTxReadyISR(void);
void prvvMBTimerExpiredISR(void);

#endif
