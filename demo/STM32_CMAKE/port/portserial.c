#include "mb.h"
#include "mbport.h"
#include "port_internal.h"
#include "stm32g4xx_nucleo.h"

/* Static variables */
static UCHAR ucRxBuffer;
static BOOL bTxEnabled;
static BOOL bRxEnabled;

/* UART handle - using the BSP UART handle */
#define pxMBUartHandle (&hcom_uart[COM1])

BOOL xMBPortSerialInit(UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    COM_InitTypeDef comInit;
    
    /* Configure COM port settings from Modbus parameters */
    comInit.BaudRate = ulBaudRate;
    
    /* Map Modbus parity to COM parity */
    switch (eParity) {
        case MB_PAR_NONE:
            comInit.Parity = COM_PARITY_NONE;
            break;
        case MB_PAR_EVEN:
            comInit.Parity = COM_PARITY_EVEN;
            break;
        case MB_PAR_ODD:
            comInit.Parity = COM_PARITY_ODD;
            break;
        default:
            return FALSE;
    }
    
    /* Map data bits to word length */
    switch (ucDataBits) {
        case 8:
            comInit.WordLength = COM_WORDLENGTH_8B;
            break;
        case 7:
            comInit.WordLength = COM_WORDLENGTH_7B;
            break;
        default:
            return FALSE;
    }
    
    /* Configure other settings */
    comInit.StopBits = COM_STOPBITS_1;
    comInit.HwFlowCtl = COM_HWCONTROL_NONE;
    
    /* Initialize the COM port */
    if (BSP_COM_Init(COM1, &comInit) != BSP_ERROR_NONE) {
        return FALSE;
    }
    
    /* Enable UART global interrupt */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    
    /* Start receiving first byte */
    HAL_UART_Receive_IT(pxMBUartHandle, &ucRxBuffer, 1);
    
    return TRUE;
}

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    /* Remember current states */
    bRxEnabled = xRxEnable;
    bTxEnabled = xTxEnable;
  
    if (xRxEnable)
    {
        /* Enable UART RX interrupt */
        __HAL_UART_ENABLE_IT(pxMBUartHandle, UART_IT_RXNE);
    }
    else
    {
        /* Disable UART RX interrupt */
        __HAL_UART_DISABLE_IT(pxMBUartHandle, UART_IT_RXNE);
    }

    if (xTxEnable)
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
    
    /* Start another receive operation */
    HAL_UART_Receive_IT(pxMBUartHandle, &ucRxBuffer, 1);
    
    return TRUE;
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    /* Transmit one byte using the BSP COM UART handle */
    if (HAL_UART_Transmit(pxMBUartHandle, (uint8_t*)&ucByte, 1, COM_POLL_TIMEOUT) == HAL_OK) {
        return TRUE;
    }
    return FALSE;
}

/* UART callback handlers - to be called from the interrupt handler */
void prvvUARTRxISR(void)
{
    pxMBFrameCBByteReceived();
}

void prvvUARTTxReadyISR(void)
{
    pxMBFrameCBTransmitterEmpty();
}
