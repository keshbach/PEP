/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace ListPorts
    {
        public ref class Port sealed
        {
        private:
            System::String^ m_sLocation;
            System::String^ m_sPhysicalDeviceObjectName;

        public:
            /// <summary>
            /// Location of the device.
            /// </summary>

            property System::String^ Location
            {
                System::String^ get()
                {
                    return m_sLocation;
                }
            }

            /// <summary>
            /// The device's physical object name.
            /// </summary>

            property System::String^ PhysicalDeviceObjectName
            {
                System::String^ get()
                {
                    return m_sPhysicalDeviceObjectName;
                }
            }

        internal:
            Port(System::String^ sLocation, System::String^ sPhysicalDeviceObjectName);

        private:
            Port();
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
