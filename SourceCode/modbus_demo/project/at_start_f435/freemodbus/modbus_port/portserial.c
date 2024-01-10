/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

static usart_type* port_uart_list[] = {USART2, USART3};
static usart_type* port_uart;

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    if(xRxEnable == TRUE)
    {
      usart_interrupt_enable(port_uart, USART_RDBF_INT, TRUE);
    }
    else
    {
      usart_interrupt_enable(port_uart, USART_RDBF_INT, FALSE);
    }
  
    if(xTxEnable == TRUE)
    {
      usart_interrupt_enable(port_uart, USART_TDBE_INT, TRUE);
    }
    else
    {
      usart_interrupt_enable(port_uart, USART_TDBE_INT, FALSE);
    }
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
  BOOL            bStatus = FALSE;
  usart_parity_selection_type usart_parity;
  usart_stop_bit_num_type     usart_stop;
  usart_data_bit_num_type     usart_data;
  gpio_init_type  gpio_init_struct;
  
  bStatus = TRUE;
  
  if(ucPORT < (sizeof(port_uart_list) / sizeof(usart_type*)))
  {
    port_uart = port_uart_list[ucPORT];
  }
  else
  {
    bStatus = FALSE;
  }
  
  switch ( eParity )
  {
  case MB_PAR_NONE:
    usart_parity = USART_PARITY_NONE;
    usart_stop = USART_STOP_2_BIT;
    break;
  case MB_PAR_ODD:
    usart_parity = USART_PARITY_ODD;
    usart_stop = USART_STOP_1_BIT;
    break;
  case MB_PAR_EVEN:
    usart_parity = USART_PARITY_EVEN;
    usart_stop = USART_STOP_1_BIT;
    break;
  default:
    bStatus = FALSE;
    break;
  }
  
  switch ( ucDataBits )
  {
  case 8:
    if(usart_stop == USART_STOP_1_BIT)
    {
      usart_data = USART_DATA_9BITS;
    }
    else if(usart_stop == USART_STOP_2_BIT)
    {
      usart_data = USART_DATA_8BITS;
    }
    else
    {
      bStatus = FALSE;
    }
    break;
  case 7:
    if(usart_stop == USART_STOP_1_BIT)
    {
      usart_data = USART_DATA_8BITS;
    }
    else if(usart_stop == USART_STOP_2_BIT)
    {
      usart_data = USART_DATA_7BITS;  //before F435 not support 7bit.
    }
    else
    {
      bStatus = FALSE;
    }
    break;
  default:
    bStatus = FALSE;
    break;
  }
  
  if(TRUE == bStatus)
  {
    gpio_default_para_init(&gpio_init_struct);
    
    switch((u32)port_uart){
      case (u32)USART2:
        nvic_irq_disable(USART2_IRQn);
        /* enable the uart2 and gpio clock */
        crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
        crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
        /* configure the uart2 tx,rx,de pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins = GPIO_PINS_1|GPIO_PINS_2|GPIO_PINS_3;
        gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
        gpio_init(GPIOA, &gpio_init_struct);
        gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE1, GPIO_MUX_7);
        gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE2, GPIO_MUX_7);
        gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE3, GPIO_MUX_7);
        break;
      case (u32)USART3:
        nvic_irq_disable(USART3_IRQn);
        /* enable the uart3 and gpio clock */
        crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);
        crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
        /* configure the uart3 tx,rx,de pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins = GPIO_PINS_10|GPIO_PINS_11|GPIO_PINS_14;
        gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
        gpio_init(GPIOB, &gpio_init_struct);
        gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE10, GPIO_MUX_7);
        gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE11, GPIO_MUX_7);
        gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE14, GPIO_MUX_7);
        break;
      default:
        bStatus = FALSE;
        break;
    }

    /* configure uart param */
    usart_parity_selection_config(port_uart, usart_parity);
    usart_init(port_uart, ulBaudRate, usart_data, usart_stop);
    usart_rs485_delay_time_config(port_uart, 2, 2);
    usart_de_polarity_set(port_uart, USART_DE_POLARITY_HIGH);
    usart_rs485_mode_enable(port_uart, TRUE);
    usart_transmitter_enable(port_uart, TRUE);
    usart_receiver_enable(port_uart, TRUE);
    usart_enable(port_uart, TRUE);
    
    switch((u32)port_uart){
      case (u32)USART2:
        NVIC_ClearPendingIRQ(USART2_IRQn);
        nvic_irq_enable(USART2_IRQn, 0, 1);
        break;
      case (u32)USART3:
        NVIC_ClearPendingIRQ(USART3_IRQn);
        nvic_irq_enable(USART3_IRQn, 0, 1);
        break;
      default:
        bStatus = FALSE;
        break;
    } 
  }
  
  return bStatus;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    usart_data_transmit(port_uart, ucByte);
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    *pucByte = usart_data_receive(port_uart);
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void vUSARTHandler( void )
{
    if(port_uart->ctrl1_bit.rdbfien == SET)
    {
      if(usart_flag_get(port_uart, USART_RDBF_FLAG) == SET)
      {
        prvvUARTRxISR();
      }
    }
    
    if(port_uart->ctrl1_bit.tdbeien == SET)
    {
      if(usart_flag_get(port_uart, USART_TDBE_FLAG) == SET)
      {
        prvvUARTTxReadyISR();
      }
    }
}

void USART2_IRQHandler(void)
{
  vUSARTHandler(  );
}

void USART3_IRQHandler(void)
{
  vUSARTHandler(  );
}
