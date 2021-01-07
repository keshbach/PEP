/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ToolbarImageKey.
        /// </summary>

        public ref class ToolbarImageKey sealed
        {
        public:
            static property System::String^ Cut
            {
                System::String^ get()
                {
                    return ImageManager::GenerateToolbarImageKey(UiUtilNet::Resources::Resource::ResourceManager, "Cut_16x");
                }
            }

            static property System::String^ Copy
            {
                System::String^ get()
                {
                    return ImageManager::GenerateToolbarImageKey(UiUtilNet::Resources::Resource::ResourceManager, "Copy_16x");
                }
            }

            static property System::String^ Paste
            {
                System::String^ get()
                {
                    return ImageManager::GenerateToolbarImageKey(UiUtilNet::Resources::Resource::ResourceManager, "Paste_16x");
                }
            }

            static property System::String^ Undo
            {
                System::String^ get()
                {
                    return ImageManager::GenerateToolbarImageKey(UiUtilNet::Resources::Resource::ResourceManager, "Undo_16x");
                }
            }

            static property System::String^ Delete
            {
                System::String^ get()
                {
                    return ImageManager::GenerateToolbarImageKey(UiUtilNet::Resources::Resource::ResourceManager, "Delete_16x");
                }
            }

            static property System::String^ Select
            {
                System::String^ get()
                {
                    return ImageManager::GenerateToolbarImageKey(UiUtilNet::Resources::Resource::ResourceManager, "Select_16x");
                }
            }
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
