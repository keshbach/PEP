/***************************************************************************/
/*  Copyright (C) 2010-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <UtilsPep/UiPepCtrls.h>

#include <Utils/UtHeap.h>

#include "UiBufferViewerCtrl.h"
#include "UiCheckedListBoxCtrl.h"
#include "UiListBoxCtrl.h"
#include "UiDeviceInfoCtrl.h"

extern "C"
{

VOID UIPEPCTRLSAPI UiPepCtrlsInitialize(VOID)
{
    UtInitHeap();

    UiBufferViewerCtrlRegister();
	UiListBoxCtrlRegister();
	UiCheckedListBoxCtrlRegister();
	UiDeviceInfoCtrlRegister();
}

VOID UIPEPCTRLSAPI UiPepCtrlsUninitialize(VOID)
{
    UiBufferViewerCtrlUnregister();
	UiListBoxCtrlUnregister();
	UiCheckedListBoxCtrlUnregister();
	UiDeviceInfoCtrlUnregister();

    UtUninitHeap();
}

};

/***************************************************************************/
/*  Copyright (C) 2010-2021 Kevin Eshbach                                  */
/***************************************************************************/
