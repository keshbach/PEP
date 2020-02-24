/***************************************************************************/
/*  Copyright (C) 2010-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <UtilsPep/UiPepCtrls.h>

#include <Utils/UtHeap.h>

#include "UiBufferViewerCtrl.h"
#include "UiCheckedListBoxCtrl.h"
#include "UiListBoxCtrl.h"

extern "C"
{

VOID UIPEPCTRLSAPI UiPepCtrlsInitialize(VOID)
{
    UtInitHeap();

    UiBufferViewerCtrlRegister();
	UiListBoxCtrlRegister();
	UiCheckedListBoxCtrlRegister();
}

VOID UIPEPCTRLSAPI UiPepCtrlsUninitialize(VOID)
{
    UiBufferViewerCtrlUnregister();
	UiListBoxCtrlUnregister();
	UiCheckedListBoxCtrlUnregister();

    UtUninitHeap();
}

};

/***************************************************************************/
/*  Copyright (C) 2010-2020 Kevin Eshbach                                  */
/***************************************************************************/
