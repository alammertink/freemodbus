#include "mb.h"
#include "mbport.h"
#include "port_internal.h"

/* Already included from port_internal.h:
#include "port.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"
*/

#if (USE_BSP_COM_FEATURE > 0)
extern UART_HandleTypeDef hcom_uart[];
#define MODBUS_UART_HANDLE   ( &hcom_uart[COM1] )
#else
#error "BSP COM feature must be enabled"
#endif

static volatile uint8_t    rxByte;
static volatile uint8_t    txByte;
static volatile BOOL       rxEnabled = FALSE;
static volatile BOOL       txEnabled = FALSE;

/* Forward declarations for HAL callbacks */
static void Modbus_UART_RxCpltCallback( UART_HandleTypeDef *huart );
static void Modbus_UART_TxCpltCallback( UART_HandleTypeDef *huart );

/* ----------------------- Start implementation -----------------------------*/

BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    // UART is already initialized by BSP, just register callbacks
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    HAL_UART_RegisterCallback( MODBUS_UART_HANDLE, HAL_UART_RX_COMPLETE_CB_ID, Modbus_UART_RxCpltCallback );
    HAL_UART_RegisterCallback( MODBUS_UART_HANDLE, HAL_UART_TX_COMPLETE_CB_ID, Modbus_UART_TxCpltCallback );
#endif
    rxEnabled = FALSE;
    txEnabled = FALSE;
    return TRUE;
}

void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    rxEnabled = xRxEnable;
    txEnabled = xTxEnable;

    if( xRxEnable )
    {
        HAL_UART_Receive_IT( MODBUS_UART_HANDLE, (uint8_t *)&rxByte, 1 );
    }

    if( xTxEnable )
    {
        // Immediately notify stack that TX is ready
        pxMBFrameCBTransmitterEmpty();
    }
}

BOOL xMBPortSerialPutByte( CHAR ucByte )
{
    txByte = (uint8_t)ucByte;
    // Start UART transmit interrupt for 1 byte
    if( HAL_UART_Transmit_IT( MODBUS_UART_HANDLE, (uint8_t *)&txByte, 1 ) == HAL_OK )
    {
        return TRUE;
    }
    return FALSE;
}

BOOL xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = (CHAR)rxByte;
    return TRUE;
}

/* ----------------------- HAL UART Callbacks -------------------------------*/

static void Modbus_UART_RxCpltCallback( UART_HandleTypeDef *huart )
{
    if( huart == MODBUS_UART_HANDLE )
    {
        if( rxEnabled )
        {
            pxMBFrameCBByteReceived();
            // Re-enable RX interrupt for next byte
            HAL_UART_Receive_IT( MODBUS_UART_HANDLE, (uint8_t *)&rxByte, 1 );
        }
    }
}

static void Modbus_UART_TxCpltCallback( UART_HandleTypeDef *huart )
{
    if( huart == MODBUS_UART_HANDLE )
    {
        if( txEnabled )
        {
            pxMBFrameCBTransmitterEmpty();
        }
    }
}

/* ----------------------- End of file --------------------------------------*/

