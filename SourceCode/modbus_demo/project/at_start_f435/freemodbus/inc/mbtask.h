
#ifndef __MBTASK_H
#define __MBTASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_SLAVE_ADDRESS                ( 0x01 )
#define MB_BAUDRATE                     ( 9600 )

//input register start address
#define REG_INPUT_START                 ( 1 )
//input register number
#define REG_INPUT_NREGS                 ( 16 )

//holding register start address
#define REG_HOLDING_START               ( 1 )
//holding register number
#define REG_HOLDING_NREGS               ( 16 )

//coils start address
#define REG_COILS_START                 ( 1 )
//coils number
#define REG_COILS_SIZE                  ( 16 )

//discrete inputs start address
#define REG_DISCRETE_START              ( 1 )
//discrete inputs number
#define REG_DISCRETE_SIZE               ( 16 )

void modbus_task(void);

#ifdef __cplusplus
}
#endif

#endif
