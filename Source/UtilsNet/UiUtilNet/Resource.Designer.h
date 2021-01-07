/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace UiUtilNet
{
    namespace Resources {
        /// <summary>
        ///   A strongly-typed resource class, for looking up localized strings, etc.
        /// </summary>
        // This class was copied from a C# project since Managed C++ does not auto-generate this file
        [::System::CodeDom::Compiler::GeneratedCodeAttribute("System.Resources.Tools.StronglyTypedResourceBuilder", "16.0.0.0")]
        [::System::Diagnostics::DebuggerNonUserCodeAttribute()]
        [::System::Runtime::CompilerServices::CompilerGeneratedAttribute()]
        private ref class Resource sealed
        {
        private:
            static ::System::Resources::ResourceManager^ resourceMan;

            static ::System::Globalization::CultureInfo^ resourceCulture;

        internal:
            [::System::Diagnostics::CodeAnalysis::SuppressMessageAttribute("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
            Resource();

            /// <summary>
            ///   Returns the cached ResourceManager instance used by this class.
            /// </summary>
            [::System::ComponentModel::EditorBrowsableAttribute(::System::ComponentModel::EditorBrowsableState::Advanced)]
            static property ::System::Resources::ResourceManager^ ResourceManager
            {
                ::System::Resources::ResourceManager^ get()
                {
                    if (System::Object::ReferenceEquals(resourceMan, nullptr)) {
                        ::System::Resources::ResourceManager^ temp = gcnew ::System::Resources::ResourceManager("UiUtilNet.Resources", ::UiUtilNet::Resources::Resource::typeid->Assembly);
                        resourceMan = temp;
                    }
                    return resourceMan;
                }
            }

            /// <summary>
            ///   Overrides the current thread's CurrentUICulture property for all
            ///   resource lookups using this strongly typed resource class.
            /// </summary>
            [::System::ComponentModel::EditorBrowsableAttribute(::System::ComponentModel::EditorBrowsableState::Advanced)]
            static property ::System::Globalization::CultureInfo^ Culture
            {
                ::System::Globalization::CultureInfo^ get()
                {
                    return resourceCulture;
                }
                void set(::System::Globalization::CultureInfo^ value)
                {
                    resourceCulture = value;
                }
            }
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
