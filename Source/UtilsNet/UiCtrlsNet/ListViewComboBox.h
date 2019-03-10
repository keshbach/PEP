/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ListViewComboBox.
        /// </summary>

        public ref class ListViewComboBox : Common::Forms::ComboBox
        {
        public:
            ref class ListViewComboBoxKeydownEventArgs : System::EventArgs
            {
            public:
                property System::Windows::Forms::Keys Key
                {
                    System::Windows::Forms::Keys get()
                    {
                        return m_Key;
                    }

                    void set(System::Windows::Forms::Keys value)
                    {
                        m_Key = value;
                    }
                }

            public:
                ListViewComboBoxKeydownEventArgs(
                    System::Windows::Forms::Keys Key)
                {
                    m_Key = Key;
                }

            private:
                System::Windows::Forms::Keys m_Key;

            protected:
                ListViewComboBoxKeydownEventArgs() { }
            };

            // List View Combo Box Events
        public:
            delegate void ListViewComboBoxKeydownHandler(System::Object^ sender, ListViewComboBoxKeydownEventArgs^ e);
            event ListViewComboBoxKeydownHandler^ ListViewComboBoxKeydown;

            delegate void ListViewComboBoxSetFocusHandler(System::Object^ sender, System::EventArgs^ e);
            event ListViewComboBoxSetFocusHandler^ ListViewComboBoxSetFocus;

            delegate void ListViewComboBoxKillFocusHandler(System::Object^ sender, System::EventArgs^ e);
            event ListViewComboBoxKillFocusHandler^ ListViewComboBoxKillFocus;

        public:
            ListViewComboBox();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ListViewComboBox();

            virtual void WndProc(System::Windows::Forms::Message% Message) override;

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

            void HandleSetFocus(System::Windows::Forms::Message% Message);
            void HandleKillFocus(System::Windows::Forms::Message% Message);
            void HandleKeydown(System::Windows::Forms::Message% Message);
            void HandleGetDlgCode(System::Windows::Forms::Message% Message);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
