/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        private ref class ToolStripComparer : public System::Collections::Generic::IComparer<System::Windows::Forms::ToolStrip^>
        {
        internal:
            ToolStripComparer();

        public:
            // System::Collections::Generic::IComparer
            virtual int Compare(System::Windows::Forms::ToolStrip^ x, System::Windows::Forms::ToolStrip^ y);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
