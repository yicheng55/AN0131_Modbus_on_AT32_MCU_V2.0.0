
#include "stdio.h"
#include "mbtask.h"


/* ----------------------- Static variables ---------------------------------*/
static USHORT usRegInputStart = REG_INPUT_START;
static USHORT usRegInputBuf[REG_INPUT_NREGS] = {0x1234, 0x5678, 0x9ABC, 0xDEF0};
static USHORT usRegHoldingStart = REG_HOLDING_START;
static USHORT usRegHoldingBuf[REG_HOLDING_NREGS] = {0, 11, 22, 33, 44, 55, 66, 77, 88, 99};

//coils buffer
static UCHAR  ucRegCoilsBuf[REG_COILS_SIZE / 8] = {0x01, 0x02};
//discrete Inputs buffer
static UCHAR  ucRegDiscreteBuf[REG_DISCRETE_SIZE / 8] = {0x80, 0x90};


void modbus_task(void)
{
  eMBErrorCode    eStatus;
  
 /* ucPort: select port_uart.
  * this parameter can be one of the following values:
  * 0: USART2: tx--PA2,  rx--PA3,  de--PA1;
  * 1: USART3: tx--PB10, rx--PB11, de--PB14;
  * other: invalid.
  */
  eStatus = eMBInit(MB_RTU, MB_SLAVE_ADDRESS, 0, MB_BAUDRATE, MB_PAR_NONE);
  if(MB_ENOERR == eStatus)
  {
    printf("modbus init ok\r\n");
    eStatus = eMBEnable();
    if(MB_ENOERR == eStatus)
    {
      printf("modbus enable ok\r\n");
    }
    else
    {
      printf("modbus enable fail, error code: %u\r\n", eStatus);
    }
  }
  else
  {
    printf("modbus init fail, error code: %u\r\n", eStatus);
  }
  
  if(MB_ENOERR != eStatus)
  {
    printf("exit modbus task.\r\n");
    return;
  }
    
  printf("start modbus pooling..\r\n");
  for(;;){
    eMBPoll();
  }
}

/****************************************************************************
* brief: read input register;
*        responsive function code: 
*        04 (Read Input Register).
* param: pucRegBuffer: data buffer, used for respond master;
*						usAddress: register address;
*						  usNRegs: register number.
* retval: eStatus: error code.
****************************************************************************/
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

/****************************************************************************
* brief: read/write holding register;
*        responsive function code: 
*        06 (Write Holding Register); 
*				 16 (Write Multiple Holding Register);
*				 03 (Read Holding Register);
*				 23 (Read/Write Multiple Holding Register).
* param: pucRegBuffer: data buffer, used for respond master;
*						usAddress: register address;
*						  usNRegs: register number;
*               eMode: access register mode.
* retval: eStatus: error code.
****************************************************************************/
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_HOLDING_START ) && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

extern void xMBUtilSetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits, UCHAR ucValue );
extern UCHAR xMBUtilGetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits );

/****************************************************************************
* brief: read/write coils;
*        responsive function code: 
*        01 (Read Coils);
*				 05 (Write Coil);
*				 15 (Write Multiple Coils).
* param: pucRegBuffer: data buffer, used for respond master;
*						usAddress: coils address;
*						 usNCoils: coils number;
*               eMode: access register mode.
* retval: eStatus: error code.
****************************************************************************/
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    eMBErrorCode eStatus = MB_ENOERR;
    int16_t iNCoils = ( int16_t )usNCoils;
    int16_t usBitOffset;
    
    if( ( (int16_t)usAddress >= REG_COILS_START ) &&
       ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
    {
      usBitOffset = ( int16_t )( usAddress - REG_COILS_START );
      switch ( eMode )
      {
      case MB_REG_READ:
        while( iNCoils > 0 )
        {
          *pucRegBuffer++ = xMBUtilGetBits( ucRegCoilsBuf, usBitOffset,
                                           ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ) );
          iNCoils -= 8;
          usBitOffset += 8;
        }
        break;
        
      case MB_REG_WRITE:
        while( iNCoils > 0 )
        {
          xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,
                         ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ),
                         *pucRegBuffer++ );
          iNCoils -= 8;
        }
        break;
      }
    }
    else
    {
      eStatus = MB_ENOREG;
    }
    return eStatus;
}

/****************************************************************************
* brief: read discrete inputs;
*        responsive function code: 
*				 02 (read discrete inputs).
* param: pucRegBuffer: data buffer, used for respond master;
*						usAddress: discrete inputs address;
*				  usNDiscrete: discrete inputs number.
* retval: eStatus: error code.
****************************************************************************/
eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode eStatus = MB_ENOERR;
    int16_t iNDiscrete = ( int16_t )usNDiscrete;
    uint16_t usBitOffset;
    
    if( ( (int16_t)usAddress >= REG_DISCRETE_START ) &&
       ( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE ) )
    {
      usBitOffset = ( uint16_t )( usAddress - REG_DISCRETE_START );
      
      while( iNDiscrete > 0 )
      {
        *pucRegBuffer++ = xMBUtilGetBits( ucRegDiscreteBuf, usBitOffset,
                                         ( uint8_t)( iNDiscrete > 8 ? 8 : iNDiscrete ) );
        iNDiscrete -= 8;
        usBitOffset += 8;
      }
    }
    else
    {
      eStatus = MB_ENOREG;
    }
    return eStatus;
}

