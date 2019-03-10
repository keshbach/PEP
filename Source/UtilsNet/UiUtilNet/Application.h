/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2017 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IProcessMessage.h"
#include "ProcessMessage.h"
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
            /// <summary>
            /// A collection of objects to be notified when a key or mouse message is received.
            /// </summary>
            static property System::Collections::Generic::List<IProcessMessage^>^ ProcessMessageList
            {
                System::Collections::Generic::List<IProcessMessage^>^ get()
                {
                    if (s_ProcessMessage == nullptr)
                    {
                        s_ProcessMessage = gcnew Common::Forms::ProcessMessage();
                    }

                    return s_ProcessMessage->ProcessMessageList;
                }
            }

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

        private:
            static ProcessMessage^ s_ProcessMessage = nullptr;

        internal:
            static GetMessageCallback^ s_GetMessageCallback = nullptr;
            static MyApplicationContext^ s_ApplicationContext = nullptr;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2017 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
