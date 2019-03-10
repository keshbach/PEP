/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace IO
    {
        public ref class Path sealed
        {
        public:
            /// <summary>
            /// Resolves a path by removing the current and up path special characters.
            /// <param name="sRootPath">
            /// Root of the path that contains the path to resolve.
            /// </param>
            /// <param name="sPath">
            /// Path to resolve.
            /// </param>
            /// </summary>

            static System::String^ Canonicalize(System::String^ sRootPath, System::String^ sPath);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
