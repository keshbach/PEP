/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Debug
    {
        public ref class Thread sealed
        {
        public:
            [System::Diagnostics::Conditional("DEBUG")]
            static void IsWorkerThread();

            [System::Diagnostics::Conditional("DEBUG")]
            static void IsUIThread();

            [System::Diagnostics::Conditional("DEBUG")]
            static void IsAnyThread();
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
