// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LCBossSpawner.generated.h"

UCLASS()
class LASTCANARY_API ALCBossSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ALCBossSpawner();

protected:
	virtual void BeginPlay() override;

public:
	/** 보스 스폰 포인트 (디자이너가 배치한 TargetPoint들 참조) */
	UPROPERTY(EditAnywhere, Category = "Boss|Spawn")
	TArray<AActor*> BossSpawnPoints;

	void SpawnBoss(TSubclassOf<AActor> BossClass);
};
