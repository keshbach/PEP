/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for NativeEdit
        /// </summary>

        private ref class NativeEdit
        {
        internal:
            NativeEdit(HWND hWnd,
                       Common::Forms::ITextBoxKeyPress^ TextBoxKeyPress,
                       Common::Forms::ITextBoxClipboard^ TextBoxClipboard);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~NativeEdit();

        internal:
            Common::Forms::ITextBoxKeyPress^ m_TextBoxKeyPress;
            Common::Forms::EditContextMenuStrip^ m_EditContextMenuStrip;

            static System::Collections::Generic::Dictionary<System::IntPtr, NativeEdit^>^ s_IntPtrNativeEditDict = gcnew System::Collections::Generic::Dictionary<System::IntPtr, NativeEdit^>();
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
