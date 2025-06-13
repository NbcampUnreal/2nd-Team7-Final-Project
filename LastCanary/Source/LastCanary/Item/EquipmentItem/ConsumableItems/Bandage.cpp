#include "Item/EquipmentItem/ConsumableItems/Bandage.h"
#include "Character/BaseCharacter.h"
#include "Character/BasePlayerState.h"

ABandage::ABandage()
{
	CurrentDurability = MaxDurability; // 초기 내구도 설정
}

void ABandage::UseItem()
{
	Super::UseItem();

	if (!HasAuthority())
	{
		return;
	}
	
	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	ABasePlayerState* PS = OwnerCharacter->GetPlayerState<ABasePlayerState>();
	if (!IsValid(PS))
	{
		return;
	}

	if (PS->GetHP() >= PS->MaxHP)
	{
		// 이미 최대 체력에 도달했을 때
		LOG_Item_WARNING(TEXT("체력이 가득 차 있어 사용 노노~"));
		return;
	}

	if (CurrentDurability <= 0)
	{
		// 내구도가 0 이하일 때
		LOG_Item_WARNING(TEXT("붕대의 내구도가 소진되어 사용 노노~"));
		return;
	}

	//1. 총 회복량 2. 소요 시간
	OwnerCharacter->StartHealing(HealingAmount, HealDuration);

	// 내구도 감소
	CurrentDurability--;
}

