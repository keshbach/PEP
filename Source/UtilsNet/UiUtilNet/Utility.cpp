/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Utility.h"

#include "Includes/UtTemplates.h"

#include "Includes/UtKeyboard.inl"

#pragma unmanaged

static VOID lCalcStringSize(
  LPCWSTR pszText,
  HFONT hFont,
  LPSIZE pSize)
{
    HDC hDC = ::CreateDCW(L"DISPLAY", NULL, NULL, NULL);

    pSize->cx = 0;
    pSize->cy = 0;

    if (hDC)
    {
        ::SaveDC(hDC);

        ::SelectObject(hDC, hFont);

        ::GetTextExtentPoint32W(hDC, pszText, ::lstrlenW(pszText), pSize);

        ::RestoreDC(hDC, -1);

        ::DeleteDC(hDC);
    }
}

static INT lKeyCodeFromText(
  _In_z_ LPCWSTR pszText)
{
    while (*pszText != NULL)
    {
        while (*pszText != NULL && *pszText != '&')
        {
            ++pszText;
        }

        if (*pszText == NULL || *(pszText + 1) == NULL)
        {
            return 0;
        }

        ++pszText;

        if (*pszText == NULL)
        {
            return 0;
        }

        if (*pszText != '&')
        {
            return *pszText;
        }

        ++pszText;
    }

    return 0;
}

#pragma managed

static BOOL lKeyCodeMatchesToolStripMenuItemByShortcutKeys(
  _In_ INT nKeyCode,
  System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem)
{
    BOOL bShiftPressed, bControlPressed, bAltPressed;

    if (nKeyCode != (((INT)ToolStripMenuItem->ShortcutKeys) & 0xFF))
    {
        return FALSE;
    }

    bShiftPressed = UtIsKeyDown(VK_SHIFT);
    bControlPressed = UtIsKeyDown(VK_CONTROL);
    bAltPressed = UtIsKeyDown(VK_MENU);

    if (((INT)ToolStripMenuItem->ShortcutKeys & (INT)System::Windows::Forms::Keys::Shift) != 0)
    {
        if (!bShiftPressed)
        {
            return FALSE;
        }
    }
    else
    {
        if (bShiftPressed)
        {
            return FALSE;
        }
    }

    if (((INT)ToolStripMenuItem->ShortcutKeys & (INT)System::Windows::Forms::Keys::Control) != 0)
    {
        if (!bControlPressed)
        {
            return FALSE;
        }
    }
    else
    {
        if (bControlPressed)
        {
            return FALSE;
        }
    }

    if (((INT)ToolStripMenuItem->ShortcutKeys & (INT)System::Windows::Forms::Keys::Alt) != 0)
    {
        if (!bAltPressed)
        {
            return FALSE;
        }
    }
    else
    {
        if (bAltPressed)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL lKeyCodeMatchesToolStripMenuItemByHotKey(
  _In_ INT nKeyCode,
  System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem)
{
    pin_ptr<const wchar_t> pszText = PtrToStringChars(ToolStripMenuItem->Text);
    INT nToolStripMenuItemKeyCode = lKeyCodeFromText(pszText);
    WCHAR cBuffer[2];

    if (nToolStripMenuItemKeyCode == 0)
    {
        return FALSE;
    }

    cBuffer[0] = (WCHAR)nToolStripMenuItemKeyCode;
    cBuffer[1] = NULL;

    ::CharUpper(cBuffer);

    nToolStripMenuItemKeyCode = cBuffer[0];

    if (nKeyCode == nToolStripMenuItemKeyCode)
    {
        return TRUE;
    }

    return FALSE;
}

System::Drawing::Size^ Common::Forms::Utility::CalcStringSize(
  System::String^ sText,
  System::Drawing::Font^ TextFont)
{
    System::Drawing::Size^ DrawingSize = gcnew System::Drawing::Size(0, 0);
    System::String^ sTmpText(sText);
    HFONT hFont = (HFONT)TextFont->ToHfont().ToPointer();
    SIZE Size;
    pin_ptr<const wchar_t> pszText;

    if (hFont)
    {
        sTmpText += L"  ";

        pszText = PtrToStringChars(sTmpText);

        lCalcStringSize(pszText, hFont, &Size);

        ::DeleteObject(hFont);

        DrawingSize->Width = Size.cx;
        DrawingSize->Height = Size.cy;
    }

    return DrawingSize;
}

System::Windows::Forms::ToolStripMenuItem^ Common::Forms::Utility::FindToolStripMenuItemByShortcutKeys(
  System::Int32 nKeyCode,
  System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection)
{
    System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem;

    for each (System::Windows::Forms::ToolStripItem ^ ToolStripItem in ToolStripItemCollection)
    {
        if (IsInstance<System::Windows::Forms::ToolStripMenuItem^>(ToolStripItem))
        {
            ToolStripMenuItem = (System::Windows::Forms::ToolStripMenuItem^)ToolStripItem;

            if (lKeyCodeMatchesToolStripMenuItemByShortcutKeys(nKeyCode, ToolStripMenuItem))
            {
                return ToolStripMenuItem;
            }
        }
    }

    return nullptr;
}

System::Windows::Forms::ToolStripMenuItem^ Common::Forms::Utility::FindToolStripMenuItemByHotKey(
  System::Int32 nKeyCode,
  System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection)
{
    System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem;

    for each (System::Windows::Forms::ToolStripItem ^ ToolStripItem in ToolStripItemCollection)
    {
        if (IsInstance<System::Windows::Forms::ToolStripMenuItem^>(ToolStripItem))
        {
            ToolStripMenuItem = (System::Windows::Forms::ToolStripMenuItem^)ToolStripItem;

            if (lKeyCodeMatchesToolStripMenuItemByHotKey(nKeyCode, ToolStripMenuItem))
            {
                return ToolStripMenuItem;
            }
        }
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
