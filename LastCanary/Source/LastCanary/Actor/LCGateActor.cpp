#include "Actor/LCGateActor.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Character/BasePlayerController.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/PlayerController/LCInGamePlayerController.h"
#include "Character/BaseCharacter.h"
#include "Character/BasePlayerState.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"

#include "Net/UnrealNetwork.h"

#include "LastCanary.h"

ALCGateActor::ALCGateActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	TravelType = EGateTravelType::ToInGame;
}

void ALCGateActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALCGateActor::Interact_Implementation(APlayerController* Controller)
{
	LOG_Frame_WARNING(TEXT("Gate actor %s is being interacted with."), *GetName());

	if (!IsValid(Controller))
	{
		return;
	}
	switch (TravelType)
	{
	case EGateTravelType::ToBaseCamp:
	{
		LOG_Frame_WARNING(TEXT("Travel To BaseCamp"));
		ReturnToBaseCamp(Controller);
		break;
	}
	case EGateTravelType::ToInGame:
	{
		LOG_Frame_WARNING(TEXT("Play Game Level"));
		IntoGameLevel(Controller);
		break;
	}
	}
}

void ALCGateActor::ReturnToBaseCamp(APlayerController* Controller)
{
	if (!IsValid(Controller))
	{
		LOG_Frame_WARNING(TEXT("컨트롤러가 유효하지 않음"));
		return;
	}
	//ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(Controller);
	//if (!IsValid(RoomPC))
	//{
	//	LOG_Frame_WARNING(TEXT("컨트롤러 캐스팅이 실패함"));
	//	return;
	//}
	ALCInGamePlayerController* InGamePC = Cast<ALCInGamePlayerController>(Controller);
	if (!IsValid(InGamePC))
	{
		LOG_Frame_WARNING(TEXT("컨트롤러 캐스팅이 실패함"));
		return;
	}
	ULCGameInstance* LCGameInstance = GetGameInstance<ULCGameInstance>();
	if (!IsValid(LCGameInstance))
	{
		LOG_Frame_WARNING(TEXT("게임 인스턴스가 유효하지 않음"));
		return;
	}
	ULCGameInstanceSubsystem* LCGameInstanceSubsystem = LCGameInstance->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!IsValid(LCGameInstanceSubsystem))
	{
		LOG_Frame_WARNING(TEXT("게임 인스턴스 서브시스템이 유효하지 않음"));
		return;
	}
	ABasePlayerState* PlayerState = InGamePC->GetPlayerState<ABasePlayerState>();
	if (!IsValid(PlayerState))
	{
		LOG_Frame_WARNING(TEXT("플레이어 스테이트를 가져오지 못함"));
		return;
	}
	// 아이템 ID 복사
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(InGamePC->GetPawn());
	if (!IsValid(PlayerCharacter))
	{
		LOG_Frame_WARNING(TEXT("플레이어 캐릭터가 유효하지 않음"));
		return;
	}

	// 캐릭터에서 사망처리 및 탐사결과 보고서 제출 후 처리
	//InGamePC->Server_MarkPlayerAsEscaped();


	if (!HasAuthority()) // 서버에서만 처리
	{
		LOG_Frame_WARNING(TEXT("서버가 아님"));
		InGamePC->OnExitGate();
		return;
	}

	// 자원 수집 기록
	UDataTable* ItemTable = LCGameInstanceSubsystem->GetItemDataTable();
	if (!IsValid(ItemTable))
	{
		return;
	}

	for (int32 ItemID : PlayerState->AquiredItemIDs)
	{
		FName RowName = *FString::Printf(TEXT("Item_%d"), ItemID);
		const FItemDataRow* Row = ItemTable->FindRow<FItemDataRow>(RowName, TEXT("Gate Resource Parse"));

		if (Row && Row->bIsResourceItem)
		{
			PlayerState->AddCollectedResource(RowName);
			LOG_Frame_WARNING(TEXT("자원 기록: %s → %d개 누적"), *RowName.ToString(), PlayerState->CollectedResourceMap[RowName]);
		}
	}

	InGamePC->OnExitGate();

	//if (RoomPC->HasAuthority())
	//{
	//	RoomPC->Server_MarkPlayerAsEscaped_Implementation();
	//	if (ALCGameState* GS = GetWorld()->GetGameState<ALCGameState>())
	//	{
	//		//GS->MarkPlayerAsEscaped(RoomPC->PlayerState);
	//	}
	//}
	//else
	//{
	//	RoomPC->Server_MarkPlayerAsEscaped();
	//}

	/*
	ABasePlayerController* BasePlayerController = Cast<ABasePlayerController>(Controller);
	BasePlayerController->OnExitGate();
	*/

	// TODO : 탈출, 체크리스트 띄우고 전부 작성하면 결과 UI-> 호스트가 버튼 눌러서 베이스캠프로 이동
	// 사망->시체 스켈레탈메시남고->관전(컨트롤러)
	// 관전으로 넘기는 함수
	// 탈출시 PS로 아이템 아이디넘김 타이머로 캐릭터 Destroy
}

void ALCGateActor::IntoGameLevel(APlayerController* Controller)
{
	if (!IsValid(Controller))
	{
		return;
	}
	ALCGameMode* LCGM = Cast<ALCGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(LCGM))
	{
		return;
	}
	ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!IsValid(GISubsystem))
	{
		return;
	}
	//if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(Controller))
	//{
	//	if (ABasePlayerState* PlayerState = RoomPC->GetPlayerState<ABasePlayerState>())
	//	{
	//		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(RoomPC->GetPawn());
	//		PlayerState->AquiredItemIDs.Append(PlayerCharacter->GetToolbarInventoryComponent()->GetInventoryItemIDs());
	//	}
	//}

	if (HasAuthority() == false)
	{
		return;
	}
	else
	{
		if (LCGM)
		{
			if (LCGM->IsAllPlayersReady() == false)
			{
				LOG_Server_ERROR(TEXT("All Client is Not Ready!!"));
				return;
			}
		}
		else
		{
			LOG_Server_ERROR(TEXT("Cast Fail GameMode : Not Server!!"));
			return;
		}

		LOG_Server_WARNING(TEXT("All Client is Ready!! Try To Server Travel"));

		// 랜덤 맵 지정이 아직 안 된 경우
		if (TargetMapID == 0)
		{
			// MapDataTable에서 인게임 맵만 추출
			if (UDataTable* MapTable = GISubsystem->GetMapDataTable())
			{
				TArray<FMapDataRow*> AllMaps;
				static const FString Ctx = TEXT("GateActor-SelectRandomMap");
				MapTable->GetAllRows(Ctx, AllMaps);

				TArray<int32> InGameMapIDs;

				for (const FMapDataRow* Row : AllMaps)
				{
					if (Row && Row->MapInfo.MapName != TEXT("BaseCamp"))
					{
						InGameMapIDs.Add(Row->MapID);
					}
				}

				if (InGameMapIDs.Num() > 0)
				{
					int32 RandomIdx = FMath::RandRange(0, InGameMapIDs.Num() - 1);
					TargetMapID = InGameMapIDs[RandomIdx];
					LOG_Frame_WARNING(TEXT("Gate assigned random InGame TargetMapID: %d"), TargetMapID);
				}
				else
				{
					LOG_Frame_WARNING(TEXT("No InGame maps found in MapDataTable."));
					return;
				}
			}
			else
			{
				LOG_Frame_WARNING(TEXT("MapDataTable is null in GameInstanceSubsystem."));
				return;
			}
		}
	}
	GISubsystem->ChangeLevelByMapID(TargetMapID);

	if (LCGM)
	{
		LCGM->ShowLoading();
	}
	else
	{
		LOG_Server_ERROR(TEXT("Base GameMode Is Null"));
		return;
	}

}

FString ALCGateActor::GetInteractMessage_Implementation() const
{
	if (IA_Interact == nullptr)
	{
		return TEXT("No Interact Key Assigned");
	}

	FString InteractKeyName = GetCurrentKeyNameForAction(IA_Interact);

	switch (TravelType)
	{
	case EGateTravelType::ToBaseCamp:
	{
		return FString::Printf(TEXT("Press [%s] to Use Gate"), *InteractKeyName);
	}
	case EGateTravelType::ToInGame:
	{
		if (HasAuthority())
		{
			return FString::Printf(TEXT("Press [%s] to Use Gate"), *InteractKeyName);
		}
		else
		{
			return TEXT("");
		}
	}
	default:
	{
		return TEXT("Unknown Gate Type");
	}
	}
}

FString ALCGateActor::GetCurrentKeyNameForAction(UInputAction* InputAction) const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!IsValid(PC))
	{
		return TEXT("Invalid");
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return TEXT("Invalid");
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(Subsystem))
	{
		return TEXT("Invalid");
	}
	const TArray<FEnhancedActionKeyMapping> Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();

	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (Mapping.Action == InputAction)
		{
			return Mapping.Key.GetDisplayName().ToString();
		}
	}
	return TEXT("Unbound");
}

void ALCGateActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALCGateActor, TravelType);
}