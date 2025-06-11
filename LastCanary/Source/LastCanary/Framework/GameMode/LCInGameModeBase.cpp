#include "Framework/GameMode/LCInGameModeBase.h"

#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/Manager/ChecklistManager.h"

#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

ALCInGameModeBase::ALCInGameModeBase()
{
	if (ChecklistManagerClass.IsValid())
	{
		FActorSpawnParameters SpawnParams;
		AChecklistManager* Spawned = GetWorld()->SpawnActor<AChecklistManager>(ChecklistManagerClass.Get(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		ChecklistManager = Spawned;
	}
}

void ALCInGameModeBase::StartPlay()
{
	Super::StartPlay();
	LOG_Server(Log, TEXT("InGameMode Start!!"));
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

		if (ChecklistManagerClass.IsValid())
		{
			ChecklistManager = GetWorld()->SpawnActor<AChecklistManager>(
				ChecklistManagerClass.Get(),               // UClass*
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams
			);
		}

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
