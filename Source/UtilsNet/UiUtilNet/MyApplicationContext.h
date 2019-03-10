/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2017 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MainForm.h"

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for Application Context.
        /// </summary>

        private ref class MyApplicationContext : public System::Windows::Forms::ApplicationContext
        {
        internal:
            MyApplicationContext(Common::Forms::MainForm^ form);

        private:
            MyApplicationContext();
            MyApplicationContext(System::Windows::Forms::Form^ form);

        private:
            void OnFormClosed(System::Object^ sender, System::EventArgs^ e);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2017 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
