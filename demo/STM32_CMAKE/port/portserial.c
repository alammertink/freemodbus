#include "mb.h"
#include "mbport.h"
#include "port_internal.h"


UART_HandleTypeDef          uart_mb; 

static volatile uint8_t     rxByte = 0;
static volatile uint8_t     txByte = 0;


/* Forward declarations for HAL callbacks */
static void Modbus_UART_RxCpltCallback( UART_HandleTypeDef *huart );
static void Modbus_UART_TxCpltCallback( UART_HandleTypeDef *huart );

/* ----------------------- Start implementation -----------------------------*/

/*  Note that UART configuration for ST processors is, well, different:

    https://community.st.com/t5/stm32-mcus-products/uart-parity-and-data-bit-issue-in-stm32c0-series/td-p/713896

    Rest-of-World:  8E1

    ST:
    UartHandle.Init.WordLength = UART_WORDLENGTH_9B; // 8+Parity
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_EVEN;


    Rest-of-World: 7E1

    ST:
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B; // 7+Parity
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_EVEN;
*/


BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    UNUSED( ucPORT );

    // Configure UART for Modbus communication
    uart_mb.Instance          = MB_USART;
    uart_mb.Init.BaudRate     = ulBaudRate;
    uart_mb.Init.StopBits     = UART_STOPBITS_1; // Always use 1 stop bit
    uart_mb.Init.Mode         = UART_MODE_TX_RX;
    uart_mb.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart_mb.Init.OverSampling = UART_OVERSAMPLING_16;

    // Configure WordLength and Parity based on data bits and parity
    if( ucDataBits == 8 )
    {
        if( eParity == MB_PAR_NONE )
        {
            uart_mb.Init.WordLength = UART_WORDLENGTH_8B;
            uart_mb.Init.Parity     = UART_PARITY_NONE;
        }
        else
        {
            uart_mb.Init.WordLength = UART_WORDLENGTH_9B; // 8 data bits + parity
            uart_mb.Init.Parity     = (eParity == MB_PAR_ODD) ? UART_PARITY_ODD : UART_PARITY_EVEN;
        }
    }
    else if( ucDataBits == 7 )
    {
        if( eParity == MB_PAR_NONE )
        {
            uart_mb.Init.WordLength = UART_WORDLENGTH_7B;
            uart_mb.Init.Parity     = UART_PARITY_NONE;
        }
        else
        {
            uart_mb.Init.WordLength = UART_WORDLENGTH_8B; // 7 data bits + parity
            uart_mb.Init.Parity     = (eParity == MB_PAR_ODD) ? UART_PARITY_ODD : UART_PARITY_EVEN;
        }
    }
    else
    {
        return FALSE; // Unsupported data bits configuration
    }

    if( HAL_UART_Init( &uart_mb ) != HAL_OK )
    {
        return FALSE; // UART initialization failed
    }

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    if( HAL_UART_RegisterCallback( &uart_mb, HAL_UART_RX_COMPLETE_CB_ID, Modbus_UART_RxCpltCallback ) != HAL_OK )
    {
        return FALSE; // Callback registration failed
    }

    if( HAL_UART_RegisterCallback( &uart_mb, HAL_UART_TX_COMPLETE_CB_ID, Modbus_UART_TxCpltCallback ) != HAL_OK )
    {
        return FALSE; // Callback registration failed
    }
#else
#error "HAL UART callback registration must be enabled"
#endif

    // Disable RX and TX interrupts initially
    __HAL_UART_DISABLE_IT(&uart_mb, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(&uart_mb, UART_IT_TXE);

    return TRUE;
}


void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xRxEnable)
    {
        __HAL_UART_ENABLE_IT(&uart_mb, UART_IT_RXNE);
        HAL_UART_Receive_IT(&uart_mb, (uint8_t *)&rxByte, 1);
    }
    else
    {
        __HAL_UART_DISABLE_IT(&uart_mb, UART_IT_RXNE);
    }

    if (xTxEnable)
    {
        __HAL_UART_ENABLE_IT(&uart_mb, UART_IT_TXE);
        pxMBFrameCBTransmitterEmpty();  // kickstart transmission
    }
    else
    {
        __HAL_UART_DISABLE_IT(&uart_mb, UART_IT_TXE);
    }
}


BOOL xMBPortSerialPutByte( CHAR ucByte )
{
    txByte = (uint8_t)ucByte;

    if( HAL_UART_Transmit_IT( &uart_mb, (uint8_t *)&txByte, 1 ) == HAL_OK )
    {
        return TRUE;
    }
    return FALSE;
}

BOOL xMBPortSerialGetByte( CHAR* pucByte )
{
    *pucByte = (CHAR)rxByte;
    return TRUE;
}

/* ----------------------- HAL UART Callbacks -------------------------------*/

static void Modbus_UART_RxCpltCallback( UART_HandleTypeDef *huart )
{
    UNUSED(huart);

    HAL_UART_Receive_IT( &uart_mb, (uint8_t *)&rxByte, 1 );

    pxMBFrameCBByteReceived();
}

static void Modbus_UART_TxCpltCallback( UART_HandleTypeDef *huart )
{
    UNUSED(huart);

    pxMBFrameCBTransmitterEmpty();
}

/**
 * @brief UART MSP Initialization
 *        This function configures the hardware resources used for Modbus UART:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 *           - NVIC configuration for UART interrupt
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if( huart->Instance != MB_USART )        {  return; }

    // Enable UART peripheral clock
    MB_USART_CLK_ENABLE();

    // Enable GPIO clocks
    MB_TX_GPIO_CLK_ENABLE();
    MB_RX_GPIO_CLK_ENABLE();

    // Configure UART TX pin
    GPIO_InitStruct.Pin       = MB_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = MB_TX_AF;

    HAL_GPIO_Init(MB_TX_GPIO_PORT, &GPIO_InitStruct);

    // Configure UART RX pin
    GPIO_InitStruct.Pin       = MB_RX_PIN;
    GPIO_InitStruct.Alternate = MB_RX_AF;
    
    HAL_GPIO_Init(MB_RX_GPIO_PORT, &GPIO_InitStruct);

    // Configure NVIC for UART interrupt
    HAL_NVIC_SetPriority(MB_USART_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MB_USART_IRQn);
}

/**
 * @brief UART MSP De-Initialization
 *        This function frees the hardware resources used for Modbus UART:
 *           - Disable the Peripheral's clock
 *           - Revert GPIO configuration to default state
 *           - Disable NVIC for UART interrupt
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if( huart->Instance != MB_USART )        {  return; }

    // Disable UART peripheral clock
    MB_USART_CLK_DISABLE();

    // Deinitialize UART TX pin
    HAL_GPIO_DeInit(MB_TX_GPIO_PORT, MB_TX_PIN);

    // Deinitialize UART RX pin
    HAL_GPIO_DeInit(MB_RX_GPIO_PORT, MB_RX_PIN);

    // Disable NVIC for UART interrupt
    HAL_NVIC_DisableIRQ(MB_USART_IRQn);
}

void MB_USART_IRQHandler(void)
{
    HAL_UART_IRQHandler(&uart_mb);
}

/* ----------------------- End of file --------------------------------------*/

