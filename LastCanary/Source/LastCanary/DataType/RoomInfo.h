#pragma once

#include "CoreMinimal.h"
#include "RoomInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FRoomInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Room")
    FString RoomID = TEXT("");  // 고유 방 식별자

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    FString RoomName = TEXT("");

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    int32 CurrentPlayers = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    int32 MaxPlayers = 4;

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    bool bIsPrivate = false;

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    FString HostPlayerName = TEXT("");
};