#include "AI/LCBossVampire.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/DamageType.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ALCBossVampire::ALCBossVampire()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ALCBossVampire::BeginPlay()
{
    Super::BeginPlay();

    // Ÿ�̸� ����
    GetWorldTimerManager().SetTimer(BatSwarmTimerHandle, this, &ALCBossVampire::ExecuteBatSwarm, BatSwarmInterval, true);
    GetWorldTimerManager().SetTimer(GazeTimerHandle, this, &ALCBossVampire::ExecuteNightmareGaze, GazeInterval, true);
}

void ALCBossVampire::ExecuteBatSwarm()
{
    if (!HasAuthority() || !BatSwarmClass) return;
    for (int32 i = 0; i < BatCount; ++i)
    {
        FVector Dir = FMath::VRand();
        Dir.Z = 0.5f;
        FVector SpawnLoc = GetActorLocation() + Dir * 300.f + FVector(0, 0, 100);
        GetWorld()->SpawnActor<AActor>(BatSwarmClass, SpawnLoc, Dir.Rotation());
    }
}

void ALCBossVampire::EnterMistForm()
{
    if (!HasAuthority() || !bCanUseMist) return;
    bIsMistForm = true;
    Multicast_StartMistForm();

    // ���� ����(�ǰ� �鿪)
    SetCanBeDamaged(false);

    // Mist ���� ����
    GetWorldTimerManager().SetTimer(MistCooldownHandle, this, &ALCBossVampire::EndMistForm, MistDuration, false);
    bCanUseMist = false;
}

void ALCBossVampire::EndMistForm()
{
    bIsMistForm = false;
    OnRep_MistForm();

    // ��ٿ� �� ���� ����
    GetWorldTimerManager().SetTimer(MistResetTimerHandle, this, &ALCBossVampire::ResetMist, MistCooldown, false);
}

void ALCBossVampire::ResetMist()
{
    bCanUseMist = true;
}

void ALCBossVampire::OnRep_MistForm()
{
    if (bIsMistForm)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Enter Mist Form"));
        // Ŭ���̾�Ʈ�� ����Ʈ/���� �߰�
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Exit Mist Form"));
        SetCanBeDamaged(true);
    }
}

void ALCBossVampire::Multicast_StartMistForm_Implementation()
{
    OnRep_MistForm();
}

void ALCBossVampire::ExecuteNightmareGaze()
{
    if (!HasAuthority()) return;

    // �ݰ� �� �÷��̾� �������� Fear ���� (�̵��ӵ� ����)
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(GazeRadius);
    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits,
        GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, Sphere);

    if (bHit)
    {
        for (auto& Hit : Hits)
        {
            if (APawn* P = Cast<APawn>(Hit.GetActor()))
            {
                if (P->IsPlayerControlled())
                {
                    ACharacter* Ch = Cast<ACharacter>(P);
                    if (Ch)
                    {
                        float OrigSpeed = Ch->GetCharacterMovement()->MaxWalkSpeed;
                        Ch->GetCharacterMovement()->MaxWalkSpeed = OrigSpeed * 0.5f;

                        // 5�� �� ���� �ӵ��� ����
                        FTimerHandle RestoreHandle;
                        FTimerDelegate RestoreDel = FTimerDelegate::CreateLambda([Ch, OrigSpeed]()
                            {
                                Ch->GetCharacterMovement()->MaxWalkSpeed = OrigSpeed;
                            });
                        GetWorldTimerManager().SetTimer(RestoreHandle, RestoreDel, 5.f, false);

                        UE_LOG(LogTemp, Log, TEXT("[Vampire] %s is Feared"), *P->GetName());
                    }
                }
            }
        }
    }
}


void ALCBossVampire::OnRep_Bloodlust()
{
    if (bIsBloodlust)
    {
        // ���� ��ٿ� ���ҡ�Ȯ�� ����, �̵��ӵ� ���
        StrongAttackCooldown *= 0.5f;
        StrongAttackChance = FMath::Clamp(StrongAttackChance * 2.f, 0.f, 1.f);
        GetCharacterMovement()->MaxWalkSpeed *= 1.2f;
        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Eternal Bloodlust activated"));
    }
}

void ALCBossVampire::Multicast_StartBloodlust_Implementation()
{
    OnRep_Bloodlust();
}

void ALCBossVampire::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALCBossVampire, bIsMistForm);
    DOREPLIFETIME(ALCBossVampire, bIsBloodlust);
}
