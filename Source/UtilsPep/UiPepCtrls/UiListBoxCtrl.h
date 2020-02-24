/***************************************************************************/
/*  Copyright (C) 2020-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UiListBoxCtrl_H)
#define UiListBoxCtrl_H

#define CUiListBoxCtrlClass L"ListBoxCtrl"

typedef struct tagTListBoxCtrlNMClick
{
	NMHDR Hdr;
	POINT Point;
	INT nIndex;
} TListBoxCtrlNMClick;

VOID UiListBoxCtrlRegister(VOID);
VOID UiListBoxCtrlUnregister(VOID);

#endif /* end of UiListBoxCtrl_H */

/***************************************************************************/
/*  Copyright (C) 2020-2020 Kevin Eshbach                                  */
/***************************************************************************/
