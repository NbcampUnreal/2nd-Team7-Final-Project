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
    // 스켈레탈 메시 컴포넌트 생성 및 설정
    GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
    RootComponent = GunMesh;

    // 기본 속성 초기화
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

    // 무기 컴포넌트 초기화
    MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
    MuzzleLocation->SetupAttachment(GunMesh);
    MuzzleLocation->SetRelativeLocation(FVector(0, 50, 10));

    bDrawDebugLine = true;
    bDrawImpactDebug = true;
    DebugDrawDuration = 10.0f;
}

void AGunBase::UseItem()
{
    LOG_Item_WARNING(TEXT("UseItem() called"));

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (Durability <= 0.0f || CurrentTime - LastFireTime < FireRate)
    {
        if (Durability <= 0.0f)
        {
            LOG_Item_WARNING(TEXT("UseItem: Durability is 0 or less"));

            if (EmptySound)
            {
                LOG_Item_WARNING(TEXT("UseItem: Playing EmptySound"));
                UGameplayStatics::PlaySoundAtLocation(this, EmptySound, GetActorLocation());
            }
        }
        else
        {
            LOG_Item_WARNING(TEXT("UseItem: FireRate not met"));
        }
        return;
    }

    LOG_Item_WARNING(TEXT("UseItem: Firing weapon"));

    LastFireTime = CurrentTime;

    Multicast_PlayFireEffects();
    Multicast_PlayFireAnimation();
    Multicast_PlayFireEffects();

    // 클라이언트에서 호출된 경우 서버에 알림
    if (GetLocalRole() < ROLE_Authority)
    {
        LOG_Item_WARNING(TEXT("UseItem: Calling Server_Fire() from client"));
        Server_Fire();
    }
    else
    {
        LOG_Item_WARNING(TEXT("UseItem: Handling fire locally with HandleFire()"));
        HandleFire();

        // 서버에서는 즉시 모든 클라이언트에 효과 전파
        Multicast_SpawnImpactEffects(RecentHits);
    }

    LOG_Item_WARNING(TEXT("UseItem: Broadcasting OnItemStateChanged"));
    OnItemStateChanged.Broadcast();
}

void AGunBase::Server_Fire_Implementation()
{
    HandleFire();

    // 명시적으로 모든 클라이언트에 효과 표시 요청
    Multicast_SpawnImpactEffects(RecentHits);
}

void AGunBase::HandleFire()
{
    Durability = FMath::Max(0.0f, Durability - 1.0f);
    UpdateAmmoState();

    // 최근 히트 결과 초기화
    RecentHits.Empty();

    for (int32 i = 0; i < BulletsPerShot; i++)
    {
        FHitResult HitResult;
        FVector StartLocation;
        FVector EndLocation;

        if (PerformLineTrace(HitResult, StartLocation, EndLocation))
        {
            // 히트 결과 저장
            RecentHits.Add(HitResult);
            ProcessHit(HitResult, StartLocation);
        }
    }
}

bool AGunBase::PerformLineTrace(FHitResult& OutHit, FVector& StartLocation, FVector& EndLocation)
{
    AActor* OwnerActor = GetOwner();
    if (OwnerActor)
    {
        LOG_Item_WARNING(TEXT("PerformLineTrace: Owner exists - Name: %s, Class: %s"),
            *OwnerActor->GetName(),
            *OwnerActor->GetClass()->GetName());
    }
    else
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

    LOG_Item_WARNING(TEXT("PerformLineTrace: Camera Location = (%f, %f, %f), Rotation = (%f, %f, %f)"),
        CameraLocation.X, CameraLocation.Y, CameraLocation.Z,
        CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);

    FVector ShootDirection = CameraRotation.Vector();
    if (Spread > 0.0f)
    {
        ShootDirection = FMath::VRandCone(ShootDirection, FMath::DegreesToRadians(Spread));
        LOG_Item_WARNING(TEXT("PerformLineTrace: Applied spread, ShootDirection = (%f, %f, %f)"),
            ShootDirection.X, ShootDirection.Y, ShootDirection.Z);
    }

    StartLocation = MuzzleLocation ? MuzzleLocation->GetComponentLocation() : GetActorLocation();
    EndLocation = StartLocation + ShootDirection * FireRange;

    LOG_Item_WARNING(TEXT("PerformLineTrace: StartLocation = (%f, %f, %f), EndLocation = (%f, %f, %f)"),
        StartLocation.X, StartLocation.Y, StartLocation.Z,
        EndLocation.X, EndLocation.Y, EndLocation.Z);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(OwnerActor);
    QueryParams.bTraceComplex = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECC_Visibility, QueryParams);

    if (bHit)
    {
        LOG_Item_WARNING(TEXT("PerformLineTrace: Hit detected! ImpactPoint = (%f, %f, %f), Actor = %s"),
            OutHit.ImpactPoint.X, OutHit.ImpactPoint.Y, OutHit.ImpactPoint.Z,
            OutHit.GetActor() ? *OutHit.GetActor()->GetName() : TEXT("None"));
    }
    else
    {
        LOG_Item_WARNING(TEXT("PerformLineTrace: No hit detected"));
    }

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

            LOG_Item_WARNING(TEXT("PerformLineTrace: Debug visuals drawn for hit at (%f, %f, %f)"),
                OutHit.ImpactPoint.X, OutHit.ImpactPoint.Y, OutHit.ImpactPoint.Z);
        }
        else // 명중하지 않은 경우
        {
            // 전체 라인을 빨간색으로 표시
            DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, DebugDrawDuration, 0, 2.0f);
            LOG_Item_WARNING(TEXT("PerformLineTrace: Debug line drawn for miss"));
        }
    }

    return bHit;
}

void AGunBase::ProcessHit(const FHitResult& HitResult, const FVector& StartLocation)
{
    AActor* HitActor = HitResult.GetActor();
    LOG_Item_WARNING(TEXT("ProcessHit: Hit Actor = %s, Component = %s"),
        HitActor ? *HitActor->GetName() : TEXT("None"),
        HitResult.Component.IsValid() ? *HitResult.Component->GetName() : TEXT("None"));

    if (HitActor && HitActor != this && HitActor != GetOwner())
    {
        // GameplayTag로 적 캐릭터 판별
        static const FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy"));

        // ⭐ 수정된 부분: GameplayTagAssetInterface 사용
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

            LOG_Item_WARNING(TEXT("ProcessHit: Actual damage applied = %.1f"), ActualDamage);
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
        LOG_Item_WARNING(TEXT("SpawnImpactEffects: Spawning effects at location (%f, %f, %f)"),
            Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);

        // 데칼 크기 증가 (더 잘 보이도록)
        FVector EnhancedDecalSize = DecalSize * 5.0f;  // 5배 크기로 증가

        // 데칼 생성 위치를 약간 앞으로 이동 (표면 겹침 방지)
        FVector AdjustedLocation = Hit.ImpactPoint + Hit.ImpactNormal * 0.5f;

        if (ImpactDecalMaterial)
        {
            UGameplayStatics::SpawnDecalAtLocation(
                GetWorld(),
                ImpactDecalMaterial,
                EnhancedDecalSize,  // 크기 증가
                AdjustedLocation,   // 위치 조정
                Hit.ImpactNormal.Rotation(),
                DecalLifeSpan
            );
            LOG_Item_WARNING(TEXT("SpawnImpactEffects: Spawned decal with material %s"), *ImpactDecalMaterial->GetName());
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
        // 카메라 흔들림 효과나 반동 애니메이션 등을 여기서 처리할 수 있음
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
    if (!AnimMontage || !GunMesh)
    {
        LOG_Item_WARNING(TEXT("PlayGunAnimation: AnimMontage or GunMesh is null"));
        return;
    }

    // 애니메이션 인스턴스 가져오기
    UAnimInstance* AnimInstance = GunMesh->GetAnimInstance();
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

    // 게임인스턴스 서브시스템에서 데이터 테이블 참조 가져오기
    if (GISubsystem->GunDataTable)
    {
        GunDataTable = GISubsystem->GunDataTable;
    }
    else
    {
        LOG_Item_WARNING(TEXT("[GunBase::BeginPlay] 게임인스턴스 서브시스템의 GunDataTable이 null입니다!"));
    }

    if (Durability <= 0.0f)
    {
        Durability = MaxAmmo;
    }

    ApplyItemDataFromTable();
    ApplyGunDataFromDataTable();
    UpdateAmmoState();

    if (GunMesh)
    {
        UAnimInstance* AnimInstance = GunMesh->GetAnimInstance();
        if (AnimInstance)
        {
            LOG_Item_WARNING(TEXT("GunBase: AnimInstance found on GunMesh"));
        }
        else
        {
            LOG_Item_WARNING(TEXT("GunBase: No AnimInstance on GunMesh - animations will not work"));
        }
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

    // 총기 데이터 적용
    GunData = *RowData;

    // 데이터 테이블의 값들을 클래스 변수에 적용
    BaseDamage = GunData.BaseDamage;
    FireRate = GunData.FireRate;
    FireRange = GunData.Range;
    Spread = GunData.Spread;
    BulletsPerShot = GunData.BulletsPerShot;
    MaxAmmo = GunData.MaxAmmo;

    // 이펙트 및 사운드 설정
    MuzzleFlash = GunData.MuzzleFlash;
    ImpactDecalMaterial = GunData.ImpactDecal;
    DecalSize = GunData.DecalSize;
    DecalLifeSpan = GunData.DecalLifeSpan;
    FireSound = GunData.FireSound;
    EmptySound = GunData.EmptySound;
    ImpactSound = GunData.ImpactSound;

    // 메시 설정 (있을 경우)
    if (GunData.GunMesh)
    {
        GunMesh->SetSkeletalMesh(GunData.GunMesh);
        GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GunMesh->SetCollisionObjectType(ECC_WorldDynamic);
        GunMesh->SetCollisionResponseToAllChannels(ECR_Block);

        // 부모의 MeshComponent 숨기기
        if (MeshComponent)
        {
            MeshComponent->SetVisibility(false);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
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
}

USkeletalMeshComponent* AGunBase::GetGunMesh()
{
    return GunMesh;
}
