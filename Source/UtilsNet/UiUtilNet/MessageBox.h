/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for MessageBox.
        /// </summary>

        public ref class MessageBox sealed
        {
        public:
            /// <summary>
            /// Displays a message box that is parented to the main form.
            /// </summary>

            static System::Windows::Forms::DialogResult Show(
                System::String^ sMsg,
                System::Windows::Forms::MessageBoxButtons Buttons,
                System::Windows::Forms::MessageBoxIcon Icon);

            /// <summary>
            /// Displays a message box that is parented to the given form.
            /// </summary>

            static System::Windows::Forms::DialogResult Show(
                System::Windows::Forms::IWin32Window^ Owner,
                System::String^ sMsg,
                System::Windows::Forms::MessageBoxButtons Buttons,
                System::Windows::Forms::MessageBoxIcon Icon);

            /// <summary>
            /// Displays a message box that is parented to the main form.
            /// </summary>

            static System::Windows::Forms::DialogResult Show(
                System::String^ sMsg,
                System::Windows::Forms::MessageBoxButtons Buttons,
                System::Windows::Forms::MessageBoxIcon Icon,
                System::Windows::Forms::MessageBoxDefaultButton DefButton);

            /// <summary>
            /// Displays a message box that is parented to the given form.
            /// </summary>

            static System::Windows::Forms::DialogResult Show(
                System::Windows::Forms::IWin32Window^ Owner,
                System::String^ sMsg,
                System::Windows::Forms::MessageBoxButtons Buttons,
                System::Windows::Forms::MessageBoxIcon Icon,
                System::Windows::Forms::MessageBoxDefaultButton DefButton);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
