// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/Bullet.h"
#include "Weapon/Firearm.h"
//#include "Monster/BaseMonster.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/DecalComponent.h"
#include "Sound/SoundAttenuation.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = false;

	bulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	SetRootComponent(bulletMesh);

	bulletMesh->SetCollisionProfileName("BlockAll");
	bulletMesh->BodyInstance.bUseCCD = true;

	projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	projectileMovement->InitialSpeed = 7000.0f;
	projectileMovement->MaxSpeed = 7000.0f;
	projectileMovement->bRotationFollowsVelocity = true;
	projectileMovement->bShouldBounce = false;
	projectileMovement->ProjectileGravityScale = 1.0f;

	bulletMesh->OnComponentHit.AddDynamic(this, &ABullet::onHit);

	AttenuationSetting = LoadObject<USoundAttenuation>(nullptr, TEXT("/Game/Weapons/Audio/SA_BodyShot.SA_BodyShot"));
}

void ABullet::ActivateBullet(FVector SpawnLocation, FRotator SpawnRotation, float Speed)
{
	SetActorLocation(SpawnLocation);
	SetActorRotation(SpawnRotation);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	if (projectileMovement)
	{
		projectileMovement->InitialSpeed = Speed;
		projectileMovement->MaxSpeed = Speed;
		projectileMovement->Velocity = SpawnRotation.Vector() * Speed;
		projectileMovement->SetUpdatedComponent(bulletMesh);
	}
}

void ABullet::SetFirearm(AFirearm* Firearm)
{
	if (Firearm)
	{
		gun = Firearm;
	}
}

void ABullet::SetBulletMesh(UStaticMesh* BulletMesh)
{
	if (BulletMesh)
	{
		bulletMesh->SetStaticMesh(BulletMesh);
	}
}

void ABullet::SetBulletSpeed(float Speed)
{
	projectileMovement->InitialSpeed = Speed;
	projectileMovement->MaxSpeed = Speed;
}

void ABullet::onHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (gun)
	{
		if (OtherActor != gun)
		{
			SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
		}

		if (OtherActor && OtherActor != this)
		{
			

			if (hitEffect)
			{
				if (OtherActor->ActorHasTag("Enemy"))
				{
					gun->DealDamage(OtherActor);

					UNiagaraComponent* Hit = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(),
						hitEffect,
						GetActorLocation(),
						GetActorRotation(),
						FVector(1.0f), true);
				}
				else if(hitStaticEffect)
				{
					UNiagaraComponent* Hit = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(),
						hitStaticEffect,
						GetActorLocation(),
						GetActorRotation(),
						FVector(1.0f), true);
				}

			}

			if (hitDecal)
			{
				//if (OtherActor->IsA<ABaseMonster>())
				//{
				//	USkeletalMeshComponent* EnemyMesh = Cast<USkeletalMeshComponent>(OtherComp);
				//	if (EnemyMesh)
				//	{
				//		if (DecalComp)
				//		{
				//			// 보류. 몬스터에서 작업 필요. BaseMonster->ApplyBulletDecal(Hit.Location, Hit.ImpactNormal.Rotation());
				//		}
				//	}
				//	//DecalComp = UGameplayStatics::SpawnDecalAttached(hitDecal, FVector(5.0f, 5.0f, 5.0f), OtherComp, NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), EAttachLocation::KeepRelativeOffset, 5.0f);
				//}
				
				//else
				//{
					DecalComp = UGameplayStatics::SpawnDecalAttached(hitDecal, FVector(3.0f, 3.0f, 3.0f), OtherComp, NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 5.0f);

					if (DecalComp)
					{
						DecalComp->SetFadeScreenSize(0.001f);
					}
				//}
			}

			if (hitSound && AttenuationSetting)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), hitSound, Hit.ImpactPoint, 1.0f, 1.0f, 0.0f, AttenuationSetting);
			}
		}

		gun->ReturnBulletToPool(this);
	}
}