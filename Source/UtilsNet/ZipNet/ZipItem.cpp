/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ZipItem.h"

Common::Zip::Item::Item(
  System::DateTime FileModDateTime,
  EFileAttributes FileAttributes,
  System::UInt32 nCRC,
  ECompressionMethod CompressionMethod,
  System::UInt64 nCompressedSize,
  System::UInt64 nUncompressedSize,
  System::String^ sFileName,
  System::String^ sPath,
  System::String^ sSrcFileName,
  System::String^ sComment) :
  m_FileModDateTime(FileModDateTime),
  m_FileAttributes(FileAttributes),
  m_nCRC(nCRC),
  m_CompressionMethod(CompressionMethod),
  m_nCompressedSize(nCompressedSize),
  m_nUncompressedSize(nUncompressedSize),
  m_sFileName(sFileName),
  m_sPath(sPath),
  m_sSrcFileName(sSrcFileName),
  m_sComment(sComment)
{
}

Common::Zip::Item::Item()
{
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
