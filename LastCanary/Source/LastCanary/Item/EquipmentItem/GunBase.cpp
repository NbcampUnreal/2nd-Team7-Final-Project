#include "Item/EquipmentItem/GunBase.h"
#include "Item/ItemBase.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagAssetInterface.h"
#include "LastCanary.h"

AGunBase::AGunBase()
{
    ;
    FireRange = 10000.0f;
    BaseDamage = 20.0f;
    FireRate = 0.2f;
    Spread = 2.0f;
    BulletsPerShot = 1;
    MaxAmmo = 30.0f;
    CurrentAmmo = MaxAmmo;
    LastFireTime = 0.0f;
    DecalSize = FVector(5.0f, 5.0f, 5.0f);
    DecalLifeSpan = 10.0f;

    MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
    MuzzleLocation->SetupAttachment(RootComponent);
    MuzzleLocation->SetRelativeLocation(FVector(0, 50, 10));

    bDrawDebugLine = true;
    bDrawImpactDebug = true;
    DebugDrawDuration = 10.0f;
}

void AGunBase::UseItem()
{
    LOG_Item_WARNING(TEXT("[UseItem] 발사 시도 - 모드: %s"),
        CurrentFireMode == EFireMode::Single ? TEXT("단발") : TEXT("연발"));

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
            LOG_Item_WARNING(TEXT("[UseItem] 연발 사격 중단"));
        }
        else
        {
            // 연발 중이 아니면 시작
            StartAutoFire();
            LOG_Item_WARNING(TEXT("[UseItem] 연발 사격 시작"));
        }
    }
}

void AGunBase::Server_Fire_Implementation()
{
    HandleFire();

    //Multicast_SpawnImpactEffects(RecentHits);
}

void AGunBase::HandleFire()
{
    float OldDurability = Durability;
    Durability = FMath::Max(0.0f, Durability - 1.0f);

    if (Durability > MaxAmmo)
    {
        LOG_Item_WARNING(TEXT("[HandleFire] 경고: Durability(%.0f)가 MaxAmmo(%.0f)를 초과함. 수정합니다."),
            Durability, MaxAmmo);
        Durability = MaxAmmo;
    }

    UpdateAmmoState();

    LOG_Item_WARNING(TEXT("[HandleFire] 총알 소모: %.0f → %.0f (남은 총알: %.0f/%.0f)"),
        OldDurability, Durability, Durability, MaxAmmo);

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

    Multicast_PlayFireEffects();

    if (RecentHits.Num() > 0)
    {
        Multicast_SpawnImpactEffects(RecentHits);
    }

    // 애니메이션 재생
    Multicast_PlayFireAnimation();
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

    // 디버그 라인 표시 (개발 모드나 디버그 설정이 활성화된 경우에만)
    if (bDrawDebugLine)
    {
        if (bHit) // 명중한 경우
        {
            // 시작점에서 히트 지점까지 녹색 라인
            DrawDebugLine(GetWorld(), StartLocation, OutHit.ImpactPoint, FColor::Green, false, DebugDrawDuration, 0, 2.0f);

            // 히트 지점에 빨간색 구체 표시 (더 크게 만들어 가시성 향상)
            DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 10.0f, 16, FColor::Red, false, DebugDrawDuration);

            // 히트 지점에 법선 방향 표시
            DrawDebugDirectionalArrow(GetWorld(), OutHit.ImpactPoint,
                OutHit.ImpactPoint + OutHit.ImpactNormal * 50.0f,
                20.0f, FColor::Blue, false, DebugDrawDuration);
        }
        else // 명중하지 않은 경우
        {
            // 전체 라인을 빨간색으로 표시
            DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, DebugDrawDuration, 0, 2.0f);
        }
    }

    return bHit;
}

void AGunBase::ProcessHit(const FHitResult& HitResult, const FVector& StartLocation)
{
    AActor* HitActor = HitResult.GetActor();

    if (HitActor && HitActor != this && HitActor != GetOwner())
    {
        // GameplayTag로 적 캐릭터 판별
        static const FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy"));

        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(HitActor);
        if (TagInterface && TagInterface->HasMatchingGameplayTag(EnemyTag))
        {
            float AppliedDamage = BaseDamage;
            LOG_Item_WARNING(TEXT("ProcessHit: Applying %.1f damage to enemy %s"),
                AppliedDamage, *HitActor->GetName());

            FPointDamageEvent DamageEvent(
                AppliedDamage,
                HitResult,
                (HitResult.ImpactPoint - StartLocation).GetSafeNormal(),
                nullptr
            );

            float ActualDamage = HitActor->TakeDamage(
                AppliedDamage,
                DamageEvent,
                GetInstigatorController(),
                this
            );
        }
        else
        {
            LOG_Item_WARNING(TEXT("ProcessHit: Hit non-enemy actor %s. No damage applied"),
                *HitActor->GetName());
        }
    }
}

void AGunBase::Multicast_SpawnImpactEffects_Implementation(const TArray<FHitResult>& Hits)
{
    // 모든 히트 지점에 대해 효과 생성
    for (const FHitResult& Hit : Hits)
    {
        FVector EnhancedDecalSize = DecalSize * 5.0f;

        // 데칼 생성 위치를 약간 앞으로 이동 (표면 겹침 방지)
        FVector AdjustedLocation = Hit.ImpactPoint + Hit.ImpactNormal * 0.5f;

        if (ImpactDecalMaterial)
        {
            UGameplayStatics::SpawnDecalAtLocation(
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
            LOG_Item_WARNING(TEXT("SpawnImpactEffects: No decal material assigned"));
        }

        if (ImpactSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.ImpactPoint);
            LOG_Item_WARNING(TEXT("SpawnImpactEffects: Played sound %s"), *ImpactSound->GetName());
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

    // 총구 화염 이펙트 재생
    if (MuzzleFlash && MuzzleLocation)
    {
        UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlash,
            MuzzleLocation,
            NAME_None,
            MuzzleLocation->GetComponentLocation(),
            MuzzleLocation->GetComponentRotation(),
            EAttachLocation::KeepWorldPosition
        );
    }

    // 플레이어 카메라 흔들림 등 추가 이펙트 (옵션)
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn && OwnerPawn->IsLocallyControlled())
    {
        // TODO : 카메라 흔들림 효과나 반동 애니메이션 등을 여기서 처리할 수 있음
    }
}

void AGunBase::Multicast_PlayFireAnimation_Implementation()
{
    if (GunData.FireAnimation)
    {
        PlayGunAnimation(GunData.FireAnimation);
        LOG_Item_WARNING(TEXT("Playing fire animation: %s"), *GunData.FireAnimation->GetName());
    }
    else
    {
        LOG_Item_WARNING(TEXT("No fire animation assigned"));
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

    /*ApplyGunDataFromDataTable();
    ApplyItemDataFromTable();*/

    if (Durability > MaxAmmo || Durability <= 0.0f)
    {
        Durability = MaxAmmo;
    }

    UpdateAmmoState();

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

    // 총기 데이터 적용
    GunData = *RowData;

    // 데이터 테이블의 값들을 클래스 변수에 적용
    BaseDamage = GunData.BaseDamage;
    FireRate = GunData.FireRate;
    FireRange = GunData.Range;
    Spread = GunData.Spread;
    BulletsPerShot = GunData.BulletsPerShot;
    MaxAmmo = GunData.MaxAmmo;

    if (Durability > MaxAmmo)
    {
        Durability = MaxAmmo;
    }

    // 이펙트 및 사운드 설정
    MuzzleFlash = GunData.MuzzleFlash;
    ImpactDecalMaterial = GunData.ImpactDecal;
    DecalSize = GunData.DecalSize;
    DecalLifeSpan = GunData.DecalLifeSpan;
    FireSound = GunData.FireSound;
    EmptySound = GunData.EmptySound;
    ImpactSound = GunData.ImpactSound;
}

void AGunBase::UpdateAmmoState()
{
    CurrentAmmo = Durability;

    if (Durability <= 0.0f)
    {
        // TODO : 탄약 부족 UI 표시
        LOG_Item_WARNING(TEXT("[UpdateAmmoState] 탄약 완전 소진"));
    }
}

bool AGunBase::Reload(float AmmoAmount)
{
    if (FMath::IsNearlyEqual(Durability, MaxAmmo))
    {
        return false;
    }

    Multicast_PlayReloadAnimation();
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

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGunBase, RecentHits);
    //DOREPLIFETIME(AGunBase, CurrentFireMode);
    //DOREPLIFETIME(AGunBase, bIsAutoFiring);
}

bool AGunBase::CanFire()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // 탄약 부족 체크
    if (Durability <= 0.0f)
    {
        LOG_Item_WARNING(TEXT("[CanFire] 탄약 부족"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
                FString::Printf(TEXT("탄약 부족! 현재: %.0f/%.0f"), Durability, MaxAmmo));
        }

        if (EmptySound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, EmptySound, GetActorLocation());
        }
        return false;
    }

    return true;
}

void AGunBase::FireSingle()
{
    LOG_Item_WARNING(TEXT("[FireSingle] 단발 사격"));

    // 클라이언트에서는 서버 RPC만 호출
    if (!HasAuthority())
    {
        Server_Fire();
        return;
    }

    // 서버에서 직접 발사 처리
    HandleFire();

    // 마지막 발사 시간 업데이트
    LastFireTime = GetWorld()->GetTimeSeconds();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green,
            FString::Printf(TEXT("단발 발사! 남은 탄약: %.0f"), Durability));
    }
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
        if (HasAuthority())
        {
            HandleFire();
        }
        else
        {
            Server_Fire();
        }
        LastFireTime = GetWorld()->GetTimeSeconds();
    }

    // 연발 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(AutoFireTimerHandle,
        this, &AGunBase::FireAuto, FireRate, true);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan,
            FString::Printf(TEXT("연발 사격 시작! (%.1f초 간격)"), FireRate));
    }
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
        // 탄약이 부족하거나 다른 이유로 발사할 수 없으면 연발 중단
        StopAutoFire();
        return;
    }

    LOG_Item_WARNING(TEXT("[FireAuto] 연발 사격 중"));

    // 서버에서 발사 처리
    if (HasAuthority())
    {
        HandleFire();
    }
    else
    {
        Server_Fire();
    }

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

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange,
            FString::Printf(TEXT("연발 사격 중단")));
    }
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
    // 연발 중이면 먼저 중단
    if (bIsAutoFiring)
    {
        StopAutoFire();
    }

    // 발사 모드 전환
    EFireMode OldMode = CurrentFireMode;
    CurrentFireMode = (CurrentFireMode == EFireMode::Single) ? EFireMode::FullAuto : EFireMode::Single;

    FString ModeString = (CurrentFireMode == EFireMode::Single) ? TEXT("단발") : TEXT("연발");

    LOG_Item_WARNING(TEXT("[Server_ToggleFireMode] 발사 모드 변경: %s → %s"),
        OldMode == EFireMode::Single ? TEXT("단발") : TEXT("연발"),
        *ModeString);
}


void AGunBase::SetEquipped(bool bNewEquipped)
{
    Super::SetEquipped(bNewEquipped);

    // 장착 해제 시 연발 사격 중단
    if (!bNewEquipped && bIsAutoFiring)
    {
        StopAutoFire();
        LOG_Item_WARNING(TEXT("[SetEquipped] 장착 해제로 인한 연발 사격 중단"));
    }
}
