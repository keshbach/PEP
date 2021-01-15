/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ToolStripItemGroupImpl.h"

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for StatusStrip.
        /// </summary>

        public ref class StatusStrip : public ToolStripItemGroupImpl<System::Windows::Forms::StatusStrip>
        {
        public:
            StatusStrip();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~StatusStrip();

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
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
