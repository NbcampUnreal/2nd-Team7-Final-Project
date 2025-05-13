#pragma once

#include "CoreMinimal.h"
#include "PlayerDataTypes.generated.h"

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
    None UMETA(DisplayName = "None"),
    Idle UMETA(DisplayName = "Idle"),
    SplitMind UMETA(DisplayName = "정신분열중"),
    Dead UMETA(DisplayName = "사망")
    // 필요한 상태 더 추가
};

// ------------------------------
// 플레이어 능력치 Struct
// ------------------------------
USTRUCT(BlueprintType)
struct FPlayerStats
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WalkSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RunSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CrouchSpeed = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float JumpPower = 420.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RollSpeed = 500.f;

    // 필요하면 함수도 추가 가능
    void ResetHP() { CurrentHP = MaxHP; }
};