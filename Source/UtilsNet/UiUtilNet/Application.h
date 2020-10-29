/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MyApplicationContext.h"
#include "MainForm.h"

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for Application.
        /// </summary>

        public ref class Application sealed
        {
        public:
            static property Common::Forms::MainForm^ MainForm
            {
                Common::Forms::MainForm^ get()
                {
                    if (s_ApplicationContext == nullptr)
                    {
                        return nullptr;
                    }

                    return (Common::Forms::MainForm^)s_ApplicationContext->MainForm;
                }
            }

        public:
            /// <summary>
            /// Starts executing the application with a form.
            /// </summary>

            static System::Boolean Run(Common::Forms::MainForm^ MainForm);

        private:
            static System::Boolean Initialize(Common::Forms::MainForm^ MainForm);
            static System::Boolean Uninitialize();

        internal:
            static MyApplicationContext^ s_ApplicationContext = nullptr;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
