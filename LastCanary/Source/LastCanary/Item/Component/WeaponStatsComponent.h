#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "WeaponStatsComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponUsageStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 TotalShots = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ShotsHit = 0;

	UPROPERTY(BlueprintReadOnly)
	float TotalDamageDealt = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly)
	float AccuracyRate = 0.0f;

	void CalculateAccuracy()
	{
		AccuracyRate = (TotalShots > 0) ? (static_cast<float>(ShotsHit) / TotalShots) * 100.0f : 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FWeaponStatsEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly)
	FWeaponUsageStats Stats;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponStatsUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UWeaponStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponStatsComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:	
	// 통계 데이터(서버에서만 업데이트, 클라이언트로 복제)
	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<FWeaponStatsEntry> WeaponStatsArray;

	// 전체 통계
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponUsageStats TotalStats;

	// 총기 발사 기록
	UFUNCTION(BlueprintCallable, Category = "Weapon Stats")
	void RecordShot(FGameplayTag WeaponTag);

	// 명중 기록
	UFUNCTION(BlueprintCallable, Category = "Weapon Stats")
	void RecordHit(FGameplayTag WeaponTag, float DamageDealt, bool bWasKill = false);

	// 통계 조회
	UFUNCTION(BlueprintPure, Category = "Weapon Stats")
	FWeaponUsageStats GetWeaponStats(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "Weapon Stats")
	float GetOverallAccuracy() const;

	// 통계 초기화
	UFUNCTION(BlueprintCallable, Category = "Weapon Stats")
	void ResetStats();

	UFUNCTION(BlueprintCallable, Category = "Weapon Stats")
	void ResetWeaponStats(FGameplayTag WeaponTag);

	//-----------------------------------------------------
	// 델리게이트
	//-----------------------------------------------------

	/** 통계가 업데이트될 때마다 브로드캐스트 */
	UPROPERTY(BlueprintAssignable, Category = "Weapon Stats")
	FOnWeaponStatsUpdated OnStatsUpdated;

private:
	FWeaponUsageStats* FindWeaponStats(FGameplayTag WeaponTag);
	const FWeaponUsageStats* FindWeaponStats(FGameplayTag WeaponTag) const;
};
