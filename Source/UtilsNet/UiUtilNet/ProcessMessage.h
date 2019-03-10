/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2016 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IProcessMessage.h"

namespace Common
{
    namespace Forms
    {
        private delegate void GetMessageCallback(System::Windows::Forms::Message^ msg);

        private ref class ProcessMessage
        {
        internal:
            property System::Collections::Generic::List<IProcessMessage^>^ ProcessMessageList
            {
                System::Collections::Generic::List<IProcessMessage^>^ get()
                {
                    return m_ProcessMessageList;
                }
            }

        internal:
            ProcessMessage();
            ~ProcessMessage();
            !ProcessMessage();

        internal:
            void GetMessageCallback(System::Windows::Forms::Message^ msg);

        private:
            System::Collections::Generic::List<IProcessMessage^>^ m_ProcessMessageList;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2016 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
