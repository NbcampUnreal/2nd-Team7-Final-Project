#include "Character/Component/CharacterADSComponent.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsAnimationInstance.h"

#include "LastCanary.h"
UCharacterADSComponent::UCharacterADSComponent()
{
    //PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    //PrimaryComponentTick.TickGroup = TG_PrePhysics;
    PrimaryComponentTick.TickGroup = TG_DuringPhysics;

}

void UCharacterADSComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCharacterADSComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    

    float TargetWeight = bIsADS ? 1.0f : 0.0f;
    float NewWeight = FMath::FInterpTo(Get_ADS_Weight(), TargetWeight, DeltaTime, ADS_InterpSpeed);
    Set_ADS_Weight(NewWeight);
    GetAimTargetLocation(GetADS_Distance(GetGun()));
    
    if (GetGun())
    {
        /*
        CalculateAimSocket();
        CalculateAimPoint();
        
        SetLeftHandIK();
        SetRightHandIK();

        SetGunItemSocketTransform();
        SetADSMode(Get_ADS_Weight());
        */
    }
}

void UCharacterADSComponent::SwitchADS(bool _bIsADS)
{
    bIsADS = _bIsADS;
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    ALSAnimInstance->IsAiming = bIsADS;
}

void UCharacterADSComponent::Set_ADS_Weight(float _ADS_Weight)
{
    ADS_Weight = FMath::Clamp(_ADS_Weight, 0.0f, 1.0f);

    //GetCharacter()->Camera->FieldOfView = 90.0f - ADS_Weight * 50.0f;
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    ALSAnimInstance->AimAlpha = ADS_Weight;

}

AGunBase* UCharacterADSComponent::GetGun()
{
    if (IsValid(GetCharacter()))
    {
        return GetCharacter()->GetCurrentGunItem();
    }
    return nullptr;
}

USkeletalMeshComponent* UCharacterADSComponent::GetGunSkeletalMesh()
{
    if (IsValid(GetGun()))
    {
        USkeletalMeshComponent* RifleMesh = GetGun()->GetSkeletalMeshComponent();
        return RifleMesh;
    }
    return nullptr;
}

FName UCharacterADSComponent::GetADSSocketName(AGunBase* gun)
{
    if (!IsValid(GetGun()))
    {
        return FName("");
    }
    

    AGunBase* Gun = GetGun();
    if (Gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
    {
        return FName("Rifle");
    }
    else if (Gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")))
    {
        return FName("Pistol");
    }
    else if (Gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
    {
        return FName("Shotgun");
    }
    
    return FName("");
}

float UCharacterADSComponent::GetADS_Distance(AGunBase* gun)
{
    if (!IsValid(GetGun()))
    {
        return 40.0f;
    }


    AGunBase* Gun = GetGun();
    if (Gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
    {
        return 30.0f;
    }
    else if (Gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")))
    {
        return 30.0f;
    }
    else if (Gun->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
    {
        return 30.0f;
    }

    return 10.0f;
}

void UCharacterADSComponent::SetADSMode(float _ADS_Weight)
{
    if (!IsValid(GetGun()) || !IsValid(GetGunSkeletalMesh()))
        return;

    USkeletalMeshComponent* GunMesh = GetGunSkeletalMesh();

    // --- 기준 Transform (손 소켓) ---
    FTransform RifleTransform = GetCharacter()->GetMesh()->GetSocketTransform(GetADSSocketName(GetGun()), RTS_World);

    // --- 총기 ADS Transform ---
    FTransform ScopeTransform = GunMesh->GetSocketTransform("ADS", RTS_World);

    // --- 위치 보간 ---
    FVector CameraLocation = GetCharacter()->Camera->GetComponentLocation();
    FVector CameraForward = GetCharacter()->Camera->GetForwardVector();

    FVector ADSLocation = CameraLocation + CameraForward * GetADS_Distance(GetGun())
        - (ScopeTransform.GetLocation() - GunMesh->GetComponentLocation());

    FVector TargetLocation = FMath::Lerp(RifleTransform.GetLocation(), ADSLocation, _ADS_Weight);

    // --- 회전 보간 ---
    FQuat RifleQuat = RifleTransform.GetRotation();

    // 카메라 Forward 기준 회전 생성
    FQuat CameraForwardQuat = FRotationMatrix::MakeFromX(CameraForward).ToQuat();

    // --- 메시 Forward 축 보정 ---
    // 메시 Forward가 X+, Y+, Z+ 중 무엇인지에 따라 보정 필요
    // 예시: 메시 Forward가 Y+라면 Z축 기준 90도 회전
    CameraForwardQuat *= FQuat(FVector(0.f, 0.f, 1.f), FMath::DegreesToRadians(-90.f));

    // 손 소켓 회전과 카메라 Forward 회전 보간
    FQuat TargetQuat = FQuat::Slerp(RifleQuat, CameraForwardQuat, _ADS_Weight);

    // --- 최종 적용 ---
    GunMesh->SetWorldLocationAndRotation(TargetLocation, TargetQuat);
    
    GetCharacter()->GetMesh()->RefreshBoneTransforms();

    /*
    // 최종 위치 적용 (Rifle 소켓 기준)
    
    //GunMesh->SetWorldTransform(FTransform(RifleTransform.GetRotation(), RifleTransform.GetLocation() + Offset));
    GunMesh->SetWorldLocation(RifleTransform.GetLocation() + Offset);
    GunMesh->SetRelativeRotation(GunMesh->GetComponentRotation());
    //

    FTransform RightHand = GetCharacter()->GetMesh()->GetSocketTransform(GetADSSocketName(GetGun()), RTS_World);

    FTransform RightHandGrip = GunMesh->GetSocketTransform("RightGrip", RTS_World);
    FVector RightHandGripLocation = RightHandGrip.GetLocation();
    FVector GripToScope = RightHandGripLocation + Offset;
    FVector RifleSocketLocation = GetCharacter()->GetMesh()->GetSocketLocation(GetADSSocketName(GetGun()));
    FVector RifleToRightGrip = RightHandGripLocation - RifleSocketLocation;

    FVector FinalGripLocation = GripToScope - RifleToRightGrip;

    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    
    FVector Direction = (ADSLocation - CameraLocation).GetSafeNormal();
    FRotator GripRotation = Direction.Rotation();
    FVector Offset2 = TargetLocation - ScopeTransform.GetLocation();


    

    ALSAnimInstance->RightHandLocationToScope = TargetLocation;// + ScopeTransform.GetLocation()// - RightHand.GetLocation();
    //ALSAnimInstance->RightHandLocationToScope = CameraLocation + CameraForward * GetADS_Distance(GetGun()) - ScopeTransform.GetLocation();
    ALSAnimInstance->RightHandRotationToScope = GripRotation;
    */
}

void UCharacterADSComponent::SetLeftHandIK()
{
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    
    USkeletalMeshComponent* GunMesh = GetGunSkeletalMesh();
    FTransform LeftHandGrip = GunMesh->GetSocketTransform("LeftGrip", RTS_World);
    FVector LeftHandGripLocation = LeftHandGrip.GetLocation();

    ALSAnimInstance->SetLeftHandVBLocation(LeftHandGripLocation);
}

void UCharacterADSComponent::SetRightHandIK()
{
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    
    USkeletalMeshComponent* GunMesh = GetGunSkeletalMesh();
    FTransform RightHandGrip = GunMesh->GetSocketTransform("RightGrip", RTS_World);
    FVector RightHandGripLocation = RightHandGrip.GetLocation();
    ALSAnimInstance->SetRightHandVBLocation(RightHandGripLocation);
}

void UCharacterADSComponent::SetikHandgunLocation()
{
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    GetCharacter()->GetMesh()->GetSocketTransform("hand_r",RTS_World);

}

void UCharacterADSComponent::CalculateAimSocket()
{
    FTransform GunScope_Transform = GetGunSkeletalMesh()->GetSocketTransform(TEXT("ADS"), ERelativeTransformSpace::RTS_World);
    FTransform ik_hand_gun_Transform = GetCharacter()->GetMesh()->GetSocketTransform(TEXT("ik_hand_gun"), ERelativeTransformSpace::RTS_World);
    //FTransform t = GunScope_Transform.GetRelativeTransform(ik_hand_gun_Transform);
    FTransform t = ik_hand_gun_Transform.GetRelativeTransform(GunScope_Transform);


    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());

    ALSAnimInstance->AimSocketLocation = GunScope_Transform.GetLocation();
    ALSAnimInstance->AimSocketRotation = t.GetRotation().Rotator();
}

void UCharacterADSComponent::CalculateAimPoint()
{
    FTransform Camera_Transform = GetCharacter()->Camera->GetComponentTransform();
    FTransform ik_hand_root_Transform = GetCharacter()->GetMesh()->GetSocketTransform(TEXT("ik_hand_root"), ERelativeTransformSpace::RTS_World);
    FTransform t = Camera_Transform.GetRelativeTransform(ik_hand_root_Transform);

    FVector CameraForwardVector = t.GetRotation().GetForwardVector();
    


    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());

    ALSAnimInstance->AimPointLocation = t.GetLocation() + CameraForwardVector * GetADS_Distance(GetGun());
    ALSAnimInstance->AimPointRotation = t.GetRotation().Rotator();
}

void UCharacterADSComponent::SetGunItemSocketTransform()
{
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    FTransform GunItemSocket = GetCharacter()->GetMesh()->GetSocketTransform(TEXT("Rifle"), ERelativeTransformSpace::RTS_World);
    ALSAnimInstance->GunItemSocket = GunItemSocket;
}


FVector UCharacterADSComponent::GetAimTargetLocation(float Distance)
{
    if (!GetCharacter()->Camera) return FVector::ZeroVector;

    FVector CameraLocation = GetCharacter()->Camera->GetComponentLocation();
    FVector CameraForward = GetCharacter()->Camera->GetForwardVector();

    FVector AimPoint = CameraLocation + CameraForward * Distance;
    UAlsAnimationInstance* ALSAnimInstance = Cast<UAlsAnimationInstance>(GetCharacterAnimInstance());
    ALSAnimInstance->SetAimPoint(AimPoint);

    if (GetGun())
    {
        USkeletalMeshComponent* GunMesh = GetGunSkeletalMesh();
        
        FTransform RightHandGrip = GunMesh->GetSocketTransform("RightGrip", RTS_World);
        //FTransform RightHandGrip = GunMesh->GetSocketTransform("RightGrip", RTS_Component);
        //FTransform RightHandGrip = GetCharacter()->GetMesh()->GetSocketTransform("Rifle", RTS_World);
        FTransform GunScope = GunMesh->GetSocketTransform("ADS", RTS_World);

        FVector OffsetLocation = RightHandGrip.GetLocation() - GunScope.GetLocation();
        FRotator OffsetRotation = RightHandGrip.GetRotation().Rotator() - GunScope.GetRotation().Rotator();

        ALSAnimInstance->RightHandIKTargetLocationOffset = OffsetLocation;
        ALSAnimInstance->RightHandIKTargetRotationOffset = OffsetRotation;
    }
    

    
    return AimPoint;
}