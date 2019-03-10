/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/

#include "UiPepCtrls.h"

#include <Utils/UtHeap.h>

#include "UiBufferViewerCtrl.h"

extern "C"
{

VOID UIPEPCTRLSAPI UiPepCtrlsInitialize(VOID)
{
    UtInitHeap();

    UiBufferViewerCtrlRegister();
}

VOID UIPEPCTRLSAPI UiPepCtrlsUninitialize(VOID)
{
    UiBufferViewerCtrlUnregister();

    UtUninitHeap();
}

};

/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/
