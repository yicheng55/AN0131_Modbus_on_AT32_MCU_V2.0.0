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
static void prvvTIMERExpiredISR( void );

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
    crm_clocks_freq_type crm_clocks_freq_struct = {0};
    
    crm_clocks_freq_get(&crm_clocks_freq_struct);
    
    nvic_irq_disable(TMR2_GLOBAL_IRQn);
    
    /* enable tmr2 clock */
    crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);

    /* tmr2 configuration */
    /* time base configuration */
    tmr_base_init(TMR2, (usTim1Timerout50us * 50) - 1, (crm_clocks_freq_struct.ahb_freq / 1000000) - 1);
    tmr_cnt_dir_set(TMR2, TMR_COUNT_UP);
    
    NVIC_ClearPendingIRQ(TMR2_GLOBAL_IRQn);
    nvic_irq_enable(TMR2_GLOBAL_IRQn, 0, 0);
  
    return TRUE;
}

INLINE void
vMBPortTimersEnable( void )
{
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
    tmr_flag_clear(TMR2, TMR_OVF_FLAG);
    tmr_interrupt_enable(TMR2, TMR_OVF_INT, TRUE);
    tmr_counter_value_set(TMR2, 0);
    tmr_counter_enable(TMR2, TRUE);
}

INLINE void
vMBPortTimersDisable( void )
{
    /* Disable any pending timers. */
    tmr_flag_clear(TMR2, TMR_OVF_FLAG);
    tmr_interrupt_enable(TMR2, TMR_OVF_INT, FALSE);
    tmr_counter_value_set(TMR2, 0);
    tmr_counter_enable(TMR2, FALSE);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR( void )
{
    ( void )pxMBPortCBTimerExpired(  );
}

void TMR2_GLOBAL_IRQHandler(void)
{
    if(tmr_flag_get(TMR2, TMR_OVF_FLAG) != RESET)
    {
      tmr_flag_clear(TMR2, TMR_OVF_FLAG);
      prvvTIMERExpiredISR();
    }
}
