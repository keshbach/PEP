/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for UtContextMenuStrip.
        /// </summary>

        private ref class UtContextMenuStrip sealed
        {
        internal:
            /// <summary>
            /// Saves all of the shortcut keys of the contained items.
            /// </summary>

            static void SaveShortcutKeys(System::Windows::Forms::ContextMenuStrip^ ContextMenuStrip);

            /// <summary>
            /// Restores all of the shortcut keys of the contained items.
            /// </summary>

            static void RestoreShortcutKeys(void);

            /// <summary>
            /// Clears all of the shortcut keys of the contained items.
            /// </summary>

            static void ClearShortcutKeys(void);

        private:
            ref struct TToolStripMenuItemShortcutKeys
            {
                System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem;
                System::Windows::Forms::Keys ShortcutKeys;
            };

        private:
            static void EnumToolStripItems(System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection);

        private:
            static System::Collections::Generic::List<TToolStripMenuItemShortcutKeys^>^ s_ToolStripMenuItemShortcutKeysList = nullptr;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
