/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ToolTip
        /// </summary>

        public ref class ToolTip : System::ComponentModel::Component
        {
        public:
            ToolTip();
            ToolTip(System::ComponentModel::IContainer^ container);

            System::Boolean CreateToolTip(System::Windows::Forms::Form^ ParentForm);
            System::Boolean DestroyToolTip(void);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ToolTip();

        private:
            /// <summary>
            /// Required designer variable.
            /// </summary>
            System::ComponentModel::Container^ components;

            /// <summary>
            /// Required method for Designer support - do not modify
            /// the contents of this method with the code editor.
            /// </summary>		
            void InitializeComponent(void);

        protected:
            HWND m_hToolTip;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
