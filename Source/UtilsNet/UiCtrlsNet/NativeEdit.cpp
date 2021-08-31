/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ContextMenuStrip.h"
#include "EditContextMenuStrip.h"
#include "ITextBoxKeyPress.h"
#include "NativeEdit.h"

#pragma region "Constants"

#define CEditWndProcPropName L"KE_EditWndProc"

#pragma endregion

static LRESULT CALLBACK lEditWindowProc(_In_ HWND hWnd, _In_ UINT nMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

#pragma region "Local Functions"

static VOID lSubclassEditControl(
  _In_ HWND hWnd)
{
    WNDPROC pWndProc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);

    ::SetPropW(hWnd, CEditWndProcPropName, pWndProc);

    ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)lEditWindowProc);
}

static VOID lUnsubclassEditControl(
  _In_ HWND hWnd)
{
    WNDPROC pPrevWndProc = (WNDPROC)::GetPropW(hWnd, CEditWndProcPropName);
    Common::Forms::NativeEdit^ NativeEdit = Common::Forms::NativeEdit::s_IntPtrNativeEditDict[System::IntPtr(hWnd)];

    ::RemovePropW(hWnd, CEditWndProcPropName);

    ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pPrevWndProc);

    delete NativeEdit->m_EditContextMenuStrip;

    NativeEdit->m_TextBoxKeyPress = nullptr;
    NativeEdit->m_EditContextMenuStrip = nullptr;

    Common::Forms::NativeEdit::s_IntPtrNativeEditDict->Remove(System::IntPtr(hWnd));
}

static BOOL lEditProcessKeyDownMsg(
  _In_ HWND hWnd,
  _In_ INT nKeyCode)
{
    Common::Forms::NativeEdit^ NativeEdit = Common::Forms::NativeEdit::s_IntPtrNativeEditDict[System::IntPtr(hWnd)];

    return NativeEdit->m_EditContextMenuStrip->ProcessKeyDown(nKeyCode);
}

static BOOL lEditProcessKeyUpMsg(
  _In_ HWND hWnd,
  _In_ INT nKeyCode)
{
    Common::Forms::NativeEdit^ NativeEdit = Common::Forms::NativeEdit::s_IntPtrNativeEditDict[System::IntPtr(hWnd)];

    return NativeEdit->m_EditContextMenuStrip->ProcessKeyUp(nKeyCode);
}

static BOOL lEditProcessCharMsg(
  _In_ HWND hWnd,
  _In_ INT nKeyCode)
{
    Common::Forms::NativeEdit^ NativeEdit = Common::Forms::NativeEdit::s_IntPtrNativeEditDict[System::IntPtr(hWnd)];

    return NativeEdit->m_TextBoxKeyPress->OnTextBoxKeyPress((wchar_t)nKeyCode) ? TRUE : FALSE;
}

static VOID lEditProcessContextMenuMsg(
  _In_ HWND hWnd,
  _In_ INT nXPos,
  _In_ INT nYPos)
{
    Common::Forms::NativeEdit^ NativeEdit = Common::Forms::NativeEdit::s_IntPtrNativeEditDict[System::IntPtr(hWnd)];
    POINT Point;

    Point.x = nXPos;
    Point.y = nYPos;

    NativeEdit->m_EditContextMenuStrip->DisplayContextMenuStrip(&Point);
}

static LRESULT CALLBACK lEditWindowProc(
  _In_ HWND hWnd,
  _In_ UINT nMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam)
{
    WNDPROC pPrevWndProc = (WNDPROC)::GetPropW(hWnd, CEditWndProcPropName);

    switch (nMsg)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (lEditProcessKeyDownMsg(hWnd, (INT)wParam))
            {
                return 0;
            }
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (lEditProcessKeyUpMsg(hWnd, (INT)wParam))
            {
                return 0;
            }
            break;
        case WM_CHAR:
            if (lEditProcessCharMsg(hWnd, (INT)wParam))
            {
                return 0;
            }
            break;
        case WM_CONTEXTMENU:
            lEditProcessContextMenuMsg(hWnd, GET_X_LPARAM(lParam),
                                       GET_Y_LPARAM(lParam));

            return 0;
        case WM_DESTROY:
            lUnsubclassEditControl(hWnd);
            break;
    }

    return ::CallWindowProc(pPrevWndProc, hWnd, nMsg, wParam, lParam);
}

#pragma endregion

#pragma region "Constructor"

Common::Forms::NativeEdit::NativeEdit(
  _In_ HWND hWnd,
  Common::Forms::ITextBoxKeyPress^ TextBoxKeyPress) :
  m_TextBoxKeyPress(TextBoxKeyPress)
{
    m_EditContextMenuStrip = gcnew Common::Forms::EditContextMenuStrip(hWnd);

    s_IntPtrNativeEditDict->Add(System::IntPtr(hWnd), this);

    lSubclassEditControl(hWnd);
}

#pragma endregion

#pragma region "Destructor"

Common::Forms::NativeEdit::~NativeEdit()
{
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
