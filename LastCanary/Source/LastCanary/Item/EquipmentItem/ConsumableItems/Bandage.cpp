#include "Item/EquipmentItem/ConsumableItems/Bandage.h"
#include "Character/BaseCharacter.h"
#include "Character/BasePlayerState.h"
#include "Inventory/ToolbarInventoryComponent.h"

ABandage::ABandage()
{
	Durability = MaxDurability; // 초기 내구도 설정
	LOG_Item_WARNING(TEXT("CurrentDurability : %0.f"), Durability);
}

void ABandage::UseItem()
{
	Super::UseItem();
	LOG_Item_WARNING(TEXT("CurrentDurability : %0.f"), Durability);
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

	// 사용 시작 소리 출력(false로 값을 입력할 시 사용 종료 시 소리를 출력합니다.)
	PlayItemUseSound(true);

	//1. 총 회복량 2. 소요 시간
	OwnerCharacter->StartHealing(HealingAmount, HealDuration);

	// 내구도 감소
	Durability--;
	LOG_Item_WARNING(TEXT("CurrentDurability : %0.f"), Durability);

	if (Durability <= 0)
	{
		// 내구도가 0 이하일 때
		if (TryRemoveFromInventory())
		{
			LOG_Item_WARNING(TEXT("붕대 제거 성공"));
		}
		else
		{
			LOG_Item_WARNING(TEXT("붕대 제거 실패"));
		}
		LOG_Item_WARNING(TEXT("붕대의 내구도가 소진되어 사용 노노~"));
		return;
	}
}
