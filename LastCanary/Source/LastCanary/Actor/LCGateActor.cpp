#include "Actor/LCGateActor.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameMode/BaseGameMode.h"

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
	if (HasAuthority() == false)
	{
		LOG_Frame_WARNING(TEXT("Gate interaction failed: no authority."));
		return;
	}
	if (Controller == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Gate interaction failed: no controller."));
		return;
	}

	if (ABaseGameMode* BaseGM = Cast<ABaseGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (!BaseGM->IsAllPlayersReady())
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


	if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		switch (TravelType)
		{
		case EGateTravelType::ToBaseCamp:
		{
			// TODO : 베이스캠프로 이동
			const FName BaseCampMapName = TEXT("BaseCamp");
			const int32 BaseCampID = FCrc::StrCrc32(*BaseCampMapName.ToString());
			TargetMapID = BaseCampID;
			GISubsystem->ChangeLevelByMapID(TargetMapID);
			break;
		}
		case EGateTravelType::ToInGame:
		{
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

			GISubsystem->ChangeLevelByMapID(TargetMapID);
			break;
		}
		}
	}
}

FString ALCGateActor::GetInteractMessage_Implementation() const
{
	return TEXT("Press [F] to Use Gate");
}

void ALCGateActor::SetTargetMapID(const int32& InMapID)
{
	TargetMapID = InMapID;
}