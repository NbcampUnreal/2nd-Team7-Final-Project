#include "Framework/GameMode/LCInGameModeBase.h"

#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/Manager/ChecklistManager.h"

#include "Kismet/GameplayStatics.h"

ALCInGameModeBase::ALCInGameModeBase()
{
	static ConstructorHelpers::FClassFinder<AChecklistManager> ChecklistBPClass(TEXT("/Game/_LastCanary/Blueprint/Framework/Manager/BP_ChecklistManager.BP_ChecklistManager"));
	if (ChecklistBPClass.Succeeded())
	{
		ChecklistManagerClass = ChecklistBPClass.Class;
	}
}

void ALCInGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 생존자 수 초기화
	if (ALCGameState* LCGameState = GetGameState<ALCGameState>())
	{
		int32 InitialAliveCount = 0;

		for (APlayerState* PlayerState : LCGameState->PlayerArray)
		{
			if (IsValid(PlayerState) && !PlayerState->IsOnlyASpectator())
			{
				++InitialAliveCount;
			}
		}

		LCGameState->AlivePlayerCount = InitialAliveCount;
	}

	// ChecklistManager 동적 생성
	if (HasAuthority() && ChecklistManagerClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ChecklistManager = GetWorld()->SpawnActor<AChecklistManager>(ChecklistManagerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (ChecklistManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Checklist] ChecklistManager Spawned"));
		}
	}
}

void ALCInGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 연결된 클라이언트에게 ChecklistManager의 Owner 지정
	if (ChecklistManager && NewPlayer)
	{
		ChecklistManager->SetOwner(NewPlayer);
	}
}

void ALCInGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

}
