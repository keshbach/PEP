/***************************************************************************/
/*  Copyright (C) 2007-2012 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include "UtPalDeviceInit.h"
#include "UtPalDeviceAdapter.h"

#include <UtilsDevice/UtPepDevices.h>
#include <UtilsDevice/UtPAL.h>

#include <Utils/UtHeap.h>

BOOL UtPALDeviceInit(VOID)
{
    if (!UtPALDeviceAdapterInit() || !UtPALInitialize())
    {
        return FALSE;
    }

    return UtInitHeap() ? TRUE : FALSE;
}

BOOL UtPALDeviceUninit(VOID)
{
    UtPALUninitialize();

    return UtUninitHeap() ? TRUE : FALSE;
}

/***************************************************************************/
/*  Copyright (C) 2007-2012 Kevin Eshbach                                  */
/***************************************************************************/
