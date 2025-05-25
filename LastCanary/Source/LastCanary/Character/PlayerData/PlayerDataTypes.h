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

// 상태 열거형
UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Exhausted
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
    float WalkSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RunSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SptintSpeed = 650.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CrouchSpeed = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float JumpPower = 420.f;


    //스태미나 관련 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float MaxStamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float StaminaDrainRate = 15.f; // 초당 15 소모

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float StaminaRecoveryRate = 10.f; // 초당 10 회복

    // 필요 시 탈진 상태 확인용 변수도 추가 가능
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stamina")
    bool bIsExhausted = false;
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