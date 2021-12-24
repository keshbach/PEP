/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <p18cxxx.h>

#include "UtTypeDefs.h"

#include "UtPortTypeDefs.h"

#pragma udata
UINT8 g_nControlTransferSessionOwner;

TPointer g_pSrc; // Data source pointer
TPointer g_pDst; // Data destination pointer
UINT16 g_wCount; // Data counter

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
