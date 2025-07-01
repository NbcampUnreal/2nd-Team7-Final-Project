#include "Item/EquipmentItem/WalkieTalkie.h"
#include "WalkieTalkie.h"
#include "Character/BaseCharacter.h"

#include "Net/UnrealNetwork.h"

#include "LastCanary.h"

void AWalkieTalkie::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWalkieTalkie, bUseWalkie);
}

void AWalkieTalkie::UseItem()
{
	Super::UseItem();

    StartWalkieTalkie();

    bUseWalkie = !bUseWalkie;
}

void AWalkieTalkie::Server_UseWalkieTalkie_Implementation()
{
    // 워키토키 사용 상태 활성화
    bUseWalkie = true;

    // 클라이언트에 시작 알림
    Client_StartWalkieTalkie();
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
}

void AWalkieTalkie::Client_StopWalkieTalkie_Implementation()
{
    StopWalkieTalkie();
}

void AWalkieTalkie::StopWalkieTalkie()
{
    if (bUseWalkie)
    {
        StartWalkieTalkie();
        bUseWalkie = false;
    }
}
