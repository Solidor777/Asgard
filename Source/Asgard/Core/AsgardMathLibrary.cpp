// Copyright © 2020 Justin Camden All Rights Reserved


#include "AsgardMathLibrary.h"


float UAsgardMathLibrary::AngleBetweenVectorsRadians(const FVector& ANormalized, const FVector& BNormalized)
{
	return FMath::Acos(ANormalized | BNormalized);
}

float UAsgardMathLibrary::AngleBetweenVectors(const FVector& ANormalized, const FVector& BNormalized)
{
	return FMath::RadiansToDegrees(FMath::Acos(ANormalized | BNormalized));
}
