#include "AI/BaseBossMonsterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "AI/LCBaseBossAIController.h"
#include "TimerManager.h"

ABaseBossMonsterCharacter::ABaseBossMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // 1) Aura 컴포넌트 생성
    AuraFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AuraFX"));
    AuraFX->SetupAttachment(GetRootComponent());
    AuraFX->bAutoActivate = true;   // 디폴트 활성화

    // 1) BerserkFX1 컴포넌트 생성
    BerserkFX1 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BerserkFX1"));
    BerserkFX1->SetupAttachment(GetRootComponent());
    BerserkFX1->bAutoActivate = false;   // 디폴트 비활성화

    // 2) BerserkFX2 컴포넌트 생성
    BerserkFX2 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BerserkFX2"));
    BerserkFX2->SetupAttachment(GetRootComponent());
    BerserkFX2->bAutoActivate = false;   // 디폴트 비활성화

    // 데미지 수신 완전 차단
    SetCanBeDamaged(false);
}

void ABaseBossMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();   // ← 추가!

    // ── 데칼 풀 초기화 ────────────────────────────────────────────
    RemainingCommonDecals = CommonDecalClasses;
    RemainingUniqueDecals = UniqueDecalClasses;

    // 1초마다 애니메이션 초기화 실행 (true: 반복)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ResetAnimTimerHandle,
            this,
            &ABaseBossMonsterCharacter::ResetAnimationState,
            ResetAnimInterval,
            true
        );
    }

    // (1) 서버 권한이 있고, 최소 하나씩 남아 있으면 타이머 예약
    if (HasAuthority() &&
        RemainingCommonDecals.Num() > 0 &&
        RemainingUniqueDecals.Num() > 0)
    {
        float InitialDelay = FMath::RandRange(ClueSpawnIntervalMin, ClueSpawnIntervalMax);
        GetWorldTimerManager().SetTimer(
            ClueTimerHandle,
            this,
            &ABaseBossMonsterCharacter::SpawnRandomClue,
            InitialDelay,
            false
        );
    }
}

float ABaseBossMonsterCharacter::GetNextAttackRange() const
{
    return NextAttackRange;
}

void ABaseBossMonsterCharacter::ExecuteSelectedAttack()
{
    if (NextAttackAction)
        NextAttackAction();

    // 실행 후 초기화
    NextAttackAction = nullptr;
    NextAttackRange = 0.f;
}

bool ABaseBossMonsterCharacter::RequestAttack(float TargetDistance)
{

    return false;
}


// Tick이나 다른 타이밍에 호출하여 Rage를 갱신하고 싶다면 여기서 Berserk 배수를 적용
void ABaseBossMonsterCharacter::UpdateRage(float DeltaSeconds)
{

}


void ABaseBossMonsterCharacter::EnterBerserkState()
{
    if (!HasAuthority())
        return;

    StartBerserk();
}

void ABaseBossMonsterCharacter::UpdateBlackboardValues()
{
    // 컨트롤러 널 체크
    AController* C = GetController();
    if (!C) return;

    // AIController 캐스트
    ALCBaseBossAIController* AICon = Cast<ALCBaseBossAIController>(C);
    if (!AICon) return;

    // BlackboardComponent 널 체크
    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
    if (!BB) return;

    // RagePercent 계산 시 0으로 나누기 방지
    const float RagePct = (MaxRage > 0.f) ? (Rage / MaxRage) : 0.f;
    BB->SetValueAsFloat(TEXT("RagePercent"), RagePct);
    BB->SetValueAsBool(TEXT("IsBerserkMode"), bIsBerserk);
}

void ABaseBossMonsterCharacter::ResetAnimationState()
{
    USkeletalMeshComponent* SkelMesh = GetMesh();
    if (!SkelMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ResetAnimationState] SkeletalMeshComponent가 없습니다"));
        return;
    }

    // 몽타주 정지
    if (UAnimInstance* AnimInst = SkelMesh->GetAnimInstance())
    {
        AnimInst->StopAllMontages(0.f);
    }

    // 본 포즈 리셋
    SkelMesh->RefreshBoneTransforms();
    SkelMesh->TickAnimation(0.f, false);

    // Cloth 시뮬레이션을 다음 업데이트에서 Teleport & Reset 모드로 강제
    SkelMesh->ForceClothNextUpdateTeleportAndReset();

    // Physics Asset 재생성
    SkelMesh->DestroyPhysicsState();
    SkelMesh->CreatePhysicsState();

    // 렌더·바운드 갱신
    SkelMesh->UpdateBounds();
    SkelMesh->MarkRenderTransformDirty();
    SkelMesh->MarkRenderDynamicDataDirty();

    UE_LOG(LogTemp, Log, TEXT("[ResetAnimationState] 메시·Cloth·Physics 상태 초기화 완료"));
}


void ABaseBossMonsterCharacter::StartBerserk(float Duration)
{
    bIsBerserk = true;
    Rage = 60.f;
    Multicast_StartBerserk();

    if (HasAuthority() && Duration > 0.f)
    {
        GetWorldTimerManager().SetTimer(
            BerserkDurationHandle,
            this,
            &ABaseBossMonsterCharacter::EndBerserk,
            Duration,
            false
        );
    }
}

void ABaseBossMonsterCharacter::StartBerserk()
{
    bIsBerserk = true;
    Rage = 60.f;
    Multicast_StartBerserk();
}


void ABaseBossMonsterCharacter::EndBerserk()
{
    if (!bIsBerserk)
        return;

    bIsBerserk = false;

    if (HasAuthority())
    {
        GetWorldTimerManager().ClearTimer(BerserkDurationHandle);
    }

    Multicast_EndBerserk();
}

void ABaseBossMonsterCharacter::OnRep_IsBerserk()
{
    if (bIsBerserk)
    {
        if (EnterBerserkSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                EnterBerserkSound,
                GetActorLocation()
            );
        }

        UE_LOG(LogTemp, Warning, TEXT("[Berserk] 클라이언트: Berserk 시작"));
        // 클라이언트에서 FX 활성화
        if (BerserkFX1) BerserkFX1->Activate(true);
        if (BerserkFX2) BerserkFX2->Activate(true);

        if (BerserkSound && GetRootComponent())
        {
            ActiveBerserkAudio = UGameplayStatics::SpawnSoundAttached(
                BerserkSound,
                GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Berserk] 클라이언트: Berserk 종료"));
        // 비활성화 or 제거
        if (BerserkFX1) BerserkFX1->Deactivate();
        if (BerserkFX2) BerserkFX2->Deactivate();

        if (ActiveBerserkAudio)
        {
            ActiveBerserkAudio->Stop();
            ActiveBerserkAudio = nullptr;
        }
    }
}


void ABaseBossMonsterCharacter::Multicast_StartBerserk_Implementation()
{
    OnRep_IsBerserk();
    UE_LOG(LogTemp, Warning, TEXT("→ Multicast_StartBerserk 호출 (서버→클라이언트 전체)"));
}


void ABaseBossMonsterCharacter::Multicast_EndBerserk_Implementation()
{
    OnRep_IsBerserk();
    UE_LOG(LogTemp, Warning, TEXT("→ Multicast_EndBerserk 호출 (서버→클라이언트 전체)"));
}


void ABaseBossMonsterCharacter::SpawnRandomClue()
{
    // 0) world 널 체크
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnRandomClue] World is null"));
        return;
    }

    // ─── 공통 스폰 파라미터 ──────────────────────────────────────────
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // ─── (1) 공통+고유 풀 합치기 ─────────────────────────────────
    TArray<TSubclassOf<AActor>> Pool = RemainingCommonDecals;
    Pool.Append(RemainingUniqueDecals);

    if (Pool.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnRandomClue] 더 이상 스폰할 데칼이 없습니다"));
        return;
    }

    // ─── (2) 풀에서 하나 랜덤 선택 ────────────────────────────────
    int32 Idx = FMath::RandRange(0, Pool.Num() - 1);
    TSubclassOf<AActor> DecalClass = Pool[Idx];
    if (DecalClass)
    {
        // (2-1) 랜덤 위치 계산
        FVector BossLoc = GetActorLocation();
        float OffX = FMath::RandRange(-200.f, 200.f);
        float OffY = FMath::RandRange(-200.f, 200.f);
        FVector Loc = BossLoc + FVector(OffX, OffY, 0.f);

        // (2-2) 바닥 높이 보정
        FHitResult Hit;
        FCollisionQueryParams Params(NAME_None, false, this);
        FVector Start = Loc + FVector(0, 0, 500.f), End = Loc + FVector(0, 0, -1000.f);
        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            Loc.Z = Hit.Location.Z + 5.f;
        }

        // (2-3) 액터 스폰
        AActor* NewDecal = GetWorld()->SpawnActor<AActor>(
            DecalClass,
            Loc,
            FRotator(-90, 0, 0),
            SpawnParams
        );
        if (NewDecal)
        {
            UE_LOG(LogTemp, Log, TEXT("[SpawnRandomClue] Decal Spawned: %s at %s"),
                *NewDecal->GetName(), *Loc.ToCompactString());
        }
    }

    // ─── (3) 풀에서 제거 ─────────────────────────────────────────
    if (RemainingCommonDecals.Remove(DecalClass) == 0)
    {
        RemainingUniqueDecals.Remove(DecalClass);
    }

    // ─── (4) 다음 스폰 예약 ───────────────────────────────────────
    if (RemainingCommonDecals.Num() + RemainingUniqueDecals.Num() > 0)
    {
        float Delay = FMath::RandRange(ClueSpawnIntervalMin, ClueSpawnIntervalMax);
        GetWorldTimerManager().SetTimer(
            ClueTimerHandle,
            this,
            &ABaseBossMonsterCharacter::SpawnRandomClue,
            Delay,
            false
        );
    }
}

void ABaseBossMonsterCharacter::DealDamageInRange(float DamageAmount)
{
    // 0) World 유효성 검사
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DealDamageInRange] World is null"));
        return;
    }

    FVector Origin = GetActorLocation();
    float Radius = AttackRange;

    // (옵션) 디버깅용 범위 시각화
    //DrawDebugSphere(GetWorld(), Origin, Radius, 12, FColor::Red, false, 1.0f);

    // 반경 내 모든 Pawn 검사
    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        Origin,
        Origin,
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );

    if (bHit)
    {
        for (auto& Hit : HitResults)
        {
            APawn* Pawn = Cast<APawn>(Hit.GetActor());
            if (Pawn && Pawn->IsPlayerControlled())
            {
                // 대미지 적용
                UGameplayStatics::ApplyDamage(
                    Pawn,
                    DamageAmount,
                    GetController(),          // 보스의 컨트롤러
                    this,                     // 대미지 발생자
                    UDamageType::StaticClass()
                );
            }
        }
    }
}

void ABaseBossMonsterCharacter::EnableStencilForAllMeshes(int32 StencilValue)
{
    TArray<UMeshComponent*> MeshComponents;
    GetComponents<UMeshComponent>(MeshComponents);

    for (UMeshComponent* MeshComp : MeshComponents)
    {
        MeshComp->SetRenderCustomDepth(true);
        MeshComp->SetCustomDepthStencilValue(StencilValue);
    }
}


void ABaseBossMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Rage 복제
    DOREPLIFETIME(ABaseBossMonsterCharacter, Rage);

    // Berserk 상태 복제
    DOREPLIFETIME(ABaseBossMonsterCharacter, bIsBerserk);
}