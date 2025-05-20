#include "Item/EquipmentItem/GunBase.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "LastCanary.h"

AGunBase::AGunBase()
{
}

void AGunBase::UseItem()
{
    if (Durability <= 0.0f || GetWorld()->GetTimeSeconds() - LastFireTime < FireRate)
    {
        if (Durability <= 0.0f && EmptySound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, EmptySound, GetActorLocation());
        }
        return;
    }

    LastFireTime = GetWorld()->GetTimeSeconds();

    Multicast_PlayFireEffects();

    if (GetLocalRole() < ROLE_Authority)
    {
        Server_Fire();
    }
    else
    {
        HandleFire();
    }

    OnItemStateChanged.Broadcast();
}

void AGunBase::Server_Fire_Implementation()
{
    HandleFire();
}

void AGunBase::HandleFire()
{
    Durability = FMath::Max(0.0f, Durability - 1.0f);
    UpdateAmmoState();

    for (int32 i = 0; i < BulletsPerShot; i++)
    {
        FHitResult HitResult;
        FVector StartLocation;
        FVector EndLocation;

        if (PerformLineTrace(HitResult, StartLocation, EndLocation))
        {
            ProcessHit(HitResult, StartLocation);
        }
    }
}

bool AGunBase::PerformLineTrace(FHitResult& OutHit, FVector& StartLocation, FVector& EndLocation)
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return false;

    ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor);
    if (!OwnerCharacter) return false;

    FVector CameraLocation;
    FRotator CameraRotation;
    OwnerCharacter->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector ShootDirection = CameraRotation.Vector();
    if (Spread > 0.0f)
    {
        ShootDirection = FMath::VRandCone(ShootDirection, FMath::DegreesToRadians(Spread));
    }

    StartLocation = MuzzleLocation ? MuzzleLocation->GetComponentLocation() : GetActorLocation();
    EndLocation = StartLocation + ShootDirection * FireRange;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(OwnerActor);
    QueryParams.bTraceComplex = true;

    return GetWorld()->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECC_Visibility, QueryParams);
}

void AGunBase::ProcessHit(const FHitResult& HitResult, const FVector& StartLocation)
{
    AActor* HitActor = HitResult.GetActor();
    if (HitActor && HitActor != this && HitActor != GetOwner())
    {
        FPointDamageEvent DamageEvent(BaseDamage, HitResult, (HitResult.ImpactPoint - StartLocation).GetSafeNormal(), nullptr);
        HitActor->TakeDamage(BaseDamage, DamageEvent, GetInstigatorController(), this);
    }

    Multicast_SpawnImpactEffects(HitResult);
}

void AGunBase::Multicast_SpawnImpactEffects_Implementation(const FHitResult& Hit)
{
    if (ImpactDecalMaterial)
    {
        UGameplayStatics::SpawnDecalAtLocation(
            GetWorld(),
            ImpactDecalMaterial,
            DecalSize,
            Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation(),
            DecalLifeSpan
        );
    }

    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.ImpactPoint);
    }
}

void AGunBase::Multicast_PlayFireEffects_Implementation()
{
    // TODO : 발사 이펙트 재생
}

void AGunBase::BeginPlay()
{
    Super::BeginPlay();

    if (Durability <= 0.0f)
    {
        Durability = MaxAmmo;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        LOG_Item_WARNING(TEXT("[GunBase::BeginPlay] World is null!"));
        return;
    }

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
		LOG_Item_WARNING(TEXT("[GunBase::BeginPlay] GameInstance is null!"));
        return;
    }

    ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GISubsystem)
    {
        LOG_Item_WARNING(TEXT("[GunBase::BeginPlay] GameInstanceSubsystem is null!"));
        return;
    }

    GunDataTable = GISubsystem->GunDataTable;


    ApplyGunDataFromDataTable();
    UpdateAmmoState();
}

void AGunBase::ApplyGunDataFromDataTable()
{
}

void AGunBase::UpdateAmmoState()
{
    CurrentAmmo = Durability;

    if (Durability <= 0.0f)
    {
        // TODO : 탄약 부족 UI 표시
    }
}

bool AGunBase::Reload(float AmmoAmount)
{
    if (FMath::IsNearlyEqual(Durability, MaxAmmo))
    {
        return false;
    }

    Durability = FMath::Min(MaxAmmo, Durability + AmmoAmount);

    UpdateAmmoState();
    OnItemStateChanged.Broadcast();

    return true;
}

void AGunBase::OnRepDurability()
{
    Super::OnRepDurability();

    UpdateAmmoState();
}