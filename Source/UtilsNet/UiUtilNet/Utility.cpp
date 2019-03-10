/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Utility.h"

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

#pragma managed

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

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
