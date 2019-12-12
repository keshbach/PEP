/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include "Buffer.h"

#include <UtilsPep/UtPepAnalyze.h>

System::Boolean Pep::Programmer::Buffer::IsEmpty(
  array<System::Byte>^ byData,
  System::Boolean^% bIsEmpty)
{
	pin_ptr<System::Byte> pbyData = &byData[0];
	BOOL bTmpIsEmpty;

	if (UtPepAnalyzeIsEmpty(pbyData, byData->Length, &bTmpIsEmpty))
	{
		bIsEmpty = bTmpIsEmpty ? true : false;

		return true;
	}

	return false;
}

System::Boolean Pep::Programmer::Buffer::CheckStuckBits(
  array<System::Byte>^ byData,
  System::Collections::Generic::List<EBitStuck>^% BitsStuckList)
{
	pin_ptr<System::Byte> pbyData = &byData[0];
	UINT32 nBitsStuck;

	BitsStuckList->Clear();

	if (FALSE == UtPepAnalyzeCheckStuckBits(pbyData, byData->Length, &nBitsStuck))
	{
		return false;
	}

	// Translate high stuck bits

	if (nBitsStuck & CPepAnalyzeBitStuckHigh0)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::High0);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckHigh1)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::High1);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckHigh2)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::High2);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckHigh3)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::High3);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckHigh4)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::High4);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckHigh5)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::High5);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckHigh6)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::High6);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckHigh7)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::High7);
	}

	// Translate low stuck bits

	if (nBitsStuck & CPepAnalyzeBitStuckLow0)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::Low0);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckLow1)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::Low1);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckLow2)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::Low2);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckLow3)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::Low3);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckLow4)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::Low4);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckLow5)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::Low5);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckLow6)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::Low6);
	}

	if (nBitsStuck & CPepAnalyzeBitStuckLow7)
	{
		BitsStuckList->Add(Pep::Programmer::Buffer::EBitStuck::Low7);
	}

	return true;
}

System::Boolean Pep::Programmer::Buffer::IsOverdump(
  array<System::Byte>^ byData,
  System::Boolean^% bIsOverdump)
{
	pin_ptr<System::Byte> pbyData = &byData[0];
	BOOL bTmpIsOverdump;

	if (TRUE == UtPepAnalyzeIsOverdump(pbyData, byData->Length, &bTmpIsOverdump))
	{
		bIsOverdump = bTmpIsOverdump ? true : false;

		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
