/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public ref class PinConfig sealed
		{
		public:
			property System::UInt32 Pin
			{
				System::UInt32 get()
				{
					return m_nPin;
				}
			}

			property System::String^ Type
			{
				System::String^ get()
				{
					return m_sType;
				}
			}

			property System::String^ Polarity
			{
				System::String^ get()
				{
					return m_sPolarity;
				}
			}

		public:
			PinConfig(System::UInt32 nPin, System::String^ sType, System::String^ sPolarity);
			~PinConfig();

		private:
			System::UInt32 m_nPin;
			System::String^ m_sType;
			System::String^ m_sPolarity;

		private:
			PinConfig();
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
