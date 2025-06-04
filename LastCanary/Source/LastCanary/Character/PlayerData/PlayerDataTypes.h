#pragma once

#include "CoreMinimal.h"
#include "PlayerDataTypes.generated.h"

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
    None UMETA(DisplayName = "None"),
    Idle UMETA(DisplayName = "Idle"),
    SplitMind UMETA(DisplayName = "정신분열중"),
    Dead UMETA(DisplayName = "사망"),
    Escape UMETA(DisplayName = "탈출")
    // 필요한 상태 더 추가
};

// 상태 열거형
UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Exhausted
};

UENUM(BlueprintType)
enum class EInputMode : uint8
{
    Hold,
    Toggle
};

UENUM(BlueprintType)
enum class EPlayerInGameStatus : uint8
{
    None        UMETA(DisplayName = "없음"),
    Alive      UMETA(DisplayName = "게임 플레이 중"),
    Spectating  UMETA(DisplayName = "관전 중")
};

// ------------------------------
// 플레이어 능력치 Struct
// ------------------------------
USTRUCT(BlueprintType)
struct FPlayerStats
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float MaxHP = 100.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float WalkSpeed = 175.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float RunSpeed = 375.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float SptintSpeed = 650.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CrouchSpeed = 150.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float JumpPower = 450.0f;

    //스태미나 관련 추가
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float MaxStamina = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float StaminaDrainRate = 15.f; // 초당 15 소모

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float StaminaRecoveryRate = 30.f; // 초당 30 회복

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float RecoverDelayTime = 3.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float JumpStaminaCost = 15.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float ExhaustionRecoveryThreshold = 30.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
    float FallDamageThreshold = 1000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
    float WeightSlowdownMultiplier = 50.0f;
};

// ------------------------------
// 게임 끝나고 얻을 Struct
// ------------------------------
USTRUCT(BlueprintType)
struct FMatchReward
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    int32 EarnedGold;

    UPROPERTY(BlueprintReadWrite)
    int32 EarnedExp;
};