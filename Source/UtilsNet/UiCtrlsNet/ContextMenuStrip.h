/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ToolStripItemGroupImpl.h"

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ContextMenuStrip.
        /// </summary>
        
        public ref class ContextMenuStrip : public ToolStripItemGroupImpl<System::Windows::Forms::ContextMenuStrip>
        {
        public:
            ContextMenuStrip();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ContextMenuStrip();

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
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
