// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Character/PlayerData/PlayerDataTypes.h"
#include "BasePlayerState.generated.h"


UCLASS()
class LASTCANARY_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FPlayerStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EPlayerState CurrentState = EPlayerState::Idle;

};
