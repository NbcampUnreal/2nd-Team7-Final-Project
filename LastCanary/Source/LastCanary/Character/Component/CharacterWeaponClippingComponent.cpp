#include "Character/Component/CharacterWeaponClippingComponent.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsAnimationInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Character/Component/CharacterADSComponent.h"

#include "LastCanary.h"

UCharacterWeaponClippingComponent::UCharacterWeaponClippingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCharacterWeaponClippingComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCharacterWeaponClippingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateGunWallClipOffset();
}

AGunBase* UCharacterWeaponClippingComponent::GetGun()
{
	if (IsValid(GetBaseCharacter()))
	{
		return GetBaseCharacter()->GetCurrentGunItem();
	}
	return nullptr;
}

USkeletalMeshComponent* UCharacterWeaponClippingComponent::GetGunSkeletalMesh()
{
	if (IsValid(GetGun()))
	{
		USkeletalMeshComponent* RifleMesh = GetGun()->GetSkeletalMeshComponent();
		return RifleMesh;
	}
	return nullptr;
}

void UCharacterWeaponClippingComponent::UpdateGunWallClipOffset()
{
	if (!IsValid(GetBaseCharacter()) || !IsValid(GetBaseCharacter()->GetMesh()) || !IsValid(GetBaseCharacter()->GetMesh()->GetAnimInstance()))
	{
		return;
	}
	UAlsAnimationInstance* AlsAnim = Cast<UAlsAnimationInstance>(GetBaseCharacter()->GetMesh()->GetAnimInstance());
	if (!IsValid(AlsAnim))
	{
		return;
	}

	if (!IsValid(GetGun()) || !IsValid(GetGunSkeletalMesh()))
	{
		WallClipAimOffsetPitch = 0.0f;
		AlsAnim->WallClipAimOffsetPitch = WallClipAimOffsetPitch;
		return;
	}

	FHitResult HitResult;

	FVector TraceStart = GetBaseCharacter()->FPSCamera->GetComponentLocation();
	FVector TraceEnd = TraceStart + GetBaseCharacter()->FPSCamera->GetForwardVector() * 150.0f;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	TArray<AActor*> IgnoreActors = { GetBaseCharacter(), GetGun() };

	// SphereTrace로 넒은 범위 확인하기
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		TraceStart,
		TraceEnd,
		TraceRadius, // 기존 변수 활용
		TraceType,
		false,
		IgnoreActors,
		EDrawDebugTrace::ForOneFrame,
		HitResult,
		true
	);

	float LerpSpeed = (1.0f / GetWorld()->GetDeltaSeconds()) / 10.0f;

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor))
		{
			//LOG_Char_WARNING(TEXT("Camera Trace hit: %s"), *HitActor->GetName());
		}

		FVector TargetLocation = HandClipLocation + HandClipLocationOffset;

		// 위치 클램핑 (벽 밀착 시 튀지 않도록 보정)
		FVector ClampedLocation = FVector(
			FMath::Clamp(TargetLocation.X, 0.0f, 25.0f),
			0.0f, // Y는 고정
			FMath::Clamp(TargetLocation.Z, -30.0f, 0.0f)
		);

		TargetLocation = FMath::VInterpTo(TargetLocation, ClampedLocation, GetWorld()->GetDeltaSeconds(), LerpSpeed);

		// 회전 클램핑
		float ClampedTargetRotationRoll = FMath::Clamp(HandClipRotation.Roll + 4.0f, 0.0f, 50.0f);
		FRotator ClampedRotator(HandClipRotation.Pitch, HandClipRotation.Yaw, ClampedTargetRotationRoll);

		HandClipRotation = FMath::RInterpTo(HandClipRotation, ClampedRotator, GetWorld()->GetDeltaSeconds(), LerpSpeed / 2);

		// 카메라 -> 벽까지 남은 거리 기반으로 Pitch 오프셋 계산
		float DistanceToWall = (HitResult.ImpactPoint - TraceStart).Size();
		float TargetPitchOffset = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 100.0f), FVector2D(90.0f, 0.0f), DistanceToWall);

		WallClipAimOffsetPitch = FMath::FInterpTo(
			WallClipAimOffsetPitch,
			TargetPitchOffset,
			GetWorld()->GetDeltaSeconds(),
			LerpSpeed / 2
		);

		if (FMath::Abs(WallClipAimOffsetPitch) > 0.5f)
		{
			GetBaseCharacter()->SetIsCloseToWall(true);
			GetBaseCharacter()->SwitchADS(false);
		}
		else
		{
			GetBaseCharacter()->SetIsCloseToWall(false);
		}
	}
	else
	{
		WallClipAimOffsetPitch = FMath::FInterpTo(WallClipAimOffsetPitch, 0.0f, GetWorld()->GetDeltaSeconds(), LerpSpeed / 2);
	}

	AlsAnim->WallClipAimOffsetPitch = WallClipAimOffsetPitch;
	AlsAnim->WallClipAimOffsetLocation = HandClipLocation;
	AlsAnim->WallClipAimOffsetRotation = HandClipRotation;
}
