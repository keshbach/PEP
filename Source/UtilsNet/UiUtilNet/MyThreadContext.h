/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for Thread Context.
        /// </summary>

        private ref class MyThreadContext sealed
        {
        internal:
            MyThreadContext();

        internal:
			void OnThreadExit(System::Object^ sender, System::EventArgs^ e);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////