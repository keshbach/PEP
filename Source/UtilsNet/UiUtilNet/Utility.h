/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for Utility.
        /// </summary>

        public ref class Utility sealed
        {
        public:
            /// <summary>
            /// Calculates the size of text based on the specified font.
            /// <param name="sText">
            /// Text to calculate the size of
            /// </param>
            /// <param name="TextFont">
            /// Font to calculate the text size with
            /// </param>
            /// </summary>

            static System::Drawing::Size^ CalcStringSize(System::String^ sText,
                                                         System::Drawing::Font^ TextFont);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
