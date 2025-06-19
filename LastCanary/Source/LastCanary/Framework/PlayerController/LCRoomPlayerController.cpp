#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/GameMode/LCRoomGameMode.h"
#include "Framework/GameState/LCGameState.h"
#include "Character/BasePlayerState.h"
#include "Framework/Manager/LCCheatManager.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"

#include "Actor/LCDroneDelivery.h"
#include "Item/ItemBase.h"

#include "UI/UIElement/ResultMenu.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/PackageName.h"

#include "Blueprint/UserWidget.h"

#include "EnhancedInputComponent.h"

ALCRoomPlayerController::ALCRoomPlayerController()
{

}

void ALCRoomPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

}

void ALCRoomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			UIManager->SetUIContext(ELCUIContext::Room);
		}
	}
}


void ALCRoomPlayerController::Server_ShowShopWidget_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		ULCGameManager* GM = GameInstance->GetSubsystem<ULCGameManager>();
		Client_ShowShopWidget(GM->GetGold());
	}
}

void ALCRoomPlayerController::Client_ShowShopWidget_Implementation(int Gold)
{
	LCUIManager->ShowShopPopup(Gold);
}

void ALCRoomPlayerController::Server_RequestPurchase_Implementation(const TArray<FItemDropData>& DropList)
{
	if (DropList.IsEmpty())
	{
		LOG_Frame_WARNING(TEXT("Server_RequestPurchase called with empty DropList"));
		return;
	}
	if (!DroneDeliveryClass)
	{
		LOG_Frame_WARNING(TEXT("Server_RequestPurchase called with invalid DroneDeliveryClass."));
		return;
	}

	ABasePlayerState* PS = GetPlayerState<ABasePlayerState>();
	if (PS == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Server_RequestPurchase called with invalid PlayerState."));
		return;
	}

	// 게임 인스턴스 서브시스템에서 ItemDataTable 가져오기
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Server_RequestPurchase called with invalid World."));
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (GameInstance == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Server_RequestPurchase called with invalid GameInstance."));
		return;
	}

	ULCGameInstanceSubsystem* GISubsystem = GameInstance->GetSubsystem<ULCGameInstanceSubsystem>();
	if (GISubsystem == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Server_RequestPurchase called with invalid GISubsystem."));
		return;
	}

	ULCGameManager* LCGM = GameInstance->GetSubsystem<ULCGameManager>();
	if (LCGM == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Server_RequestPurchase called with invalid GISubsystem."));
		return;
	}

	UDataTable* ItemDataTable = GISubsystem->GetItemDataTable();
	if (ItemDataTable == nullptr)
	{
		LOG_Frame_WARNING(TEXT("ItemDataTable is NULL in Server_RequestPurchase"));
		return;
	}

	// 가격 계산
	const FString ContextStr(TEXT("Purchase Validation"));
	int32 TotalPrice = 0;
	TArray<TPair<FString, int32>> ItemArray;

	for (const FItemDropData& DropData : DropList)
	{
		if (DropData.ItemClass == nullptr)
		{
			LOG_Frame_WARNING(TEXT("DropData.ItemClass is NULL"));
			continue;
		}

		for (const FName& RowName : ItemDataTable->GetRowNames())
		{
			if (const FItemDataRow* Row = ItemDataTable->FindRow<FItemDataRow>(RowName, ContextStr))
			{
				if (Row->ItemID == DropData.ItemID)
				{
					TotalPrice += Row->ItemPrice * DropData.Count;
					FString Reson = FString::Printf(TEXT("아이템 %s %d개 구매!"), *Row->ItemName.ToString(), DropData.Count);
					ItemArray.Add(TPair<FString, int32>(Reson, Row->ItemPrice * DropData.Count));
					break;
				}
			}
		}
	}

	// 골드 차감 조건 확인
	if (PS->GetTotalGold() < TotalPrice)
	{
		LOG_Frame_WARNING(TEXT("구매 실패: 골드 부족 (보유: %d, 필요: %d)"), PS->GetTotalGold(), TotalPrice);
		return;
	}

	for (const TPair<FString, int32>& Item : ItemArray)
	{
		LOG_Frame_WARNING(TEXT("구매 아이템: %s"), *Item.Key);
		LCGM->UpdateGold(Item.Key, -Item.Value);
	}

	// 골드 차감
	PS->Server_SpendGold(TotalPrice);

	// 드론 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;

	FVector SpawnLoc = FVector::ZeroVector;

	ALCDroneDelivery* Drone = GetWorld()->SpawnActor<ALCDroneDelivery>(DroneDeliveryClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (Drone)
	{
		Drone->ItemsToDrop = DropList;
		Drone->StartDelivery();
	}
}
