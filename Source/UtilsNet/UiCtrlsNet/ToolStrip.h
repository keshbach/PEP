/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ToolStrip.
        /// </summary>

        public ref class ToolStrip : System::Windows::Forms::ToolStrip
        {
        public:
            /// <summary>
            /// Indicates the group of ToolStripItem's that are active.
            /// </summary>

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
            property System::String^ ActiveGroup
            {
                System::String^ get()
                {
                    return m_sActiveGroupName;
                }

                void set(System::String^ value)
                {
                    ShowGroup(value);
                }
            }

        public:
            ToolStrip();

            /// <summary>
            /// Creates a new group and associates them with an array of ToolStripItems.
            /// </summary>

            void CreateGroup(System::String^ sGroupName,
                             array<System::Windows::Forms::ToolStripItem^>^ ItemsArray);

            /// <summary>
            /// Destroys a group associated with any array of ToolStripItems.
            /// </summary>

            void DestroyGroup(System::String^ sGroupName);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ToolStrip();

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

            void ShowGroup(System::String^ sGroupName);

        private:
            ref struct TGroupToolStripItems
            {
                System::String^ sGroupName;
                array<System::Windows::Forms::ToolStripItem^>^ ItemsArray;
            };

            TGroupToolStripItems^ FindGroup(System::String^ sGroupName);

            System::Collections::Generic::List<TGroupToolStripItems^>^ m_GroupToolStripItemsList;
            System::String^ m_sActiveGroupName;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
