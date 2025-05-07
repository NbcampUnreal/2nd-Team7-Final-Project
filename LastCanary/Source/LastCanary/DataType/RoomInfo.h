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
    FString RoomID;  // 고유 방 식별자

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    FString RoomName;

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    int32 CurrentPlayers;

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    int32 MaxPlayers;

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    bool bIsPrivate;

    UPROPERTY(BlueprintReadWrite, Category = "Room")
    FString HostPlayerName;
};