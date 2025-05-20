// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/Shotgun.h"
#include "Weapon/Parts.h"
#include "Kismet/GameplayStatics.h"

AShotgun::AShotgun()
{
	this->Damage = 20.0f;
	this->AttackRate = 0.8f;
	this->ReloadTime = 4.0f;
	this->WeaponType = "Shotgun";
}

void AShotgun::Fire()
{
	int32 bulletCount = 8;
	float BaseSpreadAngle = 10.0f;
	float MaxRange = 1000.0f; // 10M
	float MinDamage = 5.0f;
	float MaxDamage = Damage;

	for (int i = 1; i <= bulletCount; i++)
	{
		UE_LOG(LogTemp, Warning, (TEXT("Bullet firing")))
		float DistanceFacter = float(i) / float(bulletCount);
		float SpreadAngle = BaseSpreadAngle + DistanceFacter * 10.0f;

		FRotator ShotDirection = WeaponMesh->GetSocketRotation("MuzzleSocket");
		ShotDirection.Yaw += FMath::RandRange(-SpreadAngle, SpreadAngle);
		ShotDirection.Pitch += FMath::RandRange(-SpreadAngle, SpreadAngle);

		FVector Start = WeaponMesh->GetSocketLocation("MuzzleSocket");
		FVector End = Start + ShotDirection.Vector() * MaxRange;

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Pawn, Params))
		{
			float Distance = (HitResult.Location - Start).Size();
					
			if (AActor* HitActor = HitResult.GetActor())
			{
				float DamagePerDistance = FMath::Lerp(MaxDamage, MinDamage, Distance / MaxRange);
				DealDamage(HitActor);
			}
			
		}
	}
}

void AShotgun::EquipParts(AParts* Parts)
{
	if (Parts->ActorHasTag("Shotgun"))
	{
		Super::EquipParts(Parts);
	}
}