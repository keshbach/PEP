/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        public ref class MenuItemHelpListTypeEditor : System::Drawing::Design::UITypeEditor
        {
        public:
            MenuItemHelpListTypeEditor()
            {
            }

            System::Drawing::Design::UITypeEditorEditStyle GetEditStyle(System::ComponentModel::ITypeDescriptorContext^ context) override
            {
				context;

                return System::Drawing::Design::UITypeEditorEditStyle::Modal;
            }
        };

        public ref class MenuItemHelpListConverter : System::ComponentModel::CollectionConverter
        {
        public:
            System::Collections::ICollection^ GetStandardValues() new
            {
                return nullptr;
            }

            System::ComponentModel::TypeConverter::StandardValuesCollection^ GetStandardValues(System::ComponentModel::ITypeDescriptorContext^ context) override
            {
				context;

                return nullptr;
            }

            bool GetStandardValuesSupported() new
            {
                return true;
            }

            bool GetStandardValuesSupported(System::ComponentModel::ITypeDescriptorContext^ context) override
            {
				context;

                return true;
            }

            bool IsValid(System::ComponentModel::ITypeDescriptorContext^ context, System::Object^ value) override
            {
				context;
				value;

                return true;
            }

            bool CanConvertTo(System::ComponentModel::ITypeDescriptorContext^ context, System::Type^ destType) override
            {
                if (destType == System::ComponentModel::Design::Serialization::InstanceDescriptor::typeid)
                {
                    return true;
                }

                return System::ComponentModel::CollectionConverter::CanConvertTo(context, destType);
            }

            bool CanConvertFrom(System::ComponentModel::ITypeDescriptorContext^ context, System::Type^ sourceType) override
            {
                if (sourceType == System::String::typeid ||
                    System::ComponentModel::TypeConverter::CanConvertFrom(context, sourceType))
                {
                    return true;
                }

                return false;
            }

            System::Object^ ConvertFrom(System::ComponentModel::ITypeDescriptorContext^ context, System::Globalization::CultureInfo^ culture, System::Object^ value) override
            {
                System::String^ stringValue = dynamic_cast<System::String^>(value);

				context; 
				culture;

                if (stringValue != nullptr)
                {
                }

                return nullptr;
            }

            System::Object^ ConvertTo(System::Object^ value, System::Type^ destType) new
            {
				value;
				destType;

                return nullptr;
            }

            System::Object^ ConvertTo(System::ComponentModel::ITypeDescriptorContext^ context, System::Globalization::CultureInfo^ culture, System::Object^ value, System::Type^ destType) override
            {
				context;
				culture;
				value;

                //if (value == MenuItemHelpList::typeid)
                {
                    if (destType == System::String::typeid)
                    {
                        return gcnew System::String(L"(Collection)");
                    }
                }

                return nullptr;
            }
        };

        /// <summary>
        /// Summary description for MenuItemHelp.
        /// </summary>

        [System::Serializable
            , System::ComponentModel::TypeConverterAttribute(MenuItemHelpListConverter::typeid)
            , System::ComponentModel::EditorAttribute(MenuItemHelpListTypeEditor::typeid,
                                                      System::Drawing::Design::UITypeEditor::typeid)
        ]
        public ref class MenuItemHelpList : System::Collections::Generic::ICollection<MenuItemHelp^>,
                                            System::Collections::Generic::IList<MenuItemHelp^>,
                                            System::Runtime::Serialization::ISerializable
        {
        public:
            MenuItemHelpList();
            MenuItemHelpList(System::Runtime::Serialization::SerializationInfo^ info,
                             System::Runtime::Serialization::StreamingContext context);

        // System.Collections.Generic.ICollection
        public:
            property int Count
            {
                virtual int get()
                {
                    return m_MenuItemHelpList->Count;
                }
            }

            property bool IsReadOnly
            {
                virtual bool get()
                {
                    return false;
                }
            }

            virtual void Add(MenuItemHelp^ item);
            virtual void Clear();
            virtual bool Contains(MenuItemHelp^ item);
            virtual void CopyTo(array<MenuItemHelp^>^ array, int arrayIndex);
            virtual bool Remove(MenuItemHelp^ item);

        // System.Collections.Generic.IEnumerable
        public:
            virtual System::Collections::IEnumerator^ GetEnumerator();
            virtual System::Collections::Generic::IEnumerator<MenuItemHelp^>^ GetEnumeratorGeneric() = System::Collections::Generic::IEnumerable<MenuItemHelp^>::GetEnumerator;

        // System::Collections::Generic::IList
        public:
            property MenuItemHelp^ default[int]
            {
                virtual MenuItemHelp^ get(int index)
                {
                    return m_MenuItemHelpList[index];
                }
                virtual void set(int index, MenuItemHelp^ value)
                {
                    if (!m_MenuItemHelpList->Contains(value))
                    {
                        m_MenuItemHelpList[index] = value;
                    }
                }
            }

            virtual int IndexOf(MenuItemHelp^ item);
            virtual void Insert(int index, MenuItemHelp^ item);
            virtual void RemoveAt(int index);

        // System::Runtime::Serialization::ISerializable
        public:
            virtual void GetObjectData(System::Runtime::Serialization::SerializationInfo^ info,
                                       System::Runtime::Serialization::StreamingContext context);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~MenuItemHelpList();

        private:
            System::Collections::Generic::List<MenuItemHelp^>^ m_MenuItemHelpList;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
