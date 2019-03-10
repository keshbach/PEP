/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CUtZipString
{
public:
    bool operator == (const CUtZipString& rhs);
    bool operator == (LPCWSTR pszString);

public:
    CUtZipString(const CUtZipString& rhs);
    CUtZipString(LPCSTR pszText, INT nTextLen, INT nCodePage);
    CUtZipString(LPCWSTR pszText);
    virtual ~CUtZipString();

    inline LPCWSTR AsUnicode() const;
    inline LPCSTR AsAnsi() const;

    inline INT UnicodeLength() const;
    inline INT AnsiLength() const;

    inline bool IsValid() const;

    // operator overload compare two instances, compare to ansi or unicode text

protected:
    LPWSTR m_pszUnicodeText;
    INT m_nUnicodeTextLen;
    LPSTR m_pszAnsiText;
    INT m_nAnsiTextLen;

private:
    CUtZipString();

    CUtZipString& operator = (const CUtZipString&);
};

inline LPCWSTR CUtZipString::AsUnicode() const
{
    return m_pszUnicodeText;
}

inline LPCSTR CUtZipString::AsAnsi() const
{
    return m_pszAnsiText;
}

inline INT CUtZipString::UnicodeLength() const
{
    return m_nUnicodeTextLen;
}

inline INT CUtZipString::AnsiLength() const
{
    return m_nAnsiTextLen;
}

inline bool CUtZipString::IsValid() const
{
    return (m_pszUnicodeText && m_pszAnsiText) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
