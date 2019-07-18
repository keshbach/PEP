/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Port.h"

#include "PortEnumerator.h"

Common::ListPorts::Enumerator::Enumerator() :
  m_PortList(nullptr),
  m_nCurrentPortIndex(-1),
  m_bAtEndOfList(false)
{
}

Common::ListPorts::Enumerator::~Enumerator()
{
    delete m_PortList;

    m_PortList = nullptr;
}

Common::ListPorts::Enumerator::Enumerator(
  System::Collections::Generic::List<Common::ListPorts::Port^>^ ZipItemList) :
  m_PortList(ZipItemList),
  m_nCurrentPortIndex(-1),
  m_bAtEndOfList(false)
{
}

System::Boolean Common::ListPorts::Enumerator::MoveNext(void)
{
    if (m_bAtEndOfList)
    {
        return false;
    }

    ++m_nCurrentPortIndex;

    if (m_nCurrentPortIndex < m_PortList->Count)
    {
        return true;
    }

    m_bAtEndOfList = true;

    return false;
}

void Common::ListPorts::Enumerator::Reset(void)
{
    m_nCurrentPortIndex = -1;
    m_bAtEndOfList = false;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
