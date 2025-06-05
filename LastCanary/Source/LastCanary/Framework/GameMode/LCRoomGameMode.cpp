#include "Framework/GameMode/LCRoomGameMode.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/Manager/ChecklistManager.h"

#include "Kismet/GameplayStatics.h"

ALCRoomGameMode::ALCRoomGameMode()
{
	//SelectedMap = EMapType::LuinsMap;
	static ConstructorHelpers::FClassFinder<AChecklistManager> ChecklistBPClass(TEXT("/Game/_LastCanary/Blueprint/Framework/Manager/BP_ChecklistManager.BP_ChecklistManager"));
	if (ChecklistBPClass.Succeeded())
	{
		ChecklistManagerClass = ChecklistBPClass.Class;
	}
}

void ALCRoomGameMode::BeginPlay()
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

void ALCRoomGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 연결된 클라이언트에게 ChecklistManager의 Owner 지정
	if (ChecklistManager && NewPlayer)
	{
		ChecklistManager->SetOwner(NewPlayer);
	}
}

APlayerController* ALCRoomGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

}

void ALCRoomGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

}

//void ALCRoomGameMode::SelectGameMap(int32 MapIndex)
//{
//	if (!GetWorld() || !MapReferences.IsValidIndex(MapIndex))
//	{
//		UE_LOG(LogTemp, Warning, TEXT("SelectGameMap: Invalid index %d"), MapIndex);
//		return;
//	}
//	
//	SelectedMap = static_cast<EMapType>(MapIndex);
//}
//
//void ALCRoomGameMode::StartGame()
//{
//	// 1) SoftObjectPath: "/Game/.../MainLevel.MainLevel"
//	const FString SoftPath = MapReferences[(uint8)SelectedMap]
//								.ToSoftObjectPath()
//								.GetAssetPathString();
//
//	// 2) 패키지 경로만 분리: "/Game/.../MainLevel"
//	const FString PackageName = FPackageName::ObjectPathToPackageName(SoftPath);
//
//	// 3) ?listen 붙여서 최종 URL 생성
//	const FString TravelURL = PackageName + TEXT("?listen");
//	UE_LOG(LogTemp, Log, TEXT("SelectedMap=%d, Traveling to: %s"),
//		   (uint8)SelectedMap, *TravelURL);
//
//	// 4) 이제 오류 없이 ServerTravel 가능
//	GetWorld()->ServerTravel(TravelURL, true);
//}
