// Copyright Recursoft LLC 2019-2020. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"


class SMSYSTEMEDITOR_API FSMEditorStyle
{
public:
	// Register with the system.
	static void Initialize();

	// Unregister from the system.
	static void Shutdown();

	/** Gets the singleton instance. */
	static TSharedPtr<ISlateStyle> Get() { return StyleSetInstance; }

	static FName GetStyleSetName() { return TEXT("SMEditorStyle"); }

protected:
	static void SetGraphStyles();
	static void SetIcons();

	static FString InResources(const FString& RelativePath, const ANSICHAR* Extension);

	static FTextBlockStyle NormalText;
private:
	// Singleton instance.
	static TSharedPtr<FSlateStyleSet> StyleSetInstance;

};
