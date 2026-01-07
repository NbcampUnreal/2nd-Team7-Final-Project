#include "Character/Component/CharacterADSComponent.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsAnimationInstance.h"

#include "LastCanary.h"
UCharacterADSComponent::UCharacterADSComponent()
{
    PrimaryComponentTick.TickGroup = TG_DuringPhysics;

}

void UCharacterADSComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCharacterADSComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    /*

    float TargetWeight = bIsADS ? 1.0f : 0.0f;

    float NewWeight = FMath::FInterpTo(Get_ADS_Weight(), TargetWeight, DeltaTime, ADS_InterpSpeed);
    */
    //Set_ADS_Weight(NewWeight);
}

UAlsAnimationInstance* UCharacterADSComponent::GetALSAnimInstance()
{
    if(!IsValid(GetBaseCharacter()))
    {
        return nullptr;
    }

    if (!IsValid(GetCharacterAnimInstance()))
    {
        return nullptr;
    }
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    if (!IsValid(ALSAnimInstance))
    {
        return nullptr;
    }
    return ALSAnimInstance;
}

void UCharacterADSComponent::Set_ADS_Weight(float _ADS_Weight)
{
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    if (!IsValid(ALSAnimInstance))
    {
        return;
    }

    ADS_Weight = FMath::Clamp(_ADS_Weight, 0.0f, 1.0f);
    ALSAnimInstance->AimAlpha = ADS_Weight;
    
    GetBaseCharacter()->FPSCamera->FieldOfView = 90.0f - ADS_Weight * GetADS_CameraFieldOfView(GetGun());
}

AGunBase* UCharacterADSComponent::GetGun()
{
    if (IsValid(GetBaseCharacter()))
    {
        return GetBaseCharacter()->GetCurrentGunItem();
    }
    return nullptr;
}

float UCharacterADSComponent::GetADS_CameraFieldOfView(AGunBase* gun)
{
    if (!IsValid(gun))
    {
        return 30.0f;
    }

    if (gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
    {
        return 50.0f;
    }
    else if (gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")))
    {
        return 30.0f;
    }
    else if (gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
    {
        return 40.0f;
    }

    return 10.0f;
}
