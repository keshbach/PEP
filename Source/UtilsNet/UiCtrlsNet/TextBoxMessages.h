/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        public ref class TextBoxMessages : System::Windows::Forms::TextBox
        {
            // Text Box Messages Events
        public:
            delegate void TextSelectedHandler(System::Object^ sender, Common::Forms::TextBoxMessagesTextSelectedEventArgs^ e);
            [System::ComponentModel::Description("Occurs when text is selected or not."),
                System::ComponentModel::Category("TextSelected")]
                event TextSelectedHandler^ TextSelected;

        public:
            TextBoxMessages();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~TextBoxMessages();

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

        protected:
            void OnTextSelected(Common::Forms::TextBoxMessagesTextSelectedEventArgs^ e);

        private:
            void UpdateTextSelected(System::Boolean bTextSelected);

        private:
            System::Boolean m_bTextSelected;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
