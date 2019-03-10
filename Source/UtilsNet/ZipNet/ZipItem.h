/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

using namespace System;

namespace Common
{
    namespace Zip
    {
        public ref class Item sealed
        {
        public:
            enum class ECompressionMethod
            {
                Unknown,
                Stored,
                Deflated
            };

            [System::FlagsAttribute]
            enum class EFileAttributes : System::UInt32
            {
                None = 0x0000,
                Hidden = 0x0001,
                ReadOnly = 0x0002,
                System = 0x0004,
                Archive = 0x0008
            };

        public:
            property System::DateTime FileModDateTime
            {
                System::DateTime get()
                {
                    return m_FileModDateTime;
                }
            }

            property EFileAttributes FileAttributes
            {
                EFileAttributes get()
                {
                    return m_FileAttributes;
                }
            }

            property System::UInt32 CRC
            {
                System::UInt32 get()
                {
                    return m_nCRC;
                }
            }

            property ECompressionMethod CompressionMethod
            {
                ECompressionMethod get()
                {
                    return m_CompressionMethod;
                }
            }

            property System::UInt64 CompressedSize
            {
                System::UInt64 get()
                {
                    return m_nCompressedSize;
                }
            }

            property System::UInt64 UncompressedSize
            {
                System::UInt64 get()
                {
                    return m_nUncompressedSize;
                }
            }

            property System::String^ FileName
            {
                System::String^ get()
                {
                    return m_sFileName;
                }
            }

            property System::String^ Path
            {
                System::String^ get()
                {
                    return m_sPath;
                }
            }

            property System::String^ SrcFileName
            {
                System::String^ get()
                {
                    return m_sSrcFileName;
                }
            }

            property System::String^ Comment
            {
                System::String^ get()
                {
                    return m_sComment;
                }
            }

        internal:
            Item(System::DateTime FileModDateTime,
                 EFileAttributes FileAttributes,
                 System::UInt32 nCRC,
                 ECompressionMethod CompressionMethod,
                 System::UInt64 nCompressedSize,
                 System::UInt64 nUncompressedSize,
                 System::String^ sFileName,
                 System::String^ sPath,
                 System::String^ sSrcFileName,
                 System::String^ sComment);

        private:
            Item();

        private:
            System::DateTime m_FileModDateTime;
            EFileAttributes m_FileAttributes;
            System::UInt32 m_nCRC;
            ECompressionMethod m_CompressionMethod;
            System::UInt64 m_nCompressedSize;
            System::UInt64 m_nUncompressedSize;
            System::String^ m_sFileName;
            System::String^ m_sPath;
            System::String^ m_sSrcFileName;
            System::String^ m_sComment;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
