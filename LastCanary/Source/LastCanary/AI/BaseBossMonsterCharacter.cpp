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
    // (A) 남은 단서가 아무것도 없으면 더 이상 스폰하지 않음
    if (RemainingClueClasses.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnRandomClue] 남은 단서 없음 → 스폰 종료"));
        return;
    }

    // (B) RemainingClueClasses에서 랜덤으로 하나 선택
    int32 Index = FMath::RandRange(0, RemainingClueClasses.Num() - 1);
    TSubclassOf<AActor> ChosenClass = RemainingClueClasses[Index];
    if (!ChosenClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnRandomClue] 선택된 Clue 클래스가 유효하지 않음: Index=%d"), Index);

        // 선택된 항목이 null이면 목록에서 제거만 하고 다음 스폰 예약
        RemainingClueClasses.RemoveAt(Index);
    }
    else
    {
        // (C) 보스 주변에 랜덤 오프셋 계산
        FVector BossLoc = GetActorLocation();
        FVector Forward = GetActorForwardVector();
        FVector Right = GetActorRightVector();
        FVector Up = FVector::UpVector;

        const float OffsetForward = FMath::RandRange(-50.f, 50.f);
        const float OffsetRight = FMath::RandRange(-100.f, 100.f);
        const float OffsetUp = FMath::RandRange(-20.f, 20.f);

        FVector SpawnLoc = BossLoc
            + Forward * OffsetForward
            + Right * OffsetRight
            + Up * OffsetUp;
        FRotator SpawnRot = GetActorRotation();

        // (D) 스폰 파라미터
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // (E) 실제 액터 스폰
        AActor* NewClue = GetWorld()->SpawnActor<AActor>(
            ChosenClass, SpawnLoc, SpawnRot, SpawnParams);

        if (NewClue)
        {
            UE_LOG(LogTemp, Log,
                TEXT("[SpawnRandomClue] %s 을(를) 인덱스 %d 로 스폰 성공 (위치=%s)"),
                *NewClue->GetName(),
                Index,
                *SpawnLoc.ToCompactString());
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("[SpawnRandomClue] 스폰 실패: 클래스=%s, 인덱스=%d, 위치=%s"),
                *GetNameSafe(ChosenClass),
                Index,
                *SpawnLoc.ToCompactString());
        }

        // (F) 방금 스폰한 클래스는 “다시 스폰하지 않도록” 목록에서 제거
        RemainingClueClasses.RemoveAt(Index);
    }

    // (G) 다음 스폰 예약: 아직 남은 단서가 있으면 랜덤 간격으로 재호출
    if (RemainingClueClasses.Num() > 0)
    {
        float NextDelay = FMath::RandRange(ClueSpawnIntervalMin, ClueSpawnIntervalMax);
        GetWorldTimerManager().SetTimer(
            ClueTimerHandle,
            this,
            &ABaseBossMonsterCharacter::SpawnRandomClue,
            NextDelay,
            false
        );
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
