/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        private ref class EditContextMenuStrip
        {
        private:
            EditContextMenuStrip();

        internal:
            EditContextMenuStrip(System::Windows::Forms::TextBox^ TextBox);
            EditContextMenuStrip(System::Windows::Forms::MaskedTextBox^ MaskedTextBox);
            EditContextMenuStrip(HWND hEdit);

            void DisplayContextMenuStrip(LPPOINT pPoint);

            BOOL ProcessKeyDown(INT nKeyCode);
            BOOL ProcessKeyUp(INT nKeyCode);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~EditContextMenuStrip();

        private:
            void CreateContextMenu();
            void DestroyContextMenu();

            void CreateToolStripGroups();
            void DestroyToolStripGroups();

        // Context menu event handlers
        private:
            void toolStripMenuItemUndo_Click(System::Object^ Object, System::EventArgs^ EventArgs);
            void toolStripMenuItemCut_Click(System::Object^ Object, System::EventArgs^ EventArgs);
            void toolStripMenuItemCopy_Click(System::Object^ Object, System::EventArgs^ EventArgs);
            void toolStripMenuItemPaste_Click(System::Object^ Object, System::EventArgs^ EventArgs);
            void toolStripMenuItemDelete_Click(System::Object^ Object, System::EventArgs^ EventArgs);
            void toolStripMenuItemSelectAll_Click(System::Object^ Object, System::EventArgs^ EventArgs);
            void contextMenuStrip_Opening(System::Object^ Object, System::ComponentModel::CancelEventArgs^ EventArgs);

        private:
            System::Windows::Forms::TextBox^ m_TextBox;
            System::Windows::Forms::MaskedTextBox^ m_MaskedTextBox;

            HWND m_hEdit;

            Common::Forms::ContextMenuStrip^ m_ContextMenuStrip;

            System::Windows::Forms::ToolStripMenuItem^ m_ToolStripMenuItemUndo;
            System::Windows::Forms::ToolStripSeparator^ m_ToolStripSeparator1;
            System::Windows::Forms::ToolStripMenuItem^ m_ToolStripMenuItemCut;
            System::Windows::Forms::ToolStripMenuItem^ m_ToolStripMenuItemCopy;
            System::Windows::Forms::ToolStripMenuItem^ m_ToolStripMenuItemPaste;
            System::Windows::Forms::ToolStripMenuItem^ m_ToolStripMenuItemDelete;
            System::Windows::Forms::ToolStripSeparator^ m_ToolStripSeparator2;
            System::Windows::Forms::ToolStripMenuItem^ m_ToolStripMenuItemSelectAll;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
