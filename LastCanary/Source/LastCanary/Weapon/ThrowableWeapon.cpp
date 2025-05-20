// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/ThrowableWeapon.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Weapon/ThrowableProjectile.h"
#include "Kismet/GameplayStatics.h"
//#include "Managers/UIManager.h"
//#include "UI/InGame.h"

AThrowableWeapon::AThrowableWeapon()
{
	LocationCheck = CreateDefaultSubobject<USphereComponent>(TEXT("LocationCheck"));
	LocationCheck->SetupAttachment(RootComponent);
	LocationCheck->InitSphereRadius(10.0f);

	WeaponMesh->SetSkeletalMesh(nullptr);

	ThrowableProjectile = nullptr;
	ThrowableProjectileClass = nullptr;
	ThrowForce = 1200.0f;
	CurrentQuantity = 0;
	MaxQuantity = 10;
	bIsInHand = false;
}

int32 AThrowableWeapon::GetCurrentQuantity() const
{
	return CurrentQuantity;
}

int32 AThrowableWeapon::GetMaxQuantity() const
{
	return MaxQuantity;
}

void AThrowableWeapon::AddQuantity(int32 AddNumber)
{
	CurrentQuantity = FMath::Min(CurrentQuantity + AddNumber, MaxQuantity);
	UpdateWeaponUI();
}

void AThrowableWeapon::Spawn()
{
	if (!bIsInHand && CurrentQuantity > 0)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		ThrowableProjectile = GetWorld()->SpawnActor<AThrowableProjectile>(ThrowableProjectileClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		if (ThrowableProjectile)
		{
			if (ThrowableProjectile->AttachToComponent(this->RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale))
			{
				ThrowableProjectile->SpawnSetting();
				CurrentQuantity--;
				bIsInHand = true;
				UpdateWeaponUI();
			}
		}
	}
}

void AThrowableWeapon::ReadyToExplosion()
{
	if (ThrowableProjectile)
	{
		ThrowableProjectile->ExplosionCount();
	}
}

void AThrowableWeapon::Throw()
{
	if (ThrowableProjectile)
	{
		ThrowableProjectile->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ThrowableProjectile->Thrown(ThrowForce);
		bIsInHand = false;
		UpdateWeaponUI();
	}
}

void AThrowableWeapon::UpdateWeaponUI()
{
	Super::UpdateWeaponUI();
	//if (UIManager)
	//{
	//	UUserWidget* InGameWidgetInstance = UIManager->WidgetInstances.FindRef(EHUDState::InGameBase);
	//	if (UInGame* InGameWidget = Cast<UInGame>(InGameWidgetInstance))
	//	{
	//		InGameWidget->UpdateAmmo(bIsInHand, CurrentQuantity);
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("InGame widget not found in UIManager"));
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("UIManager is null in AFirearm"));
	//}
}