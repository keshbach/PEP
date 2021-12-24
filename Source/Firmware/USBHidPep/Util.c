/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <p18cxxx.h>
#include "UtTypeDefs.h"
#include "UtPortTypeDefs.h"
#include "Util.h"

#pragma code

void UtClearArray(UINT8* pnStartAddress, UINT8 nCount)
{
    *pnStartAddress;

    while (nCount)
    {
        _asm
        clrf POSTINC0,0
        _endasm
        nCount--;
    }
}

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
