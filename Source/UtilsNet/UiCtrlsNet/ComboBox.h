/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ComboBox.
        /// </summary>

        public ref class ComboBox : System::Windows::Forms::ComboBox
        {
        public:
            property int SelectedIndex
            {
                int get() new
                {
                    return dynamic_cast<System::Windows::Forms::ComboBox^>(this)->SelectedIndex;
                }
                void set(int value) new
                {
                    dynamic_cast<System::Windows::Forms::ComboBox^>(this)->SelectedIndex = value;
                }
            }

        public:
            ComboBox();

            /// <summary>
            /// Autosizes the width of the drop down to completely display the largest item.
            /// </summary>

            void AutosizeDropDown(void);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ComboBox();

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

        private:
            HWND m_hTooltip;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
