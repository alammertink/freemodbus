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


#endif
