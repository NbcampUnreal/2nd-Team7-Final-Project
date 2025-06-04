#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/GameMode/LCRoomGameMode.h"
#include "Framework/GameState/LCGameState.h"
#include "Character/BasePlayerState.h"
#include "Framework/Manager/LCCheatManager.h"

#include "Actor/LCDroneDelivery.h"
#include "Item/ItemBase.h"

#include "UI/UIElement/RoomWidget.h"
#include "UI/UIElement/ResultMenu.h"

#include "Engine/World.h"
#include "Misc/PackageName.h"
#include "EngineUtils.h"

#include "UI/Manager/LCUIManager.h"
#include "Blueprint/UserWidget.h"
#include "DataType/SessionPlayerInfo.h"

#include "EnhancedInputComponent.h"
#include "LastCanary.h"

ALCRoomPlayerController::ALCRoomPlayerController()
{
	CheatClass = ULCCheatManager::StaticClass();
}

void ALCRoomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CreateRoomWidget();

	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			UIManager->SetUIContext(ELCUIContext::Room);
		}
	}
}

void ALCRoomPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	LOG_Frame_WARNING(TEXT("PostSeamlessTravel: Ensuring CheatManager is ready"));

	// 치트매니저 재초기화
	if (CheatManager == nullptr)
	{
		CheatManager = NewObject<ULCCheatManager>(this, CheatClass);
		CheatManager->InitCheatManager();
	}
}

void ALCRoomPlayerController::Client_UpdatePlayerList_Implementation(const TArray<FSessionPlayerInfo>& PlayerInfos)
{
	Super::Client_UpdatePlayerList_Implementation(PlayerInfos);

	UpdatePlayerList(PlayerInfos);
}

void ALCRoomPlayerController::UpdatePlayerList(const TArray<FSessionPlayerInfo>& PlayerInfos)
{
	if (IsValid(RoomWidgetInstance))
	{
		LOG_Frame_WARNING(TEXT("Try Update Player List!"));
		RoomWidgetInstance->UpdatePlayerLists(PlayerInfos);
	}
	else
	{
		LOG_Frame_WARNING(TEXT("Not Initialized Widget Instance!! Retry Update Info"));

		FTimerHandle TimerHandle;
		TWeakObjectPtr<ALCRoomPlayerController> WeakPtr(this);
		TArray<FSessionPlayerInfo> InfosCopy = PlayerInfos;

		GetWorld()->GetTimerManager().SetTimer
		(
			TimerHandle,
			[WeakPtr, InfosCopy]()
			{
				if (WeakPtr.IsValid())
				{
					if (WeakPtr->RoomWidgetInstance)
					{
						UE_LOG(LogTemp, Warning, TEXT("Update Lobby UI!!"));
						WeakPtr->UpdatePlayerList(InfosCopy);
					}
				}
			},
			RePeatRate,
			false
		);
	}
}

void ALCRoomPlayerController::Server_SetReady_Implementation(bool bIsReady)
{

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

	UDataTable* ItemDataTable = GISubsystem->GetItemDataTable();
	if (ItemDataTable == nullptr)
	{
		LOG_Frame_WARNING(TEXT("ItemDataTable is NULL in Server_RequestPurchase"));
		return;
	}

	// 가격 계산
	const FString ContextStr(TEXT("Purchase Validation"));
	int32 TotalPrice = 0;

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

void ALCRoomPlayerController::InitInputComponent()
{
	Super::InitInputComponent();

	if (IsValid(EnhancedInput))
	{
		if (RoomUIAction)
		{
			EnhancedInput->BindAction(RoomUIAction, ETriggerEvent::Started, this, &ALCRoomPlayerController::ToggleShowRoomWidget);
		}
	}

}

void ALCRoomPlayerController::CreateRoomWidget()
{
	if (IsLocalPlayerController())
	{
		if (RoomWidgetClass)
		{
			RoomWidgetInstance = CreateWidget<URoomWidget>(this, RoomWidgetClass);
			RoomWidgetInstance->CreatePlayerSlots();
			RoomWidgetInstance->AddToViewport();
			RoomWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
			bIsShowRoomUI = false;
		}
	}
}

void ALCRoomPlayerController::ToggleShowRoomWidget()
{
	bIsShowRoomUI = !bIsShowRoomUI;

	if (IsValid(RoomWidgetInstance))
	{
		if (bIsShowRoomUI)
		{
			//RoomWidgetInstance->AddToViewport(10);
			RoomWidgetInstance->SetVisibility(ESlateVisibility::Visible);
			FInputModeGameAndUI GameAndUIInputMode;
			SetInputMode(GameAndUIInputMode);
		}
		else
		{
			//RoomWidgetInstance->RemoveFromParent();
			RoomWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
			FInputModeGameOnly GameInputMode;
			SetInputMode(GameInputMode);
		}

		bShowMouseCursor = bIsShowRoomUI;
	}
}

void ALCRoomPlayerController::Client_NotifyResultReady_Implementation(const FChecklistResultData& ResultData)
{
	LOG_Frame_WARNING(TEXT("[Client] 결과 수신 → 결과 UI 출력 시작"));

	if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = GISubsystem->GetUIManager())
		{
			UIManager->ShowResultMenu();
			if (UResultMenu* Menu = UIManager->GetResultMenuClass())
			{
				Menu->SetChecklistResult(ResultData); 
			}
			else
			{
				LOG_Frame_WARNING(TEXT("[Client] GetCachedResultMenu가 null을 반환함"));
			}
		}
	}
}

void ALCRoomPlayerController::Client_StartChecklist_Implementation()
{
	for (TActorIterator<AChecklistManager> It(GetWorld()); It; ++It)
	{
		if (AChecklistManager* ChecklistManager = *It)
		{
			ChecklistManager->StartChecklist();
			break;
		}
	}
}

void ALCRoomPlayerController::Server_MarkPlayerAsEscaped_Implementation()
{
	LOG_Frame_WARNING(TEXT("== Server_MarkPlayerAsEscaped_Implementation Called =="));

	if (GetWorld()->GetGameState<ALCGameState>())
	{
		GetWorld()->GetGameState<ALCGameState>()->MarkPlayerAsEscaped(PlayerState);
	}
}

void ALCRoomPlayerController::Server_RequestSubmitChecklist_Implementation(const TArray<FChecklistQuestion>& PlayerAnswers)
{
	LOG_Frame_WARNING(TEXT("Server_RequestSubmitChecklist_Implementation called"));

	for (TActorIterator<AChecklistManager> It(GetWorld()); It; ++It)
	{
		if (AChecklistManager* Manager = *It)
		{
			LOG_Frame_WARNING(TEXT("ChecklistManager found → Submitting"));
			Manager->Server_SubmitChecklist(this, PlayerAnswers); 
			return;
		}
	}

	LOG_Frame_WARNING(TEXT("ChecklistManager not found on server"));
}
