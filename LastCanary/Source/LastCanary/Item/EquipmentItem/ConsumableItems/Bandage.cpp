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

	// 아이템의 사운드 타입에 따라 재생
	PlaySoundByType();

	//1. 총 회복량 2. 소요 시간
	OwnerCharacter->StartHealing(HealingAmount, HealDuration);

	// 내구도 감소
	Durability--;
	LOG_Item_WARNING(TEXT("CurrentDurability : %0.f"), Durability);

	// 인벤토리에서 아이템 1개 소모
	UToolbarInventoryComponent* Inventory = OwnerCharacter->GetToolbarInventoryComponent();
	if (Inventory)
	{
		if (Inventory->TryDecreaseItem(ItemRowName, 1))
		{
			LOG_Item_WARNING(TEXT("붕대 사용 완료 - 1개 소모"));
		}
		else
		{
			LOG_Item_WARNING(TEXT("붕대 소모 실패"));
		}
	}
}
