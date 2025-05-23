#include "Framework/GameMode/LCRoomGameMode.h"

ALCRoomGameMode::ALCRoomGameMode()
{
	SelectedMap = EMapType::LuinsMap;
}


void ALCRoomGameMode::SelectGameMap(int32 MapIndex)
{
	if (!GetWorld() || !MapReferences.IsValidIndex(MapIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectGameMap: Invalid index %d"), MapIndex);
		return;
	}
	
	SelectedMap = static_cast<EMapType>(MapIndex);
}

void ALCRoomGameMode::StartGame()
{
	// 1) SoftObjectPath: "/Game/.../MainLevel.MainLevel"
	const FString SoftPath = MapReferences[(uint8)SelectedMap]
								.ToSoftObjectPath()
								.GetAssetPathString();

	// 2) 패키지 경로만 분리: "/Game/.../MainLevel"
	const FString PackageName = FPackageName::ObjectPathToPackageName(SoftPath);

	// 3) ?listen 붙여서 최종 URL 생성
	const FString TravelURL = PackageName + TEXT("?listen");
	UE_LOG(LogTemp, Log, TEXT("SelectedMap=%d, Traveling to: %s"),
		   (uint8)SelectedMap, *TravelURL);

	// 4) 이제 오류 없이 ServerTravel 가능
	GetWorld()->ServerTravel(TravelURL, true);
}
