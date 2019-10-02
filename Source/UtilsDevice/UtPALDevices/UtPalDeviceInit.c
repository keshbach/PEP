/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include "UtPalDeviceInit.h"
#include "UtPalDeviceAdapter.h"

#include <UtilsDevice/UtPepDevices.h>

#include <Utils/UtHeap.h>

BOOL UtPALDeviceInit(VOID)
{
    if (!UtPALDeviceAdapterInit())
    {
        return FALSE;
    }

    return UtInitHeap() ? TRUE : FALSE;
}

BOOL UtPALDeviceUninit(VOID)
{
    return UtUninitHeap() ? TRUE : FALSE;
}

/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/
