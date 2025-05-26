#include "Actor/LCGateActor.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

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
	if (HasAuthority() ==false)
	{
		LOG_Frame_WARNING(TEXT("Gate interaction failed: no authority."));
		return;
	}
	if (Controller==nullptr)
	{
		LOG_Frame_WARNING(TEXT("Gate interaction failed: no controller."));
		return;
	}

	if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (TargetMapID != 0)
		{
			GISubsystem->ChangeLevelByMapID(TargetMapID);
			LOG_Frame_WARNING(TEXT("Gate actor %s is changing level to map ID %d."), *GetName(), TargetMapID);
		}
		else
		{
			LOG_Frame_WARNING(TEXT("No valid target map set on GateActor."));
		}
	}
}

void ALCGateActor::SetTargetMapID(const int32& InMapID)
{
	TargetMapID = InMapID;
}