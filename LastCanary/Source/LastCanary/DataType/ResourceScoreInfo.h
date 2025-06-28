#pragma once

#include "CoreMinimal.h"
#include "ResourceScoreInfo.generated.h"

USTRUCT(BlueprintType)
struct FResourceScoreInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ResourceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BaseScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Multiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalScore;
};

USTRUCT(BlueprintType)
struct FResultRewardEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText RewardType; // 보상 종류 (예: "Gold", "Item", "Experience" 등)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description; // 보상 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardGold; // 보상 수량
};

USTRUCT(BlueprintType)
struct FExplorePointInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NoteTypeIndex;     // 노트 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExplorePoint;     // 탐사 포인트
};

USTRUCT(BlueprintType)
struct FCollectedNoteInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NoteTypeIndex;     // 노트 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExplorePoint;     // 탐사 포인트
};
