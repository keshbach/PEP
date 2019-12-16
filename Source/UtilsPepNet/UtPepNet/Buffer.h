/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public ref class Buffer sealed
		{
		public:
			enum class EBitStuck
			{
                High0,
                High1,
                High2,
                High3,
                High4,
                High5,
                High6,
                High7,
				Low0,
				Low1,
				Low2,
				Low3,
				Low4,
				Low5,
				Low6,
				Low7
			};

		public:
			static System::Boolean IsEmpty(array<System::Byte>^ byData, System::Boolean% bIsEmpty);

			static System::Boolean CheckStuckBits(array<System::Byte>^ byData, System::Collections::Generic::List<EBitStuck>^% BitsStuckList);

			static System::Boolean IsOverdump(array<System::Byte>^ byData, System::Boolean% bIsOverdump);
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
