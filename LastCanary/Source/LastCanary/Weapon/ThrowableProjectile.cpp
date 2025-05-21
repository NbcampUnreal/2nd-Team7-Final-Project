// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/ThrowableProjectile.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AThrowableProjectile::AThrowableProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	ThrowableWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThrowableWeaponMesh"));
	SetRootComponent(ThrowableWeaponMesh);

	DamageCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ShpereCollision"));
	DamageCollision->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
	ProjectileMovement->Friction = 0.2f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->SetUpdatedComponent(ThrowableWeaponMesh);


	USkeletalMesh* ThrowableWeapon = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Weapons/ThrowableWeapon/Grenade/SK_G67_X.SK_G67_X"));
	if (ThrowableWeapon)
	{
		ThrowableWeaponMesh->SetSkeletalMesh(ThrowableWeapon);
		ThrowableWeaponMesh->SetCollisionProfileName("BlockAll");
		ThrowableWeaponMesh->SetSimulatePhysics(false);
	}

	DamageRange = 500.0f;
	TimeForExplosion = 7.0f;

	DamageCollision->InitSphereRadius(DamageRange);
}

void AThrowableProjectile::ExplosionCount()	// 폭발 카운트 시작
{
	if (ThrowableWeaponMesh)
	{
		if (ReadyToAttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReadyToAttackSound, GetActorLocation()); // 핀을 뽑거나 불을 붙이거나
		}

		GetWorldTimerManager().SetTimer(WeaponTimerHandle, this, &AThrowableProjectile::Explode, TimeForExplosion, false);
	}
}

void AThrowableProjectile::Explode()	// 폭발
{
	if (GetWorldTimerManager().IsTimerActive(WeaponTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(WeaponTimerHandle);
	}

	ProjectileMovement->StopMovementImmediately();

	if (ExplosionNiagara)
	{
		UE_LOG(LogTemp, Warning, TEXT("Exist Niagara"));
		UNiagaraComponent* Explosion = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), 
			ExplosionNiagara, 
			GetActorLocation(), 
			GetActorRotation(), 
			FVector(1.0f), true);

	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}

	TArray < AActor*> OverlappingActors;
	DamageCollision->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Pawn"))
		{
			UGameplayStatics::ApplyDamage(Actor, Damage, nullptr, this, UDamageType::StaticClass());
		}
	}

	this->Destroy();
}

void AThrowableProjectile::SpawnSetting()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
	}
}

void AThrowableProjectile::Thrown(float ThrowForce)
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{			
		ProjectileMovement->Activate();
		ProjectileMovement->InitialSpeed = ThrowForce;
		ProjectileMovement->MaxSpeed = ThrowForce;

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		FRotator PlayerViewRotation = PlayerController->GetControlRotation();
		FVector PlayerViewVector = PlayerViewRotation.Vector();
		ProjectileMovement->Velocity = PlayerViewVector * ThrowForce;
	}

	if (!GetWorldTimerManager().IsTimerActive(WeaponTimerHandle))
	{
		ExplosionCount();
	}
}