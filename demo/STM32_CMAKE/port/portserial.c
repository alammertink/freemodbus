#include "mb.h"
#include "mbport.h"
#include "port_internal.h"

/* Static variables */
static UCHAR ucRxBuffer;
static UART_HandleTypeDef *pxMBUartHandle;

/* Function pointers for UART callbacks */
static void (*pxMBFrameRxCB)(void) = NULL;
static void (*pxMBFrameTxCB)(void) = NULL;

void vMBSetUARTHandle(UART_HandleTypeDef *pxUartHandle)
{
    pxMBUartHandle = pxUartHandle;
}

void vMBSetFrameRxCallback(void (*pxRxFunc)(void))
{
    pxMBFrameRxCB = pxRxFunc;
}

void vMBSetFrameTxCallback(void (*pxTxFunc)(void))
{
    pxMBFrameTxCB = pxTxFunc;
}

BOOL xMBPortSerialInit(UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    /* The UART is configured in main.c by STM32CubeMX */
    if(pxMBUartHandle == NULL)
    {
        return FALSE;
    }
    
    /* Override UART configuration if needed */
    pxMBUartHandle->Init.BaudRate = ulBaudRate;
    
    /* Set parity */
    switch(eParity)
    {
        case MB_PAR_NONE:
            pxMBUartHandle->Init.Parity = UART_PARITY_NONE;
            break;
        case MB_PAR_EVEN:
            pxMBUartHandle->Init.Parity = UART_PARITY_EVEN;
            break;
        case MB_PAR_ODD:
            pxMBUartHandle->Init.Parity = UART_PARITY_ODD;
            break;
        default:
            return FALSE;
    }
    
    /* Set data bits - handle parity impact on word length */
    if(eParity != MB_PAR_NONE)
    {
        /* With parity, word length is +1 */
        if(ucDataBits == 8)
            pxMBUartHandle->Init.WordLength = UART_WORDLENGTH_9B;
        else if(ucDataBits == 7)
            pxMBUartHandle->Init.WordLength = UART_WORDLENGTH_8B;
        else
            return FALSE;
    }
    else
    {
        /* Without parity */
        if(ucDataBits == 8)
            pxMBUartHandle->Init.WordLength = UART_WORDLENGTH_8B;
        else if(ucDataBits == 7)
            pxMBUartHandle->Init.WordLength = UART_WORDLENGTH_7B;
        else
            return FALSE;
    }
    
    /* Apply settings */
    if(HAL_UART_Init(pxMBUartHandle) != HAL_OK)
    {
        return FALSE;
    }
    
    /* Start receiving first byte in interrupt mode */
    if(HAL_UART_Receive_IT(pxMBUartHandle, &ucRxBuffer, 1) != HAL_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if(xRxEnable)
    {
        /* Enable UART RX interrupt */
        __HAL_UART_ENABLE_IT(pxMBUartHandle, UART_IT_RXNE);
    }
    else
    {
        /* Disable UART RX interrupt */
        __HAL_UART_DISABLE_IT(pxMBUartHandle, UART_IT_RXNE);
    }

    if(xTxEnable)
    {
        /* Enable UART TX interrupt */
        __HAL_UART_ENABLE_IT(pxMBUartHandle, UART_IT_TXE);
    }
    else
    {
        /* Disable UART TX interrupt */
        __HAL_UART_DISABLE_IT(pxMBUartHandle, UART_IT_TXE);
    }
}

BOOL xMBPortSerialGetByte(CHAR *pucByte)
{
    *pucByte = ucRxBuffer;
    
    /* Start receiving next byte */
    HAL_UART_Receive_IT(pxMBUartHandle, &ucRxBuffer, 1);
    
    return TRUE;
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    /* Send byte - using polling mode for simplicity */
    return (HAL_UART_Transmit(pxMBUartHandle, (uint8_t*)&ucByte, 1, 10) == HAL_OK);
}

/* Functions to be called from UART interrupt handler */
void prvvUARTRxISR(void)
{
    if(pxMBFrameRxCB != NULL)
    {
        pxMBFrameRxCB();
    }
}

void prvvUARTTxReadyISR(void)
{
    if(pxMBFrameTxCB != NULL)
    {
        pxMBFrameTxCB();
    }
}
