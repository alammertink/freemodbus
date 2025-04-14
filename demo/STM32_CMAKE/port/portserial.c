#include "mb.h"
#include "mbport.h"
#include "port_internal.h"

/* UART handle - customize based on your project */
static UART_HandleTypeDef *pxMBUartHandle;

BOOL xMBPortSerialInit(UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
  /* Configure UART based on parameters */
  /* Set baud rate, data bits, and parity */
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
  /* Read a byte from UART */
  *pucByte = (CHAR)(pxMBUartHandle->Instance->RDR & 0xFF);
  return TRUE;
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
  /* Write a byte to UART */
  pxMBUartHandle->Instance->TDR = ucByte;
  return TRUE;
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
