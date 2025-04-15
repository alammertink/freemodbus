#include "mb.h"
#include "mbport.h"
#include "port_internal.h"
#include "stm32g4xx_nucleo.h"

/* UART handle - customize based on your project */
static UART_HandleTypeDef *pxMBUartHandle;

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
    comInit.WordLength = (ucDataBits == 8) ? COM_WORDLENGTH_8B : COM_WORDLENGTH_7B;
    
    /* Configure other settings */
    comInit.StopBits = COM_STOPBITS_1;
    comInit.HwFlowCtl = COM_HWCONTROL_NONE;
    
    /* Initialize the COM port */
    if (BSP_COM_Init(COM1, &comInit) != BSP_ERROR_NONE) {
        return FALSE;
    }
    
    /* Enable UART interrupts for Modbus communication */
    HAL_UART_Receive_IT(&hcom_uart[COM1], &ucRxBuffer, 1);
    
    return TRUE;
}

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
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
    /* This would typically be used in the UART receive callback
       rather than being called directly */
    *pucByte = ucRxBuffer;
    
    /* Start another receive operation */
    HAL_UART_Receive_IT(&hcom_uart[COM1], &ucRxBuffer, 1);
    
    return TRUE;
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    /* Transmit one byte using the BSP COM UART handle */
    if (HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*)&ucByte, 1, COM_POLL_TIMEOUT) == HAL_OK) {
        return TRUE;
    }
    return FALSE;
}

/* UART interrupt handler */
void USARTx_IRQHandler(void)
{
  if (__HAL_UART_GET_FLAG(pxMBUartHandle, UART_FLAG_RXNE))
  {
    pxMBFrameCBByteReceived();
  }

  if (__HAL_UART_GET_FLAG(pxMBUartHandle, UART_FLAG_TXE))
  {
    pxMBFrameCBTransmitterEmpty();
  }
}
