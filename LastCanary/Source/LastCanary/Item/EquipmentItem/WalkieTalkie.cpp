#include "Item/EquipmentItem/WalkieTalkie.h"
#include "WalkieTalkie.h"
#include "Character/BaseCharacter.h"

#include "Net/UnrealNetwork.h"

#include "LastCanary.h"

void AWalkieTalkie::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AWalkieTalkie::UseItem()
{
    Super::UseItem();

    if (AActor* OwnerActor = GetOwner())
    {
        if (ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor))
        {
            if (APlayerController* PC = OwnerCharacter->GetController<APlayerController>())
            {
                Server_UseWalkieTalkie(PC);
            }
        }
    }
}

void AWalkieTalkie::Server_UseWalkieTalkie_Implementation(APlayerController* UserController)
{
    if (!UserController) return;

    if (UserController->IsLocalController())
    {
        // 리슨 서버 (호스트)에서 실행
        StartWalkieTalkie();
        bUseWalkie = !bUseWalkie; // 로컬 상태 토글

        UE_LOG(LogTemp, Warning, TEXT("Host WalkieTalkie - bUseWalkie: %s"),
            bUseWalkie ? TEXT("true") : TEXT("false"));
    }
    else
    {
        // 원격 클라이언트에게만 RPC 전송
        Client_StartWalkieTalkie();
    }
}

void AWalkieTalkie::Server_StopWalkieTalkie_Implementation()
{
    if (!bUseWalkie) return;

    // 워키토키 사용 상태 비활성화
    bUseWalkie = false;

    // 클라이언트에 중지 알림
    Client_StopWalkieTalkie();
}

void AWalkieTalkie::Client_StartWalkieTalkie_Implementation()
{
    StartWalkieTalkie();
    bUseWalkie = !bUseWalkie; // 로컬 상태 토글

    UE_LOG(LogTemp, Warning, TEXT("Client WalkieTalkie - bUseWalkie: %s"),
        bUseWalkie ? TEXT("true") : TEXT("false"));
}

void AWalkieTalkie::Client_StopWalkieTalkie_Implementation()
{
    StopWalkieTalkie();
}

void AWalkieTalkie::StopWalkieTalkie()
{
    if (bUseWalkie && IsOwnedByLocalPlayer())
    {
        StartWalkieTalkie(); // 토글로 끄기
        bUseWalkie = false;

        UE_LOG(LogTemp, Warning, TEXT("StopWalkieTalkie - bUseWalkie set to false"));
    }
}

bool AWalkieTalkie::IsOwnedByLocalPlayer() const
{
    if (AActor* OwnerActor = GetOwner())
    {
        if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
        {
            return OwnerPawn->IsLocallyControlled();
        }
    }
    return false;
}