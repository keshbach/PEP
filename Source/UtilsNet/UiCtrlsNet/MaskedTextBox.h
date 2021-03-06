/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        public ref class MaskedTextBox : System::Windows::Forms::MaskedTextBox
        {
        public:
            MaskedTextBox();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~MaskedTextBox();

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
            virtual void WndProc(System::Windows::Forms::Message% Message) override;

        private:
            Common::Forms::EditContextMenuStrip^ m_EditContextMenuStrip;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
