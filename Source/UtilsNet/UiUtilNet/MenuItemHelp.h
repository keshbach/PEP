/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for MenuItemHelp.
        /// </summary>

        [System::Serializable]
        public ref class MenuItemHelp : System::Runtime::Serialization::ISerializable,
                                        System::IEquatable<MenuItemHelp^>,
                                        System::IComparable<MenuItemHelp^>
        {
        public:
            /// <summary>
            /// The actual ToolStripMenuItem object.
            /// </summary>
            property System::Windows::Forms::ToolStripMenuItem^ MenuItem
            {
                System::Windows::Forms::ToolStripMenuItem^ get()
                {
                    return FindToolStripMenuItem(m_sMenuItemName);
                }

                void set(System::Windows::Forms::ToolStripMenuItem^ value)
                {
                    m_sMenuItemName = value->Name;
                }
            }

            /// <summary>
            /// The help text associated with a ToolStripMenuItem object.
            /// </summary>
            property System::String^ HelpText
            {
                System::String^ get()
                {
                    return m_sHelpText;
                }

                void set(System::String^ value)
                {
                    m_sHelpText = value;
                }
            }

        public:
            MenuItemHelp();
            MenuItemHelp(System::Runtime::Serialization::SerializationInfo^ info,
                         System::Runtime::Serialization::StreamingContext context);

        // System::Runtime::Serialization::ISerializable
        public:
            virtual void GetObjectData(System::Runtime::Serialization::SerializationInfo^ info,
                                       System::Runtime::Serialization::StreamingContext context);

        // System::IEquatable
        public:
            virtual bool Equals(MenuItemHelp^ other);

        // System::IComparable
        public:
            virtual int CompareTo(MenuItemHelp^ other);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~MenuItemHelp();

        private:
            static System::Windows::Forms::ToolStripMenuItem^ FindToolStripMenuItem(System::String^ name);
            static System::Windows::Forms::ToolStripMenuItem^ FindToolStripMenuItem(System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection,
                                                                                    System::String^ name);

        private:
            System::String^ m_sMenuItemName;
            System::String^ m_sHelpText;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
