// Copyright © 2020 Justin Camden All Rights Reserved


#include "AsgardCameraFunctionLibrary.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"

float UAsgardCameraFunctionLibrary::GetAspectRatio(UCameraComponent* Camera)
{
    if (Camera)
    {
        FVector2D Size = FVector2D(0.0f, 0.0f);
        Camera->GetWorld()->GetGameViewport()->GetViewportSize(Size);
        return Size.X / Size.Y;
    }
    return 0.0f;
}


float UAsgardCameraFunctionLibrary::GetFrustumWidth(UCameraComponent* Camera, float Distance)
{
    if (Camera)
    {
        FMinimalViewInfo ViewInfo;
        Camera->GetCameraView(0, ViewInfo);
        const float FrustumAngle = ViewInfo.FOV;
        const float HozHalfAngleInRadians = FMath::DegreesToRadians(FrustumAngle * 0.5f);
        return Distance * FMath::Tan(HozHalfAngleInRadians) * 2.0f;
    }
    return 0.0f;
}


float UAsgardCameraFunctionLibrary::GetFrustumHeight(UCameraComponent* Camera, float Distance)
{
    if (Camera)
    {
        // Aspect ratio
        FVector2D Size = FVector2D(0.0f, 0.0f);
        Camera->GetWorld()->GetGameViewport()->GetViewportSize(Size);
        float AspectRatio = Size.X / Size.Y;

        // Width
        FMinimalViewInfo ViewInfo;
        Camera->GetCameraView(0, ViewInfo);
        const float FrustumAngle = ViewInfo.FOV;
        const float HozHalfAngleInRadians = FMath::DegreesToRadians(FrustumAngle * 0.5f);
        float Width = Distance * FMath::Tan(HozHalfAngleInRadians) * 2.0f;

        return Width / AspectRatio;
    }

    return 0.0f;
}

FVector2D UAsgardCameraFunctionLibrary::GetFrustumSize(UCameraComponent* Camera, float Distance)
{
    if (Camera)
    {
        // Aspect ratio
        FVector2D Size = FVector2D(0.0f, 0.0f);
        Camera->GetWorld()->GetGameViewport()->GetViewportSize(Size);
        float AspectRatio = Size.X / Size.Y;

        // Width
        FMinimalViewInfo ViewInfo;
        Camera->GetCameraView(0, ViewInfo);
        const float FrustumAngle = ViewInfo.FOV;
        const float HozHalfAngleInRadians = FMath::DegreesToRadians(FrustumAngle * 0.5f);
        float Width = Distance * FMath::Tan(HozHalfAngleInRadians) * 2.0f;

        // Height
        float Height = Width / AspectRatio;

        return FVector2D(Width, Height);
    }
    return FVector2D();
}
