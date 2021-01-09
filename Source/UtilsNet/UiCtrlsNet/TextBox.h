/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// TextBox with a timer delay after a key has been pressed.
        /// </summary>

        public ref class TextBox : System::Windows::Forms::TextBox
        {
        public:
            TextBox();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~TextBox();

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
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
