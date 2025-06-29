#include "Item/EquipmentItem/GunBase.h"
#include "Item/ItemBase.h"
#include "Item/ShellEjectionComponent.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Character/BaseCharacter.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "GameplayTagAssetInterface.h"
#include "LastCanary.h"

AGunBase::AGunBase()
{
    FireRange = 10000.0f;
    BaseDamage = 20.0f;
    FireRate = 0.2f;
    Spread = 2.0f;
    BulletsPerShot = 1;
    LastFireTime = 0.0f;
    DecalSize = FVector(5.0f, 5.0f, 5.0f);
    DecalLifeSpan = 10.0f;
    VerticalRecoilAmount = 2.0f;
    HorizontalRecoilAmount = 1.0f;
    MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
    MuzzleLocation->SetupAttachment(RootComponent);
    MuzzleLocation->SetRelativeLocation(FVector(0, 50, 10));

    ScopeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScopeComponent"));
    ScopeComponent->SetupAttachment(GetSkeletalMeshComponent());
    ScopeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ScopeComponent->SetVisibility(false);

    MagazineComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagazineComponent"));
    MagazineComponent->SetupAttachment(GetSkeletalMeshComponent());
    MagazineComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MagazineComponent->SetVisibility(false);

    ShellEjectionComponent = CreateDefaultSubobject<UShellEjectionComponent>(TEXT("ShellEjectionComponent"));

    SpotlightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotlightComponent"));
    SpotlightComponent->SetVisibility(true);
    SpotlightComponent->SetHiddenInGame(true);
    SpotlightComponent->SetCastShadows(true);
    SpotlightComponent->SetIntensity(10000.0f);
    SpotlightComponent->SetAttenuationRadius(1000.0f);
    SpotlightComponent->SetInnerConeAngle(20.0f);
    SpotlightComponent->SetOuterConeAngle(40.0f);
    SpotlightComponent->SetLightColor(FLinearColor::White);

    CurrentFireMode = EFireMode::Single;
    bCanToggleFireMode = false;
    bIsSpotlightActive = false;
}

void AGunBase::BeginPlay()
{
    Super::BeginPlay();

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

    // 게임인스턴스 서브시스템에서 데이터 테이블 참조 가져오기
    if (GISubsystem->GunDataTable)
    {
        GunDataTable = GISubsystem->GunDataTable;
    }
    else
    {
        LOG_Item_WARNING(TEXT("[GunBase::BeginPlay] 게임인스턴스 서브시스템의 GunDataTable이 null입니다!"));
    }

    if (Durability <= 0.0f && !bIsEquipped)
    {
        Durability = MaxDurability;
    }

    InitializeGameplayTags();
    EnsureGunDataLoaded();

    if (USkeletalMeshComponent* ActiveMesh = GetSkeletalMeshComponent())
    {
        UAnimInstance* AnimInstance = ActiveMesh->GetAnimInstance();
        if (AnimInstance)
        {
            LOG_Item_WARNING(TEXT("GunBase: AnimInstance found on SkeletalMeshComponent"));
        }
        else
        {
            LOG_Item_WARNING(TEXT("GunBase: No AnimInstance on SkeletalMeshComponent - animations will not work"));
        }
    }
    else
    {
        LOG_Item_WARNING(TEXT("GunBase: No SkeletalMeshComponent found"));
    }
}

void AGunBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 연발 중단
    if (bIsAutoFiring)
    {
        StopAutoFire();
    }

    // 모든 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
    }

    Super::EndPlay(EndPlayReason);
}

void AGunBase::UseItem()
{
    if (!CanFire())
    {
        return;
    }

    if (CurrentFireMode == EFireMode::Single)
    {
        // 단발 모드
        FireSingle();
    }
    else // EFireMode::FullAuto
    {
        // 연발 모드: 토글 방식
        if (bIsAutoFiring)
        {
            // 연발 중이면 중단
            StopAutoFire();
        }
        else
        {
            // 연발 중이 아니면 시작
            StartAutoFire();
        }
    }
}

void AGunBase::OnRepItemRowName()
{
    Super::OnRepItemRowName();

    if (!ItemRowName.IsNone())
    {
        EnsureGunDataLoaded();
    }
}

void AGunBase::Server_Fire_Implementation()
{
    EnsureGunDataLoaded();

    HandleFire();

    LOG_Item_WARNING(TEXT("[Server_Fire] RecentHits 개수: %d"), RecentHits.Num());

    for (int32 i = 0; i < RecentHits.Num(); i++)
    {
        LOG_Item_WARNING(TEXT("[Server_Fire] Hit %d: %s at %s"),
            i,
            RecentHits[i].GetActor() ? *RecentHits[i].GetActor()->GetName() : TEXT("None"),
            *RecentHits[i].ImpactPoint.ToString());
    }

    Multicast_PlayFireEffects();

    if (RecentHits.Num() > 0)
    {
        LOG_Item_WARNING(TEXT("[Server_Fire] 멀티캐스트 이펙트 호출 - 히트 개수: %d"), RecentHits.Num());
        Multicast_SpawnImpactEffects(RecentHits);
    }
    else
    {
        LOG_Item_WARNING(TEXT("[Server_Fire] 히트 없음 - 이펙트 스킵"));
    }

    Multicast_PlayFireAnimation();

    Client_PlayCameraShake();
}

void AGunBase::HandleFire()
{
    LOG_Item_WARNING(TEXT("[HandleFire] 시작 - 현재 탄환: %.1f"), Durability);

    float OldDurability = Durability;
    Durability = FMath::Max(0.0f, Durability - 1.0f);

    LOG_Item_WARNING(TEXT("[HandleFire] 탄환 감소: %.1f → %.1f"), OldDurability, Durability);

    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (UToolbarInventoryComponent* ToolbarComp = OwnerPawn->FindComponentByClass<UToolbarInventoryComponent>())
        {
            LOG_Item_WARNING(TEXT("[HandleFire] SyncGunStateToSlot 호출"));
            ToolbarComp->SyncGunStateToSlot();

            if (HasAuthority())
            {
                int32 CurrentAmmo = FMath::RoundToInt(Durability);
                int32 MaxAmmo = FMath::RoundToInt(MaxDurability);
                ToolbarComp->MulticastSetGunAmmoUIVisibility(true, CurrentAmmo, MaxAmmo, CurrentFireMode, AvailableFireModes);
            }
        }
        else
        {
            LOG_Item_WARNING(TEXT("[HandleFire] ToolbarInventoryComponent를 찾을 수 없음"));
        }
    }
    else
    {
        LOG_Item_WARNING(TEXT("[HandleFire] Owner Pawn을 찾을 수 없음"));
    }

    FVector SoundLocation = GetActorLocation();

    AActor* SoundCauser = this;

    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        SoundLocation,
        1.0f,
        SoundCauser,
        2000.f,
        "CaveMonster"
    );

    // 최근 히트 결과 초기화
    RecentHits.Empty();

    for (int32 i = 0; i < BulletsPerShot; i++)
    {
        FHitResult HitResult;
        FVector StartLocation;
        FVector EndLocation;

        if (PerformLineTrace(HitResult, StartLocation, EndLocation))
        {
            RecentHits.Add(HitResult);
            ProcessHit(HitResult, StartLocation);
        }
    }
}

bool AGunBase::PerformLineTrace(FHitResult& OutHit, FVector& StartLocation, FVector& EndLocation)
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        LOG_Item_WARNING(TEXT("PerformLineTrace: Owner is NULL"));
        return false;
    }

    ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor);
    if (!OwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("PerformLineTrace: Owner is not a BaseCharacter - Actual class: %s"),
            *OwnerActor->GetClass()->GetName());

        // 다른 캐스팅 시도
        APawn* OwnerPawn = Cast<APawn>(OwnerActor);
        if (OwnerPawn)
        {
            LOG_Item_WARNING(TEXT("PerformLineTrace: Owner is a Pawn, trying to get controller"));
            AController* Controller = OwnerPawn->GetController();
            if (Controller)
            {
                LOG_Item_WARNING(TEXT("PerformLineTrace: Controller found: %s"), *Controller->GetClass()->GetName());
            }
        }

        return false;
    }

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

    bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECC_Visibility, QueryParams);

    return bHit;
}

void AGunBase::ProcessHit(const FHitResult& HitResult, const FVector& StartLocation)
{
    AActor* HitActor = HitResult.GetActor();

    if (!IsValid(HitActor) || HitActor == this || HitActor == GetOwner())
        return;

    // 기믹 파괴 로직
    if (ALCBaseGimmick* Gimmick = Cast<ALCBaseGimmick>(HitActor))
    {
        if (Gimmick->bDestructibleByGun)
        {
            LOG_Item_WARNING(TEXT("🔫 기믹 피격됨 → 데미지 전달: %s"), *Gimmick->GetName());

            FPointDamageEvent DamageEvent(1.0f, HitResult, (HitResult.ImpactPoint - StartLocation).GetSafeNormal(), nullptr);
            Gimmick->TakeDamage(1.0f, DamageEvent, GetInstigatorController(), this);
            return; 
        }
    }

    //  적 공격 로직
    static const FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy"));
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(HitActor);
    if (TagInterface && TagInterface->HasMatchingGameplayTag(EnemyTag))
    {
        float AppliedDamage = BaseDamage;
        LOG_Item_WARNING(TEXT("ProcessHit: Applying %.1f damage to enemy %s"), AppliedDamage, *HitActor->GetName());

        FPointDamageEvent DamageEvent(AppliedDamage, HitResult, (HitResult.ImpactPoint - StartLocation).GetSafeNormal(), nullptr);
        HitActor->TakeDamage(AppliedDamage, DamageEvent, GetInstigatorController(), this);
    }
    else
    {
        LOG_Item_WARNING(TEXT("ProcessHit: Hit non-enemy actor %s. No damage applied"), *HitActor->GetName());
    }

    USoundBase* ImpactSoundToPlay = GetImpactSoundForTarget(HitActor);
    Multicast_PlayImpactSoundAtLocation(ImpactSoundToPlay, HitResult.ImpactPoint);
}


void AGunBase::Multicast_SpawnImpactEffects_Implementation(const TArray<FHitResult>& Hits)
{
    EnsureGunDataLoaded();

    for (int32 i = 0; i < Hits.Num(); i++)
    {
        const FHitResult& Hit = Hits[i];

        LOG_Item_WARNING(TEXT("[Client] Processing Hit %d: %s at %s"), i, Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"), *Hit.ImpactPoint.ToString());

        FVector EnhancedDecalSize = DecalSize * 5.0f;
        FVector AdjustedLocation = Hit.ImpactPoint + Hit.ImpactNormal * 0.5f;

        if (ImpactDecalMaterial)
        {
            UDecalComponent* SpawnedDecal = UGameplayStatics::SpawnDecalAtLocation(
                GetWorld(),
                ImpactDecalMaterial,
                EnhancedDecalSize,
                AdjustedLocation,
                Hit.ImpactNormal.Rotation(),
                DecalLifeSpan
            );
        }
        else
        {
            LOG_Item_WARNING(TEXT("[Client] ImpactDecalMaterial이 null입니다"));
        }

        if (GunData.ImpactNiagaraEffect)
        {
            FRotator NiagaraRot = Hit.ImpactNormal.Rotation();

            /*NiagaraRot.Pitch += 90.0f;*/

            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                GunData.ImpactNiagaraEffect,
                Hit.ImpactPoint,
                NiagaraRot
            );
        }

        if (GunData.BloodNiagaraEffect)
        {
            FRotator NiagaraRot = Hit.ImpactNormal.Rotation();

            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                GunData.BloodNiagaraEffect,
                Hit.ImpactPoint,
                NiagaraRot
            );
        }
    }
}

void AGunBase::Multicast_PlayFireEffects_Implementation()
{
    // 발사 사운드 재생
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }

    // 총구 화염 이펙트 재생 - 소켓 사용
    if (MuzzleFlash)
    {
        USkeletalMeshComponent* GunMesh = GetSkeletalMeshComponent();
        if (GunMesh && GunMesh->DoesSocketExist(TEXT("Muzzle")))
        {
            // 소켓에 직접 부착
            UGameplayStatics::SpawnEmitterAttached(
                MuzzleFlash,
                GunMesh,
                TEXT("Muzzle"),
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::SnapToTarget
            );

            LOG_Item_WARNING(TEXT("머즐 플래시 이펙트가 Muzzle 소켓에 생성됨"));
        }
        else
        {
            // 소켓이 없을 경우 기존 MuzzleLocation 사용
            if (MuzzleLocation)
            {
                UGameplayStatics::SpawnEmitterAttached(
                    MuzzleFlash,
                    MuzzleLocation,
                    NAME_None,
                    MuzzleLocation->GetComponentLocation(),
                    MuzzleLocation->GetComponentRotation(),
                    EAttachLocation::KeepWorldPosition
                );
                LOG_Item_WARNING(TEXT("머즐 플래시 이펙트가 MuzzleLocation에 생성됨 (소켓 없음)"));
            }
        }
    }

    // 탄피 배출
    if (ShellEjectionComponent)
    {
        ShellEjectionComponent->EjectShell();
    }

    // 플레이어 카메라 흔들림 등 추가 이펙트 (옵션)
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn && OwnerPawn->IsLocallyControlled())
    {
        // TODO : 카메라 흔들림 효과나 반동 애니메이션 등을 여기서 처리할 수 있음
    }
}

void AGunBase::Multicast_PlayReloadSound_Implementation()
{
    if (GunData.ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, GunData.ReloadSound, GetActorLocation());
    }
}

void AGunBase::Multicast_PlayFireModeSwitchSound_Implementation()
{
    if (FireModeSwitchSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireModeSwitchSound, GetActorLocation());
    }
    else if (GunData.FireModeSwitchSound)
    {
        // 로컬 변수가 없으면 데이터 테이블에서 직접 재생
        UGameplayStatics::PlaySoundAtLocation(this, GunData.FireModeSwitchSound, GetActorLocation());
    }
    else
    {
        LOG_Item_WARNING(TEXT("[Multicast_PlayFireModeSwitchSound] 발사 모드 교체 사운드가 설정되지 않음"));
    }
}

void AGunBase::Client_PlayCameraShake_Implementation()
{
    if (ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner()))
    {
        LOG_Item_WARNING(TEXT("Client_PlayCameraShake called"));
        //OwnerCharacter->ApplyRecoil(2.0f, 1.0);
        OwnerCharacter->ApplySmoothRecoil(VerticalRecoilAmount, HorizontalRecoilAmount);
    }
}

USoundBase* AGunBase::GetImpactSoundForTarget(AActor* HitActor)
{
    if (!HitActor)
    {
        return GunData.DefaultImpactSound;
    }

    // 피격당한 액터의 게임플레이 태그 인터페이스 확인
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(HitActor);
    if (!TagInterface)
    {
        LOG_Item_WARNING(TEXT("[GetImpactSoundForTarget] %s는 GameplayTagAssetInterface를 구현하지 않음"),
            *HitActor->GetName());
        return GunData.DefaultImpactSound;
    }

    // 액터의 모든 태그 가져오기
    FGameplayTagContainer ActorTags;
    TagInterface->GetOwnedGameplayTags(ActorTags);

    LOG_Item_WARNING(TEXT("[GetImpactSoundForTarget] %s의 태그: %s"),
        *HitActor->GetName(), *ActorTags.ToString());

    // 피격 사운드 매핑에서 일치하는 태그 찾기
    for (const FImpactSoundMapping& Mapping : GunData.ImpactSoundMappings)
    {
        if (ActorTags.HasTag(Mapping.TargetTag))
        {
            LOG_Item_WARNING(TEXT("[GetImpactSoundForTarget] 태그 매칭: %s -> %s"),
                *Mapping.TargetTag.ToString(),
                Mapping.ImpactSound ? *Mapping.ImpactSound->GetName() : TEXT("None"));
            return Mapping.ImpactSound;
        }
    }

    // 매칭되는 태그가 없으면 기본 사운드 사용
    LOG_Item_WARNING(TEXT("[GetImpactSoundForTarget] 매칭되는 태그 없음 - 기본 사운드 사용"));
    return GunData.DefaultImpactSound;
}

void AGunBase::Multicast_PlayImpactSoundAtLocation_Implementation(USoundBase* Sound, FVector Location)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
        LOG_Item_WARNING(TEXT("[Multicast_PlayImpactSoundAtLocation] 사운드 재생: %s"), *Sound->GetName());
    }
}

void AGunBase::Multicast_PlayFireAnimation_Implementation()
{
    EnsureGunDataLoaded();

    if (GunData.FireAnimation)
    {
        PlayGunAnimation(GunData.FireAnimation);
    }
}

void AGunBase::Multicast_PlayReloadAnimation_Implementation()
{
    if (GunData.ReloadAnimation)
    {
        PlayGunAnimation(GunData.ReloadAnimation, 1.0f);
        LOG_Item_WARNING(TEXT("Playing reload animation: %s"), *GunData.ReloadAnimation->GetName());
    }
    else
    {
        LOG_Item_WARNING(TEXT("No reload animation assigned"));
    }
}

void AGunBase::PlayGunAnimation(UAnimMontage* AnimMontage, float PlayRate)
{
    if (!AnimMontage)
    {
        LOG_Item_WARNING(TEXT("PlayGunAnimation: AnimMontage is null"));
        return;
    }

    USkeletalMeshComponent* ActiveMesh = GetSkeletalMeshComponent();
    if (!ActiveMesh)
    {
        LOG_Item_WARNING(TEXT("PlayGunAnimation: No SkeletalMeshComponent found"));
        return;
    }

    UAnimInstance* AnimInstance = ActiveMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        LOG_Item_WARNING(TEXT("PlayGunAnimation: No AnimInstance found on GunMesh"));
        return;
    }

    // 현재 재생 중인 몽타주가 있다면 정지
    if (CurrentPlayingMontage && AnimInstance->Montage_IsPlaying(CurrentPlayingMontage))
    {
        AnimInstance->Montage_Stop(0.2f, CurrentPlayingMontage);
    }

    // 새 애니메이션 재생
    float Duration = AnimInstance->Montage_Play(AnimMontage, PlayRate);
    if (Duration > 0.0f)
    {
        CurrentPlayingMontage = AnimMontage;

        // 애니메이션 완료 콜백 바인딩
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AGunBase::OnAnimMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, AnimMontage);

        LOG_Item_WARNING(TEXT("PlayGunAnimation: Successfully started %s (Duration: %.2f)"),
            *AnimMontage->GetName(), Duration);
    }
    else
    {
        LOG_Item_WARNING(TEXT("PlayGunAnimation: Failed to play %s"), *AnimMontage->GetName());
    }
}

void AGunBase::OnAnimMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    LOG_Item_WARNING(TEXT("OnAnimMontageEnded: %s (Interrupted: %s)"),
        Montage ? *Montage->GetName() : TEXT("None"),
        bInterrupted ? TEXT("Yes") : TEXT("No"));

    if (CurrentPlayingMontage == Montage)
    {
        CurrentPlayingMontage = nullptr;
    }

    // 델리게이트 브로드캐스트
    OnAnimationComplete.Broadcast(Montage);
}

void AGunBase::ApplyGunDataFromDataTable()
{
    if (!GunDataTable || ItemRowName.IsNone())
    {
        LOG_Item_WARNING(TEXT("[GunBase::ApplyGunDataFromDataTable] GunDataTable 또는 RowName이 유효하지 않습니다."));
        return;
    }

    // 데이터 테이블에서 해당 행 데이터 로드
    FGunDataRow* RowData = GunDataTable->FindRow<FGunDataRow>(ItemRowName, TEXT("GunBase"));
    if (!RowData)
    {
        LOG_Item_WARNING(TEXT("[GunBase::ApplyGunDataFromDataTable] RowName '%s'에 해당하는 데이터를 찾을 수 없습니다."), *ItemRowName.ToString());
        return;
    }

    if (GunData.AnimationBlueprint && SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetAnimInstanceClass(GunData.AnimationBlueprint);
    }
    else
    {
        LOG_Item_WARNING(TEXT("[GunBase::ApplyGunDataFromDataTable] AnimationBlueprint 없음"));
    }

    // 총기 데이터 적용
    GunData = *RowData;

    // 데이터 테이블의 값들을 클래스 변수에 적용
    BaseDamage = GunData.BaseDamage;
    FireRate = GunData.FireRate;
    FireRange = GunData.Range;
    Spread = GunData.Spread;
    BulletsPerShot = GunData.BulletsPerShot;
    CurrentFireMode = GunData.DefaultFireMode;
    bCanToggleFireMode = GunData.bCanToggleFireMode;
    AvailableFireModes = GunData.AvailableFireModes;
    VerticalRecoilAmount = GunData.VerticalRecoilAmount;
    HorizontalRecoilAmount = GunData.HorizontalRecoilAmount;

    // 이펙트 및 사운드 설정
    MuzzleFlash = GunData.MuzzleFlash;
    ImpactDecalMaterial = GunData.ImpactDecal;
    DecalSize = GunData.DecalSize;
    DecalLifeSpan = GunData.DecalLifeSpan;
    FireSound = GunData.FireSound;
    EmptySound = GunData.EmptySound;
    FireModeSwitchSound = GunData.FireModeSwitchSound;

    // 탄피 이펙트 설정 및 소켓 할당
    if (ShellEjectionComponent && GunData.ShellEjectEffect)
    {
        ShellEjectionComponent->SetShellParticleSystem(GunData.ShellEjectEffect);
        ShellEjectionComponent->RefreshSocketCache();
    }

    ApplyAttachmentsFromDataTable();
}

bool AGunBase::Reload()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        LOG_Item_WARNING(TEXT("[AGunBase::Reload] Owner is NULL"));
        return false;
    }

    ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor);
    if (!OwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[AGunBase::Reload] Owner is NULL"));
        return false;
    }

    if (Durability >= MaxDurability)
    {
        return false;
    }

    // 약실에 탄이 남아있는지 확인
    bool bHasChambered = (Durability > 0.0f);

    if (bHasChambered)
    {
        Durability = MaxDurability + 1.0f;
    }
    else
    {
        Durability = MaxDurability;
    }

    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (UToolbarInventoryComponent* ToolbarComp = OwnerPawn->FindComponentByClass<UToolbarInventoryComponent>())
        {
            ToolbarComp->SyncGunStateToSlot();

            if (HasAuthority())
            {
                int32 CurrentAmmo = FMath::RoundToInt(Durability);
                int32 MaxAmmo = FMath::RoundToInt(MaxDurability);
                int32 CurrentSlotIndex = ToolbarComp->GetCurrentEquippedSlotIndex();
                ToolbarComp->MulticastSetGunAmmoUIVisibility(true, CurrentAmmo, MaxAmmo, CurrentFireMode, AvailableFireModes);
            }
        }
    }

    OnItemStateChanged.Broadcast();

    return true;
}

void AGunBase::CheckReloadCondition()
{
    if (Durability >= MaxDurability)
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    if (!IsValid(OwnerActor))
    {
        LOG_Item_WARNING(TEXT("Owner is NULL"));
        return;
    }

    ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor);
    if (!IsValid(OwnerCharacter))
    {
        LOG_Item_WARNING(TEXT("Owner Cast Fail"));
        return;
    }

    OwnerCharacter->StartReload();
}

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGunBase, RecentHits);
    DOREPLIFETIME(AGunBase, CurrentFireMode);
    DOREPLIFETIME(AGunBase, bIsSpotlightActive);
}

bool AGunBase::CanFire()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        LOG_Item_WARNING(TEXT("[AGunBase::CanFire] Owner is NULL"));
        return false;
    }

    ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor);
    if (!OwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[AGunBase::CanFire] Owner is NULL"));
        return false;
    }

    if (OwnerCharacter->bIsReloading)
    {
        LOG_Item_WARNING(TEXT("[AGunBase::CanFire] 재장전 중 - 발사 불가"));
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastFire = CurrentTime - LastFireTime;
    if (TimeSinceLastFire + 0.005f < FireRate)
    {
        LOG_Item_WARNING(TEXT("[AGunBase::CanFire] 발사 간격 부족 - 남은 시간: %.2f초"), FireRate - TimeSinceLastFire);
        return false;
    }

    // 탄약 부족 체크
    if (Durability <= 0.0f)
    {
        LOG_Item_WARNING(TEXT("[AGunBase::CanFire] 탄약 부족"));

        if (EmptySound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, EmptySound, GetActorLocation());
        }
        StopAutoFire();
        return false;
    }

    return true;
}

void AGunBase::FireSingle()
{
    Server_Fire();

    LastFireTime = GetWorld()->GetTimeSeconds();
}

void AGunBase::StartAutoFire()
{
    if (bIsAutoFiring)
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[StartAutoFire] 연발 사격 시작"));

    bIsAutoFiring = true;

    // 즉시 첫 발 발사
    if (CanFire())
    {
        Server_Fire();
        LastFireTime = GetWorld()->GetTimeSeconds();
    }

    // 연발 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(AutoFireTimerHandle, this, &AGunBase::FireAuto, FireRate, true);
}

void AGunBase::FireAuto()
{
    if (!bIsAutoFiring)
    {
        StopAutoFire();
        return;
    }

    if (!CanFire())
    {
        return;
    }

    Server_Fire();

    LastFireTime = GetWorld()->GetTimeSeconds();
}

void AGunBase::StopAutoFire()
{
    if (!bIsAutoFiring)
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[StopAutoFire] 연발 사격 중단"));

    bIsAutoFiring = false;

    // 연발 타이머 정리
    if (GetWorld() && AutoFireTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
    }
}

bool AGunBase::CanToggleFireMode() const
{
    return bCanToggleFireMode && AvailableFireModes.Num() > 1;
}

bool AGunBase::IsFireModeAvailable(EFireMode FireMode) const
{
    return AvailableFireModes.Contains(FireMode);
}

void AGunBase::ToggleFireMode()
{
    if (!HasAuthority())
    {
        Server_ToggleFireMode();
        return;
    }

    Server_ToggleFireMode_Implementation();
}

void AGunBase::Server_ToggleFireMode_Implementation()
{
    // 발사 모드 전환이 불가능한 총기라면 리턴
    if (!CanToggleFireMode())
    {
        LOG_Item_WARNING(TEXT("[Server_ToggleFireMode] 이 총기는 발사 모드 전환이 불가능합니다."));
        return;
    }

    // 연발 중이면 먼저 중단
    if (bIsAutoFiring)
    {
        StopAutoFire();
    }

    // 발사 모드 전환
    EFireMode OldMode = CurrentFireMode;
    int32 CurrentIndex = AvailableFireModes.Find(CurrentFireMode);

    if (CurrentIndex != INDEX_NONE)
    {
        // 다음 인덱스로 이동 (순환)
        int32 NextIndex = (CurrentIndex + 1) % AvailableFireModes.Num();
        CurrentFireMode = AvailableFireModes[NextIndex];
    }
    else
    {
        // 현재 모드가 목록에 없다면 첫 번째 모드로 설정
        CurrentFireMode = AvailableFireModes.Num() > 0 ? AvailableFireModes[0] : EFireMode::Single;
    }

    Multicast_PlayFireModeSwitchSound();
    UpdateGunUI();
}


void AGunBase::SetEquipped(bool bNewEquipped)
{
    Super::SetEquipped(bNewEquipped);

    if (bNewEquipped)
    {
        // 데이터 로딩 확인
        EnsureGunDataLoaded();

        ApplyAttachmentsFromDataTable();
        
        // 스포트라이트 초기화
        InitializeSpotlight();

        // 스포트라이트가 있는 총기라면 장착 시 자동으로 켜기
        if (HasSpotlight())
        {
            Server_SetSpotlightActive(true);
        }
    }
    else
    {
        // 장착 해제 시 연발 사격 중단
        if (bIsAutoFiring)
        {
            StopAutoFire();
        }

        // 장착 해제 시 스포트라이트 끄기
        if (bIsSpotlightActive)
        {
            Server_SetSpotlightActive(false);
        }
    }
}

bool AGunBase::IsGunDataLoaded() const
{
    // 핵심 데이터들이 로드되었는지 확인
    return ImpactDecalMaterial != nullptr && GunData.FireAnimation != nullptr && !ItemRowName.IsNone();
}

void AGunBase::EnsureGunDataLoaded()
{
    // 이미 로드되었다면 스킵
    if (IsGunDataLoaded())
    {
        LOG_Item_WARNING(TEXT("[EnsureGunDataLoaded] 데이터가 이미 로드됨 - 스킵"));
        return;
    }

    // ItemRowName이 설정되었는지 확인
    if (ItemRowName.IsNone())
    {
        LOG_Item_WARNING(TEXT("[EnsureGunDataLoaded] ItemRowName이 설정되지 않음"));
        return;
    }

    // 데이터 테이블이 있는지 확인
    if (!GunDataTable)
    {
        LOG_Item_WARNING(TEXT("[EnsureGunDataLoaded] GunDataTable이 null"));

        // 게임인스턴스에서 다시 가져오기 시도
        UWorld* World = GetWorld();
        if (World)
        {
            UGameInstance* GI = World->GetGameInstance();
            if (GI)
            {
                ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
                if (GISubsystem && GISubsystem->GunDataTable)
                {
                    GunDataTable = GISubsystem->GunDataTable;
                }
            }
        }

        if (!GunDataTable)
        {
            LOG_Item_WARNING(TEXT("[EnsureGunDataLoaded] GunDataTable 재획득 실패"));
            return;
        }
    }

    ApplyGunDataFromDataTable();
    ApplyItemDataFromTable();
    ApplyAttachmentsFromDataTable();

    // 로드 결과 확인
    if (!IsGunDataLoaded())
    {
        LOG_Item_WARNING(TEXT("[EnsureGunDataLoaded] 데이터 로드 실패"));
    }
}

void AGunBase::ApplyAttachmentsFromDataTable()
{
    // 스코프 부착 처리
    if (GunData.ScopeMesh)
    {
        AttachScope(GunData.ScopeMesh, GunData.ScopeSocketName);
    }
    else
    {
        DetachScope();
        LOG_Item_WARNING(TEXT("[ApplyAttachmentsFromDataTable] 스코프 없음"));
    }

    ApplyMagazineFromDataTable();
    InitializeSpotlight();
}

void AGunBase::ApplyMagazineFromDataTable()
{
    if (!GunData.bHasMagazine)
    {
        DetachMagazine();
        LOG_Item_WARNING(TEXT("[ApplyMagazineFromDataTable] 이 총기는 탄창을 사용하지 않음: %s"), *GunData.GunName.ToString());
        return;
    }

    // 탄창 부착 처리
    if (GunData.MagazineMesh)
    {
        AttachMagazine(GunData.MagazineMesh, GunData.AttachMagazineSocketName);
    }
    else
    {
        DetachMagazine();
        LOG_Item_WARNING(TEXT("[ApplyMagazineFromDataTable] 탄창 없음"));
    }
}

void AGunBase::UpdateGunUI()
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (UToolbarInventoryComponent* ToolbarComp = OwnerPawn->FindComponentByClass<UToolbarInventoryComponent>())
        {
            if (HasAuthority())
            {
                int32 CurrentAmmo = FMath::RoundToInt(Durability);
                int32 MaxAmmo = FMath::RoundToInt(MaxDurability);
                ToolbarComp->MulticastSetGunAmmoUIVisibility(true, CurrentAmmo, MaxAmmo, CurrentFireMode, AvailableFireModes);
            }
        }
    }
}

void AGunBase::AttachScope(UStaticMesh* ScopeMesh, FName SocketName)
{
    if (!ScopeMesh || !ScopeComponent)
    {
        LOG_Item_WARNING(TEXT("[AttachScope] ScopeMesh 또는 ScopeComponent가 null"));
        return;
    }

    USkeletalMeshComponent* GunMesh = GetSkeletalMeshComponent();
    if (!GunMesh)
    {
        LOG_Item_WARNING(TEXT("[AttachScope] GunMesh가 null"));
        return;
    }

    // 소켓 존재 확인
    if (!GunMesh->DoesSocketExist(SocketName))
    {
        LOG_Item_WARNING(TEXT("[AttachScope] 소켓 '%s'이 존재하지 않음. 기본 위치에 부착"), *SocketName.ToString());

        // 소켓이 없으면 기본 위치에 부착
        ScopeComponent->AttachToComponent(GunMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    else
    {
        // 소켓에 부착
        ScopeComponent->AttachToComponent(GunMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    }

    // 스코프 메시 설정
    ScopeComponent->SetStaticMesh(ScopeMesh);

    // 스코프 표시
    ScopeComponent->SetVisibility(true);

    // 현재 부착된 스코프 저장
    CurrentAttachedScope = ScopeMesh;
}

void AGunBase::DetachScope()
{
    if (!ScopeComponent)
    {
        return;
    }

    // 스코프 숨기기
    ScopeComponent->SetVisibility(false);

    // 메시 제거
    ScopeComponent->SetStaticMesh(nullptr);

    // 현재 부착된 스코프 초기화
    CurrentAttachedScope = nullptr;
}

bool AGunBase::HasScopeAttached() const
{
    return CurrentAttachedScope != nullptr && ScopeComponent && ScopeComponent->IsVisible();
}

void AGunBase::InitializeGameplayTags()
{
    MetalTag = FGameplayTag::RequestGameplayTag(TEXT("Material.Metal"));
    WoodTag = FGameplayTag::RequestGameplayTag(TEXT("Material.Wood"));
    FleshTag = FGameplayTag::RequestGameplayTag(TEXT("Material.Flesh"));
    StoneTag = FGameplayTag::RequestGameplayTag(TEXT("Material.Stone"));
}

void AGunBase::HandleGimmickDestruction(ALCBaseGimmick* Gimmick, const FHitResult& HitResult)
{
    if (!IsValid(Gimmick)) return;

    LOG_Item_WARNING(TEXT("🔫 파괴 가능 기믹 피격됨 → Destroy(): %s"), *Gimmick->GetName());

    // TODO: 이후 파괴 이펙트, 사운드, 이펙트 스폰 등 확장 가능
    // ex) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyEffect, HitResult.ImpactPoint);

    Gimmick->Destroy();
}

void AGunBase::DropMagazine()
{
    if (!HasAuthority())
    {
        return;
    }

    Multicast_DropMagazine();
}

void AGunBase::Multicast_DropMagazine_Implementation()
{
    EnsureGunDataLoaded();

    if (!GunData.MagazineMesh)
    {
        LOG_Item_WARNING(TEXT("[DropMagazine] MagazineMesh가 설정되지 않음"));
        return;
    }

    USkeletalMeshComponent* GunMesh = GetSkeletalMeshComponent();
    if (!GunMesh)
    {
        LOG_Item_WARNING(TEXT("[DropMagazine] GunMesh가 null"));
        return;
    }

    FVector SpawnLocation;
    FRotator SpawnRotation;

    // 소켓이 있으면 소켓 위치에서, 없으면 기본 위치에서 생성
    if (GunMesh->DoesSocketExist(GunData.MagazineSocketName))
    {
        SpawnLocation = GunMesh->GetSocketLocation(GunData.MagazineSocketName);
        SpawnRotation = GunMesh->GetSocketRotation(GunData.MagazineSocketName);
    }
    else
    {
        SpawnLocation = GetActorLocation() + GetActorForwardVector() * 20.0f + GetActorRightVector() * -15.0f;
        SpawnRotation = GetActorRotation();
        LOG_Item_WARNING(TEXT("[DropMagazine] 소켓 없음 - 기본 위치에서 생성: %s (소켓명: %s)"), *SpawnLocation.ToString(), *GunData.MagazineSocketName.ToString());
    }

    SpawnAndDropMagazine(GunData.MagazineMesh, SpawnLocation, SpawnRotation);
}

void AGunBase::SpawnAndDropMagazine(UStaticMesh* MagazineMesh, FVector SpawnLocation, FRotator SpawnRotation)
{
    if (!MagazineMesh || !GetWorld())
    {
        LOG_Item_WARNING(TEXT("[SpawnAndDropMagazine] MagazineMesh(%s) 또는 World가 null"), MagazineMesh ? TEXT("Valid") : TEXT("NULL"));
        return;
    }

    // 동적으로 액터 생성
    AActor* MagazineActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
    if (!MagazineActor)
    {
        LOG_Item_WARNING(TEXT("[SpawnAndDropMagazine] 탄창 액터 생성 실패"));
        return;
    }

    // 스태틱 메시 컴포넌트 추가
    UStaticMeshComponent* Magazine = NewObject<UStaticMeshComponent>(MagazineActor);
    if (!Magazine)
    {
        LOG_Item_WARNING(TEXT("[SpawnAndDropMagazine] MagazineComponent 생성 실패"));
        MagazineActor->Destroy();
        return;
    }

    // 메시 설정
    Magazine->SetStaticMesh(MagazineMesh);

    // 충돌 설정
    Magazine->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Magazine->SetCollisionObjectType(ECC_WorldDynamic);
    Magazine->SetCollisionResponseToAllChannels(ECR_Block);
    Magazine->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // 플레이어와는 충돌 무시

    // 물리 시뮬레이션 활성화
    Magazine->SetSimulatePhysics(true);
    Magazine->SetMassOverrideInKg(NAME_None, 0.5f); // 탄창 무게 설정 (0.5kg)

    // 루트 컴포넌트로 설정
    MagazineActor->SetRootComponent(Magazine);
    MagazineActor->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);

    // 컴포넌트 등록 확인
    Magazine->RegisterComponent();

    // 일정 시간 후 자동 삭제
    MagazineActor->SetLifeSpan(GunData.MagazineLifespan);
}

void AGunBase::AttachMagazine(UStaticMesh* MagazineMesh, FName SocketName)
{
    if (!MagazineMesh || !MagazineComponent)
    {
        LOG_Item_WARNING(TEXT("[AttachMagazine] MagazineMesh 또는 MagazineComponent가 null"));
        return;
    }

    USkeletalMeshComponent* GunMesh = GetSkeletalMeshComponent();
    if (!GunMesh)
    {
        LOG_Item_WARNING(TEXT("[AttachMagazine] GunMesh가 null"));
        return;
    }

    // 소켓/본 존재 확인
    if (!GunMesh->DoesSocketExist(SocketName))
    {
        LOG_Item_WARNING(TEXT("[AttachMagazine] 소켓/본 '%s'이 존재하지 않음"), *SocketName.ToString());
        return;
    }

    // 소켓에 부착 (SnapToTargetIncludingScale을 사용하여 본의 움직임을 따라감)
    MagazineComponent->AttachToComponent(GunMesh,
        FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);

    // 탄창 메시 설정
    MagazineComponent->SetStaticMesh(MagazineMesh);

    // 탄창의 회전을 원하는 각도로 설정 (Pitch, Yaw, Roll 순서)
    FRotator DesiredRotation = FRotator(0.0f, 0.0f, -90.0f);
    MagazineComponent->SetRelativeRotation(DesiredRotation);

    // 탄창 표시
    MagazineComponent->SetVisibility(true);

    // 현재 부착된 탄창 저장
    CurrentAttachedMagazine = MagazineMesh;
}

void AGunBase::DetachMagazine()
{
    if (!MagazineComponent)
    {
        return;
    }

    // 탄창 숨기기
    MagazineComponent->SetVisibility(false);
}

bool AGunBase::UsesMagazine() const
{
    return GunData.bHasMagazine;
}

bool AGunBase::HasMagazineAttached() const
{
    return CurrentAttachedMagazine != nullptr && MagazineComponent && MagazineComponent->IsVisible();
}

void AGunBase::OnRep_SpotlightActive()
{
    // 복제 변수가 변경될 때마다 자동으로 호출
    if (SpotlightComponent && HasSpotlight())
    {
        SpotlightComponent->SetHiddenInGame(!bIsSpotlightActive);
    }
}

void AGunBase::ToggleSpotlight()
{
    Server_SetSpotlightActive(!bIsSpotlightActive);
}

void AGunBase::Server_SetSpotlightActive_Implementation(bool bActive)
{
    if (!HasSpotlight())
    {
        LOG_Item_WARNING(TEXT("[Server_SetSpotlightActive] 이 총기는 스포트라이트가 없음"));
        return;
    }

    bIsSpotlightActive = bActive;

    // 서버에서도 즉시 적용 (RepNotify는 클라이언트만 호출됨)
    OnRep_SpotlightActive();
}

bool AGunBase::HasSpotlight() const
{
    return GunData.bHasSpotlight;
}

void AGunBase::ApplySpotlightSettings()
{
    if (!SpotlightComponent)
    {
        return;
    }

    // 데이터 테이블에서 가져온 설정 적용
    SpotlightComponent->SetIntensity(GunData.SpotlightIntensity);
    SpotlightComponent->SetAttenuationRadius(GunData.SpotlightAttenuationRadius);
    SpotlightComponent->SetInnerConeAngle(GunData.SpotlightInnerConeAngle);
    SpotlightComponent->SetOuterConeAngle(GunData.SpotlightOuterConeAngle);
    SpotlightComponent->SetLightColor(GunData.SpotlightColor);
}

void AGunBase::InitializeSpotlight()
{
    if (!HasSpotlight() || !SpotlightComponent)
    {
        return;
    }

    USkeletalMeshComponent* GunMesh = GetSkeletalMeshComponent();
    if (!GunMesh)
    {
        return;
    }

    // 소켓 존재 확인 후 부착
    if (GunMesh->DoesSocketExist(GunData.SpotlightSocketName))
    {
        SpotlightComponent->AttachToComponent(GunMesh,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            GunData.SpotlightSocketName);
    }
    else
    {
        LOG_Item_WARNING(TEXT("[InitializeSpotlight] 소켓 '%s'이 존재하지 않음 - 기본 부착 유지"),
            *GunData.SpotlightSocketName.ToString());
    }

    // 설정 적용
    ApplySpotlightSettings();
    SpotlightComponent->SetHiddenInGame(!bIsSpotlightActive);
}
