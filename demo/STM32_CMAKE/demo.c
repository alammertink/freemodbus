/*
 * FreeModbus Libary: BARE Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "port_internal.h"

/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4
#define SLAVE_ID        0x0A
#define MB_BAUDRATE     38400

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];

/* External variables for handles passed from main.c */
extern TIM_HandleTypeDef htim7;
extern UART_HandleTypeDef huart2;

/* ----------------------- Static functions ---------------------------------*/
/* Callbacks from the FreeModbus library */
static void vTimerExpiredCB(void);
static void vRxCB(void);
static void vTxCB(void);

/* Modbus register callback functions */
eMBErrorCode eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs);
eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode);
eMBErrorCode eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode);
eMBErrorCode eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete);

/* ----------------------- Public API functions -----------------------------*/
/**
  * @brief  Initialize Modbus functionality
  * @note   Call this once during system initialization
  * @retval None
  */
void modbusInit(void)
{
    eMBErrorCode eStatus;
    
    /* Set up peripheral handles */
    vMBSetTimerHandle(&htim7);
    vMBSetUARTHandle(&hcom_uart[COM1]);
    
    /* Set up callback functions for the port layer */
    vMBSetTimerExpiredCallback(vTimerExpiredCB);
    vMBSetFrameRxCallback(vRxCB);
    vMBSetFrameTxCallback(vTxCB);
    
    /* Initialize Modbus protocol stack */
    eStatus = eMBInit(MB_RTU, SLAVE_ID, 0, MB_BAUDRATE, MB_PAR_EVEN);
    
    /* Initialize register values */
    usRegInputBuf[0] = 0;  /* Initialize counter */
    usRegInputBuf[1] = 0;
    usRegInputBuf[2] = 0;
    usRegInputBuf[3] = 0;
    
    /* Enable Modbus protocol stack */
    eStatus = eMBEnable();
}

/**
  * @brief  Poll Modbus protocol stack
  * @note   Call this regularly in main loop
  * @retval None
  */
void modbusPoll(void)
{
    /* Process Modbus events */
    (void)eMBPoll();
    
    /* Update the first register as a counter */
    usRegInputBuf[0]++;
}

/* ----------------------- UART/Timer ISR callback functions --------------------------*/
/**
  * @brief  Call this from TIM7_IRQHandler in stm32g4xx_it.c
  * @retval None
  */
void ModbusTimerISR(void)
{
    prvvMBTimerExpiredISR();
}

/**
  * @brief  Call this from USART2_IRQHandler in stm32g4xx_it.c
  * @note   Must be called when RXNE flag is set
  * @retval None
  */
void ModbusUARTRxISR(void)
{
    prvvUARTRxISR();
}

/**
  * @brief  Call this from USART2_IRQHandler in stm32g4xx_it.c
  * @note   Must be called when TXE flag is set
  * @retval None
  */
void ModbusUARTTxISR(void)
{
    prvvUARTTxReadyISR();
}

/* ----------------------- Static callbacks ---------------------------------*/
static void vTimerExpiredCB(void)
{
    (void)pxMBPortCBTimerExpired();
}

static void vRxCB(void)
{
    pxMBFrameCBByteReceived();
}

static void vTxCB(void)
{
    pxMBFrameCBTransmitterEmpty();
}

/* ----------------------- Modbus register callback functions ---------------------------------*/
eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
    return MB_ENOREG;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}


/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
  if (__HAL_UART_GET_FLAG(&hcom_uart[COM1], UART_FLAG_RXNE))
  {
    prvvUARTRxISR();
  }

  if (__HAL_UART_GET_FLAG(&hcom_uart[COM1], UART_FLAG_TXE))
  {
    prvvUARTTxReadyISR();
  }
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&hcom_uart[COM1]);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{ 
     
  pxMBPortCBTimerExpired();
  HAL_TIM_IRQHandler(&htim7);

}
