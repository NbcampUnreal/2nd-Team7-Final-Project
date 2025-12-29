// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainingType.generated.h"

UENUM(BlueprintType)
enum class ETrainingMode : uint8
{
    Practice    UMETA(DisplayName = "연습"),
    Timed       UMETA(DisplayName = "시간제"),
    Survival    UMETA(DisplayName = "서바이벌")
};

UENUM(BlueprintType)
enum class EDummyType : uint8
{
    Static      UMETA(DisplayName = "정지"),
    Patrol      UMETA(DisplayName = "순찰"),
    Random      UMETA(DisplayName = "랜덤"),
    Reactive    UMETA(DisplayName = "반응형")
};

UENUM(BlueprintType)
enum class ESpawnPattern : uint8
{
    FreeRandom      UMETA(DisplayName = "자유 랜덤"),
    GridRandom      UMETA(DisplayName = "그리드 랜덤"),
    GridByDistance  UMETA(DisplayName = "거리별 그리드")
};

USTRUCT(BlueprintType)
struct FTrainingStats
{
    GENERATED_BODY()

    /** 총 발사 횟수 */
    UPROPERTY(BlueprintReadOnly)
    int32 TotalShots = 0;

    /** 총 명중 횟수 */
    UPROPERTY(BlueprintReadOnly)
    int32 TotalHits = 0;

    /** 파괴한 더미 수 */
    UPROPERTY(BlueprintReadOnly)
    int32 DummiesDestroyed = 0;

    /** 총 입힌 데미지 */
    UPROPERTY(BlueprintReadOnly)
    float TotalDamageDealt = 0.0f;

    /** 세션 진행 시간 */
    UPROPERTY(BlueprintReadOnly)
    float SessionTime = 0.0f;

    /** 명중률 (%) */
    UPROPERTY(BlueprintReadOnly)
    float AccuracyPercentage = 0.0f;

    /** 헤드샷 횟수 */
    UPROPERTY(BlueprintReadOnly)
    int32 HeadshotCount = 0;

    /** 평균 DPS */
    UPROPERTY(BlueprintReadOnly)
    float AverageDPS = 0.0f;

    /** 통계 초기화 */
    void Reset()
    {
        TotalShots = 0;
        TotalHits = 0;
        DummiesDestroyed = 0;
        TotalDamageDealt = 0.0f;
        SessionTime = 0.0f;
        AccuracyPercentage = 0.0f;
        HeadshotCount = 0;
        AverageDPS = 0.0f;
    }

    /** 명중률 계산 */
    void CalculateAccuracy()
    {
        if (TotalShots > 0)
        {
            AccuracyPercentage = (static_cast<float>(TotalHits) / TotalShots) * 100.0f;
        }
        else
        {
            AccuracyPercentage = 0.0f;
        }
    }

    /** DPS 계산 */
    void CalculateDPS()
    {
        if (SessionTime > 0.0f)
        {
            AverageDPS = TotalDamageDealt / SessionTime;
        }
        else
        {
            AverageDPS = 0.0f;
        }
    }
};

