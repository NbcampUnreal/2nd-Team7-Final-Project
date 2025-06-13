#include "AI/LCBossLich.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"

ALCBossLich::ALCBossLich()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ALCBossLich::BeginPlay()
{
    Super::BeginPlay();

    // 언데드 소환 타이머
    if (UndeadClasses.Num())
    {
        GetWorldTimerManager().SetTimer(
            UndeadSpawnTimerHandle,
            this, &ALCBossLich::SpawnUndeadMinion,
            UndeadSpawnInterval, true
        );
    }
    // 마나 파동 타이머
    GetWorldTimerManager().SetTimer(
        ManaPulseTimerHandle,
        this, &ALCBossLich::ExecuteManaPulse,
        ManaPulseInterval, true
    );
}

void ALCBossLich::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateRage(DeltaTime);
}

void ALCBossLich::UpdateRage(float DeltaSeconds)
{
    Super::UpdateRage(DeltaSeconds);

    // 살아있는 언데드 수만큼 초당 Rage 증가
    int32 Count = 0;
    for (auto& Cls : UndeadClasses)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), Cls, Found);
        Count += Found.Num();
    }
    if (Count > 0)
    {
        AddRage(UndeadRagePerSecond * Count * DeltaSeconds);
    }

    // Phylactery 파괴 확인 → 즉시 Berserk
    CheckPhylactery();

    // 체력 절반 이하에서 Death Nova 1회
    if (!bHasUsedDeathNova &&  Rage <= DeathNovaThreshold)
    {
        bHasUsedDeathNova = true;
        ExecuteDeathNova();
    }
}

void ALCBossLich::AddRage(float Amount)
{
    Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);
    if (Rage >= MaxRage && !bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Lich] Rage MAX → Berserk 진입"));
        EnterBerserkState();  // 영구 Berserk
    }
}

void ALCBossLich::SpawnUndeadMinion()
{
    if (!HasAuthority() || UndeadClasses.Num() == 0) return;
    int32 idx = FMath::RandRange(0, UndeadClasses.Num() - 1);
    TSubclassOf<APawn> Cls = UndeadClasses[idx];
    if (!Cls) return;

    FVector Loc = GetActorLocation()
        + FMath::RandPointInBox(FBox(FVector(-500, -500, 0), FVector(500, 500, 0)));
    FActorSpawnParameters P; P.Owner = this;
    if (GetWorld()->SpawnActor<APawn>(Cls, Loc, FRotator::ZeroRotator, P))
    {
        AddRage(UndeadRagePerSecond); // 즉시 보너스
    }
}

void ALCBossLich::ExecuteManaPulse()
{
    if (!HasAuthority()) return;
    FVector Origin = GetActorLocation();
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(ManaPulseRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, Origin, Origin,
        FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& H : Hits)
            if (ACharacter* C = Cast<ACharacter>(H.GetActor()))
                UGameplayStatics::ApplyDamage(C, ManaPulseDamage,
                    GetController(), this, nullptr);
    }
}

bool ALCBossLich::RequestAttack(float TargetDistance)
{
    if (!HasAuthority()) return false;
    const float Now = GetWorld()->GetTimeSeconds();

    // TargetActor 블랙보드 갱신
    AAIController* AC = Cast<AAIController>(GetController());
    AActor* Target = AC && AC->GetBlackboardComponent()
        ? Cast<AActor>(AC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")))
        : nullptr;

    struct FEntry { float W; TFunction<void()> A; };
    TArray<FEntry> L;

    // Arcane Bolt
    if (Target && TargetDistance <= ArcaneBoltRange
        && Now - LastArcaneBoltTime >= ArcaneBoltCooldown)
    {
        L.Add({ 3.f, [this,Now,Target]() {
            LastArcaneBoltTime = Now;
            UE_LOG(LogTemp,Log,TEXT("[Lich] ArcaneBolt → %s"),*Target->GetName());
            ExecuteArcaneBolt(Target);
        } });
    }
    // Soul Bind
    if (Target && TargetDistance <= SoulBindRange
        && Now - LastSoulBindTime >= SoulBindCooldown)
    {
        L.Add({ 2.f, [this,Now,Target]() {
            LastSoulBindTime = Now;
            UE_LOG(LogTemp,Log,TEXT("[Lich] SoulBind → %s"),*Target->GetName());
            ExecuteSoulBind(Target);
        } });
    }
    // Soul Absorb
    L.Add({ 1.f, [this,Target]() {
        if (Target) {
            UE_LOG(LogTemp,Log,TEXT("[Lich] SoulAbsorb → %s"),*Target->GetName());
            ExecuteSoulAbsorb(Target);
        }
    } });

    // 랜덤 실행
    float Sum = 0; for (auto& e : L) Sum += e.W;
    if (Sum <= 0) return false;
    float Pick = FMath::FRandRange(0.f, Sum), Acc = 0;
    for (auto& e : L) {
        Acc += e.W;
        if (Pick <= Acc) { e.A(); return true; }
    }
    return false;
}

void ALCBossLich::ExecuteArcaneBolt(AActor* Target)
{
    UGameplayStatics::ApplyDamage(Target, NormalAttackDamage,
        GetController(), this, nullptr);
}

void ALCBossLich::ExecuteSoulBind(AActor* Target)
{
    if (ACharacter* C = Cast<ACharacter>(Target))
    {
        C->DisableInput(nullptr);
        FTimerHandle T; GetWorldTimerManager().SetTimer(
            T, [C]() { if (C) C->EnableInput(nullptr); },
            3.f, false);
    }
}

void ALCBossLich::ExecuteSoulAbsorb(AActor* Target)
{
    UGameplayStatics::ApplyDamage(Target, SoulAbsorbDamage,
        GetController(), this, nullptr);
    UGameplayStatics::ApplyDamage(this, -SoulAbsorbHeal,
        GetController(), this, nullptr);
}

void ALCBossLich::ExecuteDeathNova()
{
    if (!HasAuthority()) return;
    FVector O = GetActorLocation();
    TArray<FHitResult> Hits;
    FCollisionShape S = FCollisionShape::MakeSphere(DeathNovaRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, O, O, FQuat::Identity, ECC_Pawn, S))
        for (auto& H : Hits)
            if (ACharacter* C = Cast<ACharacter>(H.GetActor()))
                UGameplayStatics::ApplyDamage(C, DeathNovaDamage,
                    GetController(), this, nullptr);

    UE_LOG(LogTemp, Warning, TEXT("[Lich] DeathNova executed"));
}

void ALCBossLich::CheckPhylactery()
{
    if (bIsBerserk || !HasAuthority()) return;
    //if (!IsValid(PhylacteryActor) || PhylacteryActor->IsPendingKill())
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("[Lich] Phylactery destroyed → Berserk"));
    //    EnterBerserkState();
    //}
}

void ALCBossLich::OnRep_IsBerserk()
{
    Super::OnRep_IsBerserk();
    if (bIsBerserk)
        Multicast_StartBerserk();
}