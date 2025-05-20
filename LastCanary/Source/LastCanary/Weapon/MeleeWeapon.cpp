// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/MeleeWeapon.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
//#include "Managers/UIManager.h"
//#include "UI/InGame.h"

AMeleeWeapon::AMeleeWeapon()
{
	USkeletalMesh* MeleeWeapon = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Weapons/MeleeWeapon/Knife/SK_M9_Knife_X.SK_M9_Knife_X"));
	if (MeleeWeapon)
	{
		WeaponMesh->SetSkeletalMesh(MeleeWeapon);
	}

}

void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>())
	{
		SkeletalMesh->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeapon::OnEnemyOverlap);
	}
}


void AMeleeWeapon::Attack()
{
	Super::Attack();
}

void AMeleeWeapon::OnEnemyOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyindex,
	bool bFromSweep,
	const FHitResult& Sweep)
{
	if (OtherActor && OtherActor->ActorHasTag("Enemy"))
	{
		DealDamage(OtherActor);

		if (HitEffect)
		{
			UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), HitEffect, Sweep.Location, FRotator::ZeroRotator);
		}

		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Sweep.Location);
		}
	}
}

void AMeleeWeapon::UpdateWeaponUI()
{
	Super::UpdateWeaponUI();
	//if (UIManager)
	//{
	//	UUserWidget* InGameWidgetInstance = UIManager->WidgetInstances.FindRef(EHUDState::InGameBase);
	//	if (UInGame* InGameWidget = Cast<UInGame>(InGameWidgetInstance))
	//	{
	//		InGameWidget->UpdateAmmo(1, 0);
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