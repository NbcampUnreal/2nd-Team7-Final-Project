#include "AI/BaseBossMonsterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ABaseBossMonsterCharacter::ABaseBossMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

bool ABaseBossMonsterCharacter::RequestAttack()
{
    if (!HasAuthority())
        return false;

    const float Now = GetWorld()->GetTimeSeconds();

    // (1) 강공격 우선
    if (StrongAttackMontage &&
        FMath::FRand() < StrongAttackChance &&
        (Now - LastStrongTime) >= StrongAttackCooldown)
    {
        LastStrongTime = Now;
        PlayStrongAttack();
        return true;
    }

    // (2) 일반 공격
    if (NormalAttackMontage &&
        (Now - LastNormalTime) >= NormalAttackCooldown)
    {
        LastNormalTime = Now;
        PlayNormalAttack();
        return true;
    }

    return false;
}


void ABaseBossMonsterCharacter::PlayNormalAttack()
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->Montage_Play(NormalAttackMontage);
        Anim->OnMontageEnded.AddDynamic(this, &ABaseBossMonsterCharacter::OnAttackMontageEnded);
    }
}


void ABaseBossMonsterCharacter::PlayStrongAttack()
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->Montage_Play(StrongAttackMontage);
        Anim->OnMontageEnded.AddDynamic(this, &ABaseBossMonsterCharacter::OnAttackMontageEnded);
    }
}


void ABaseBossMonsterCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    float DamageToApply = 0.f;

    if (Montage == NormalAttackMontage)
    {
        DamageToApply = NormalAttackDamage;
    }
    else if (Montage == StrongAttackMontage)
    {
        DamageToApply = StrongAttackDamage;
    }

    if (DamageToApply > 0.f)
    {
        DealDamageInRange(DamageToApply);
    }
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


void ABaseBossMonsterCharacter::StartBerserk()
{
    bIsBerserk = true;

    // 서버가 결정한 Berserk 시작을 클라이언트 전체에 알림
    Multicast_StartBerserk();
}


void ABaseBossMonsterCharacter::EndBerserk()
{
    if (!bIsBerserk)
        return;

    bIsBerserk = false;
    Multicast_EndBerserk();
}


void ABaseBossMonsterCharacter::OnRep_IsBerserk()
{
    if (bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Berserk] 클라이언트: Berserk 시작"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Berserk] 클라이언트: Berserk 종료"));
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
    // (A) 남은 단서가 없으면 종료
    if (RemainingClueClasses.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnRandomClue] 남은 단서 없음 → 스폰 종료"));
        return;
    }

    // (B) 랜덤 클래스 선택
    int32 Index = FMath::RandRange(0, RemainingClueClasses.Num() - 1);
    TSubclassOf<AActor> ClueClass = RemainingClueClasses[Index];
    if (!ClueClass)
    {
        RemainingClueClasses.RemoveAt(Index);
        return;
    }

    // (C) 보스 기준 X/Y 랜덤 오프셋
    const float OffsetF = FMath::RandRange(-200.f, 200.f);
    const float OffsetR = FMath::RandRange(-200.f, 200.f);
    FVector BossLoc = GetActorLocation();
    FVector SpawnXY = BossLoc
        + GetActorForwardVector() * OffsetF
        + GetActorRightVector() * OffsetR;

    UWorld* World = GetWorld();
    if (!World) return;

    // (D) 바닥 레이캐스트 ↓
    FHitResult Hit;
    FVector TraceStart = SpawnXY + FVector(0.f, 0.f, 500.f);
    FVector TraceEnd = SpawnXY + FVector(0.f, 0.f, -1000.f);
    FCollisionQueryParams Params(NAME_None, false, this);

    float SpawnZ = BossLoc.Z; // fallback
    if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        SpawnZ = Hit.Location.Z;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnRandomClue] 바닥 레이캐스트 실패 → 기본 Z 사용: %f"), SpawnZ);
    }

    FVector SpawnLoc = FVector(SpawnXY.X, SpawnXY.Y, SpawnZ);
    FRotator SpawnRot = GetActorRotation();

    // (E) 스폰 파라미터
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // (F) 액터 스폰
    AActor* NewClue = World->SpawnActor<AActor>(ClueClass, SpawnLoc, SpawnRot, SpawnParams);
    if (NewClue)
    {
        UE_LOG(LogTemp, Log, TEXT("[SpawnRandomClue] Spawn 성공: %s at %s"),
            *NewClue->GetName(), *SpawnLoc.ToCompactString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[SpawnRandomClue] Spawn 실패: %s"), *GetNameSafe(ClueClass));
    }

    // (G) 목록에서 제거하고 다음 예약
    RemainingClueClasses.RemoveAt(Index);
    if (RemainingClueClasses.Num() > 0)
    {
        float Delay = FMath::RandRange(ClueSpawnIntervalMin, ClueSpawnIntervalMax);
        GetWorldTimerManager().SetTimer(ClueTimerHandle, this,
            &ABaseBossMonsterCharacter::SpawnRandomClue,
            Delay, false);
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

    void ABaseBossMonsterCharacter::DealDamageInRange(float DamageAmount)
{
    FVector Origin = GetActorLocation();
    float Radius = AttackRange;

    // (옵션) 디버깅용 범위 시각화
    DrawDebugSphere(GetWorld(), Origin, Radius, 12, FColor::Red, false, 1.0f);

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
