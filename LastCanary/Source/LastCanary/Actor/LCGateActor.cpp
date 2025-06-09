#include "Actor/LCGateActor.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Character/BasePlayerController.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Character/BaseCharacter.h"
#include "Character/BasePlayerState.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"

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

	if (Controller == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Gate interaction failed: no controller."));
		return;
	}

	ALCGameMode* LCGM = Cast<ALCGameMode>(GetWorld()->GetAuthGameMode());

	if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		switch (TravelType)
		{
		case EGateTravelType::ToBaseCamp:
		{
			if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(Controller))
			{
				if (RoomPC->HasAuthority())
				{
					RoomPC->Server_MarkPlayerAsEscaped_Implementation();
					if (ALCGameState* GS = GetWorld()->GetGameState<ALCGameState>())
					{
						//GS->MarkPlayerAsEscaped(RoomPC->PlayerState);
					}
				}
				else
				{
					RoomPC->Server_MarkPlayerAsEscaped();
				}

				if (ABasePlayerState* PlayerState = RoomPC->GetPlayerState<ABasePlayerState>())
				{
					// 플레이어 상태 업데이트
					ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(RoomPC->GetPawn());
					PlayerState->AquiredItemIDs.Append(PlayerCharacter->GetToolbarInventoryComponent()->GetInventoryItemIDs());
					PlayerState->AquiredItemIDs.Append(PlayerCharacter->GetBackpackInventoryComponent()->GetInventoryItemIDs());
				}
			}

			// HUD 숨기고 관전 모드 전환
			if (ULCUIManager* UIManager = GISubsystem->GetUIManager())
			{
				UIManager->HideInGameHUD();
			}
			ABasePlayerController* BasePlayerController = Cast<ABasePlayerController>(Controller);
			BasePlayerController->OnExitGate();

			// TODO : 탈출, 체크리스트 띄우고 전부 작성하면 결과 UI-> 호스트가 버튼 눌러서 베이스캠프로 이동
			// 사망->시체 스켈레탈메시남고->관전(컨트롤러)
			// 관전으로 넘기는 함수
			// 탈출시 PS로 아이템 아이디넘김 타이머로 캐릭터 Destroy
			break;
		}
		case EGateTravelType::ToInGame:
		{
			if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(Controller))
			{
				if (ABasePlayerState* PlayerState = RoomPC->GetPlayerState<ABasePlayerState>())
				{
					ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(RoomPC->GetPawn());
					PlayerState->AquiredItemIDs.Append(PlayerCharacter->GetToolbarInventoryComponent()->GetInventoryItemIDs());
				}
			}

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

			break;
		}
		}
	}
}

FString ALCGateActor::GetInteractMessage_Implementation() const
{
	switch (TravelType)
	{
	case EGateTravelType::ToBaseCamp:
	{
		return TEXT("Press [F] to Use Gate");
	}
	case EGateTravelType::ToInGame:
	{
		if (HasAuthority())
		{
			return TEXT("Press [F] to Explore Gate");
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

void ALCGateActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALCGateActor, TravelType);
}