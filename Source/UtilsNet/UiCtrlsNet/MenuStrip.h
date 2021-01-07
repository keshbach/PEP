/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for MenuStrip.
        /// </summary>

        public ref class MenuStrip : System::Windows::Forms::MenuStrip
        {
        public:
            /// <summary>
            /// Gets or sets the image list that contains the image displayed on a System.Windows.Forms.ToolStrip
            /// item and updates any children with the image list during a set.
            /// </summary>

            [System::ComponentModel::BrowsableAttribute(false)]
            property System::Windows::Forms::ImageList^ ImageList
            {
                System::Windows::Forms::ImageList^ get() new
                {
                    return System::Windows::Forms::MenuStrip::ImageList;
                }

                void set(System::Windows::Forms::ImageList^ value) new
                {
                    System::Windows::Forms::MenuStrip::ImageList = value;

                    UpdateToolStripItemsImageList(value, this->Items);
                }
            }

        public:
            MenuStrip();

            /// <summary>
            /// Saves a copy of the enable state of all of the contained items.
            /// </summary>

            void SaveItemsEnableState(void);

            /// <summary>
            /// Restores the last copy of the enable state of all of the contained items.
            /// </summary>

            void RestoreItemsEnableState(void);

            /// <summary>
            /// Disables all ToolstripItem's except those in the array.
            /// </summary>

            void DisableAllItems(array<System::Windows::Forms::ToolStripItem^>^ ExcludedItemsArray);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~MenuStrip();

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

        private:
            ref struct TToolStripItemState
            {
                System::Windows::Forms::ToolStripItem^ ToolStripItem;
                System::Boolean bEnabled;
            };

        private:
            static void UpdateToolStripItemsImageList(System::Windows::Forms::ImageList^ ImageList,
                                                      System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection);
            static void EnumToolStripItems(System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection,
                                           System::Collections::Generic::List<TToolStripItemState^>^ ToolStripItemStateList);
            static System::Boolean IsToolStripItemInArray(array<System::Windows::Forms::ToolStripItem^>^ ToolStripItemsArray,
                                                          System::Windows::Forms::ToolStripItem^ ToolStripItem);

        private:
            System::Collections::Generic::List<TToolStripItemState^>^ m_ToolStripItemStateList;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
