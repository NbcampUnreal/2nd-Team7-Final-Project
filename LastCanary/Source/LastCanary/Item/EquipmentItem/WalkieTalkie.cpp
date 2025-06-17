#include "Item/EquipmentItem/WalkieTalkie.h"
#include "WalkieTalkie.h"
#include "Character/BaseCharacter.h"
#include "LastCanary.h"

void AWalkieTalkie::UseItem()
{
	Super::UseItem();

    // 아이템 소유자의 PlayerController 획득
    if (AActor* OwnerActor = GetOwner())
    {
        if (ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor))
        {
            if (APlayerController* PC = OwnerCharacter->GetController<APlayerController>())
            {
                // 서버에서 워키토키 사용 처리
                Server_UseWalkieTalkie(PC);
            }
        }
    }
}

void AWalkieTalkie::Server_UseWalkieTalkie_Implementation(APlayerController* UserController)
{
    if (!UserController)
    {
        UE_LOG(LogTemp, Warning, TEXT("UserController is null"));
        return;
    }

    // 해당 플레이어 클라이언트에서만 워키토키 시작
    if (UserController->IsLocalController())
    {
        // 서버에서 실행 중인 경우 (리슨 서버)
        StartWalkieTalkie();
    }
    else
    {
        // 원격 클라이언트에게 RPC 전송
        Client_StartWalkieTalkie();
    }
}

void AWalkieTalkie::Client_StartWalkieTalkie_Implementation()
{
    StartWalkieTalkie();
}
