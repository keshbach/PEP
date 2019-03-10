/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for FormTimer.
        /// </summary>

        public ref class FormTimer : System::Windows::Forms::Timer
        {
        public:
            /// <summary>
            /// Form object that should be associated with this timer.
            /// </summary>

            property System::Object^ FormObject
            {
                System::Object^ get()
                {
                    return m_FormObject;
                }

                void set(System::Object^ value)
                {
                    m_FormObject = value;
                }
            }

        public:
            FormTimer();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~FormTimer();

        private:
            /// <summary>
            /// Required designer variable.
            /// </summary>
            System::ComponentModel::Container^ components;

            /// <summary>
            /// Required method for Designer support - do not modify
            /// the contents of this method with the code editor.
            /// </summary>		
            void InitializeComponent(void)
            {
            }

        protected:
            System::Object^ m_FormObject;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
