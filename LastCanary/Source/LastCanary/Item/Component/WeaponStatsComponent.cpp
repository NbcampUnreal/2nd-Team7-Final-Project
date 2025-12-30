#include "Item/Component/WeaponStatsComponent.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"


UWeaponStatsComponent::UWeaponStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

void UWeaponStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponStatsComponent, WeaponStatsArray);
	DOREPLIFETIME(UWeaponStatsComponent, TotalStats);
}

void UWeaponStatsComponent::RecordShot(FGameplayTag WeaponTag)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (WeaponTag.IsValid())
	{
		FWeaponUsageStats* Stats = FindWeaponStats(WeaponTag);

		if (Stats)
		{
			Stats->TotalShots++;
			Stats->CalculateAccuracy();
		}
	}

	TotalStats.TotalShots++;
	TotalStats.CalculateAccuracy();

	OnStatsUpdated.Broadcast();
}

void UWeaponStatsComponent::RecordHit(FGameplayTag WeaponTag, float DamageDealt, bool bWasKill)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (WeaponTag.IsValid())
	{
		FWeaponUsageStats* Stats = FindWeaponStats(WeaponTag);

		if (Stats)
		{
			Stats->ShotsHit++;
			Stats->TotalDamageDealt += DamageDealt;

			if (bWasKill)
			{
				Stats->Kills++;
			}

			Stats->CalculateAccuracy();
		}
	}

	TotalStats.ShotsHit++;
	TotalStats.TotalDamageDealt += DamageDealt;

	if (bWasKill)
	{
		TotalStats.Kills++;
	}

	TotalStats.CalculateAccuracy();

	OnStatsUpdated.Broadcast();
}

FWeaponUsageStats UWeaponStatsComponent::GetWeaponStats(FGameplayTag WeaponTag) const
{
	const FWeaponUsageStats* Found = FindWeaponStats(WeaponTag);

	if (Found)
	{
		return *Found;
	}

	return FWeaponUsageStats();
}

float UWeaponStatsComponent::GetOverallAccuracy() const
{
	return TotalStats.AccuracyRate;
}

void UWeaponStatsComponent::ResetStats()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	WeaponStatsArray.Empty();
	TotalStats = FWeaponUsageStats();
}

void UWeaponStatsComponent::ResetWeaponStats(FGameplayTag WeaponTag)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	for (FWeaponStatsEntry& Entry : WeaponStatsArray)
	{
		if (Entry.WeaponTag == WeaponTag)
		{
			Entry.Stats = FWeaponUsageStats();
			return;
		}
	}
}



// ===== 헬퍼 함수 =====

FWeaponUsageStats* UWeaponStatsComponent::FindWeaponStats(FGameplayTag WeaponTag)
{
	// 배열에서 해당 WeaponTag를 가진 엔트리 찾기
	for (FWeaponStatsEntry& Entry : WeaponStatsArray)
	{
		if (Entry.WeaponTag == WeaponTag)
		{
			return &Entry.Stats;
		}
	}

	// 없으면 새로 추가
	FWeaponStatsEntry NewEntry;
	NewEntry.WeaponTag = WeaponTag;
	NewEntry.Stats = FWeaponUsageStats();

	int32 Index = WeaponStatsArray.Add(NewEntry);
	return &WeaponStatsArray[Index].Stats;
}

const FWeaponUsageStats* UWeaponStatsComponent::FindWeaponStats(FGameplayTag WeaponTag) const
{
	// 읽기 전용 버전 - 찾지 못하면 nullptr 반환
	for (const FWeaponStatsEntry& Entry : WeaponStatsArray)
	{
		if (Entry.WeaponTag == WeaponTag)
		{
			return &Entry.Stats;
		}
	}

	return nullptr;
}

