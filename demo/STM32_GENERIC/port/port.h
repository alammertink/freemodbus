#ifndef _PORT_H
#define _PORT_H

#include <assert.h>

/* Detect STM32 family and include appropriate HAL */
#if defined(STM32F3xx)
  #include "stm32f3xx_hal.h"
#elif defined(STM32G4xx)
  #include "stm32g4xx_hal.h"
#elif defined(STM32F1xx)
  #include "stm32f1xx_hal.h"
/* Add other STM32 families as needed */
#else
  #error "No STM32 device family defined!"
#endif

#define INLINE                      inline
#define PR_BEGIN_EXTERN_C           extern "C" {
#define PR_END_EXTERN_C             }

#define ENTER_CRITICAL_SECTION()    __disable_irq()
#define EXIT_CRITICAL_SECTION()     __enable_irq()

typedef uint8_t BOOL;
typedef unsigned char UCHAR;
typedef char CHAR;
typedef uint16_t USHORT;
typedef int16_t SHORT;
typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#endif
