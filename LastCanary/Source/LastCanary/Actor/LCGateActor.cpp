#include "Actor/LCGateActor.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameManager.h"
#include "Character/BasePlayerController.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/PlayerController/LCInGamePlayerController.h"
#include "Character/BaseCharacter.h"
#include "Character/BasePlayerState.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "Framework/Manager/GateCutsceneManager.h"
#include "EngineUtils.h"

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
		// IntoGameLevel(Controller);
		// break;

		// 컷신 전에 Ready 체크
		if (HasAuthority())
		{
			if (ALCGameMode* GM = GetWorld()->GetAuthGameMode<ALCGameMode>())
			{
				if (GM->IsAllPlayersReady() == false)
				{
					LOG_Frame_WARNING(TEXT("모든 플레이어가 준비되지 않았습니다. 컷신 재생을 중단합니다."));
					return;
				}
			}
		}

		TArray<AActor*> FoundCharacters;
		UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), FoundCharacters);

		TArray<ABaseCharacter*> PlayerCharacters;
		for (AActor* Actor : FoundCharacters)
		{
			if (ABaseCharacter* BaseChar = Cast<ABaseCharacter>(Actor))
			{
				if (APlayerController* PC = Cast<APlayerController>(BaseChar->GetController()))
				{
					// Valid한 플레이어 컨트롤러가 소유한 캐릭터만 추가
					PlayerCharacters.Add(BaseChar);
				}
			}
		}

		// GateCutsceneManager 찾고 컷신 시작
		for (TActorIterator<AGateCutsceneManager> It(GetWorld()); It; ++It)
		{
			AGateCutsceneManager* GateCutsceneManager = *It;
			GateCutsceneManager->LinkedGateActor = this;
			//서버에서 실행, 서버의 모든 플레이어 캐릭터의 정보를 게이트 매니저로 전송
			GateCutsceneManager->PlayGateCutscene(PlayerCharacters);
			break;
		}
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

	InGamePC->OnExitGate();
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
	if (HasAuthority() == false)
	{
		return;
	}
	if (!LCGM)
	{
		LOG_Server_ERROR(TEXT("Cast Fail GameMode : Not Server!!"));
		return;
	}

	LOG_Server_WARNING(TEXT("All Client is Ready!! Try To Server Travel"));

	ULCGameManager* GameManager = GetGameInstance()->GetSubsystem<ULCGameManager>();
	if (!IsValid(GameManager))
	{
		return;
	}

	if (UDataTable* MapTable = GISubsystem->GetMapDataTable())
	{
		TArray<FMapDataRow*> AllMaps;
		static const FString Ctx = TEXT("GateActor-SelectRandomMap");
		MapTable->GetAllRows(Ctx, AllMaps);


		FMapDataRow* TargetMapData = AllMaps[GameManager->CurrentRound];
		FString TargetMapPath = TargetMapData->MapInfo.MapPath.ToSoftObjectPath().ToString();
		LCGM->TravelMapBySoftPath(TargetMapPath);
	}

	//GISubsystem->ChangeLevelByMapID(TargetMapID);

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