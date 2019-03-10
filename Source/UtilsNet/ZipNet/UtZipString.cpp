/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UtZipString.h"

#if defined(__cplusplus_cli)
#pragma unmanaged
#endif

bool CUtZipString::operator == (
  const CUtZipString& rhs)
{
    if (this == &rhs)
    {
        return true;
    }

    return (::lstrcmpW(m_pszUnicodeText, rhs.m_pszUnicodeText) == 0) ? true : false;
}

bool CUtZipString::operator == (
  LPCWSTR pszString)
{
    return (::lstrcmpW(m_pszUnicodeText, pszString) == 0) ? true : false;
}

CUtZipString::CUtZipString(
  const CUtZipString& rhs) :
  m_pszUnicodeText(NULL),
  m_nUnicodeTextLen(0),
  m_pszAnsiText(NULL),
  m_nAnsiTextLen(0)
{
    if (rhs.IsValid())
    {
        m_pszUnicodeText = new (std::nothrow) WCHAR[rhs.m_nUnicodeTextLen + 1];
        m_pszAnsiText = new (std::nothrow) CHAR[rhs.m_nAnsiTextLen + 1];

        if (m_pszUnicodeText && m_pszAnsiText)
        {
            m_nUnicodeTextLen = rhs.m_nUnicodeTextLen;
            m_nAnsiTextLen = rhs.m_nAnsiTextLen;

            ::StringCchCopyW(m_pszUnicodeText, m_nUnicodeTextLen + 1, rhs.m_pszUnicodeText);
            ::StringCchCopyA(m_pszAnsiText, m_nAnsiTextLen + 1, rhs.m_pszAnsiText);
        }
        else
        {
            delete[] m_pszUnicodeText;
            delete[] m_pszAnsiText;

            m_pszUnicodeText = NULL;
            m_pszAnsiText = NULL;
        }
    }
}

CUtZipString::CUtZipString(
  LPCSTR pszText,
  INT nTextLen,
  INT nCodePage)
{
    m_nUnicodeTextLen = ::MultiByteToWideChar(nCodePage, 0, pszText, nTextLen, NULL, 0);

    m_pszUnicodeText = new (std::nothrow) WCHAR[m_nUnicodeTextLen + 1];

    if (m_pszUnicodeText)
    {
        *(m_pszUnicodeText + m_nUnicodeTextLen) = 0;

        ::MultiByteToWideChar(nCodePage, 0, pszText, nTextLen, m_pszUnicodeText,
                              m_nUnicodeTextLen + 1);

        m_nAnsiTextLen = ::WideCharToMultiByte(CP_UTF8, 0, m_pszUnicodeText, -1,
                                               NULL, 0, NULL, NULL);

        m_pszAnsiText = new (std::nothrow) CHAR[m_nAnsiTextLen];

        if (m_pszAnsiText)
        {
            ::WideCharToMultiByte(CP_UTF8, 0, m_pszUnicodeText, -1, m_pszAnsiText,
                                  m_nAnsiTextLen, NULL, NULL);

            --m_nAnsiTextLen;
        }
        else
        {
            m_nAnsiTextLen = 0;
        }
    }
    else
    {
        m_pszAnsiText = NULL;
        m_nAnsiTextLen = 0;
        m_nUnicodeTextLen = 0;
    }
}

CUtZipString::CUtZipString(
  LPCWSTR pszText)
{
    m_nUnicodeTextLen = ::lstrlenW(pszText);
    m_pszUnicodeText = new (std::nothrow) WCHAR[m_nUnicodeTextLen + 1];

    if (m_pszUnicodeText != NULL)
    {
        ::StringCchCopyW(m_pszUnicodeText, m_nUnicodeTextLen + 1, pszText);
    }
    else
    {
        m_nUnicodeTextLen = 0;
    }

    m_nAnsiTextLen = ::WideCharToMultiByte(CP_UTF8, 0, pszText, -1, NULL, 0,
                                           NULL, NULL);

    m_pszAnsiText = new (std::nothrow) CHAR[m_nAnsiTextLen];

    if (m_pszAnsiText)
    {
        ::WideCharToMultiByte(CP_UTF8, 0, pszText, -1, m_pszAnsiText, m_nAnsiTextLen,
                              NULL, NULL);

        --m_nAnsiTextLen;
    }
    else
    {
        m_nAnsiTextLen = 0;
    }
}

CUtZipString::~CUtZipString()
{
    delete[] m_pszUnicodeText;
    delete[] m_pszAnsiText;
}

#if defined(__cplusplus_cli)
#pragma unmanaged
#endif

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
