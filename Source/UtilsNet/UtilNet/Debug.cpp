/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Debug.h"

// https://stackoverflow.com/questions/2374451/how-to-tell-if-a-thread-is-the-main-thread-in-c-sharp

void Common::Debug::Thread::IsWorkerThread()
{
#if !defined(NDEBUG)
    if (System::Threading::SynchronizationContext::Current != nullptr)
    {
        System::Diagnostics::Debug::Assert(false, "Not a Worker thread.");
    }
#endif
}

void Common::Debug::Thread::IsUIThread()
{
#if !defined(NDEBUG)
    if (System::Threading::SynchronizationContext::Current == nullptr)
    {
        System::Diagnostics::Debug::Assert(false, "Not the UI thread.");
    }
#endif
}

void Common::Debug::Thread::IsAnyThread()
{
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
