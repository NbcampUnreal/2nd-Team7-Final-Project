// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterAnimationComponent.generated.h"

UENUM(BlueprintType)
enum class EAnimationMontageType : uint8
{
	None		UMETA(DisplayName = "None"),
	UseItem		UMETA(DisplayName = "아이템 사용"),
	Interaction UMETA(DisplayName = "상호작용"),
	GunReload	UMETA(DisplayName = "재장전"),
	Emote		UMETA(DisplayName = "감정표현"),
	Attack		UMETA(DisplayName = "근접공격"),
	Etc			UMETA(DisplayName = "기타등등")
	// 필요한 상태 더 추가
};

UENUM(BlueprintType)
enum class EAnimationPlaybackMode : uint8
{
	Hold	UMETA(DisplayName = "Hold"),
	Toggle	UMETA(DisplayName = "Toggle")
};

UCLASS()
class LASTCANARY_API UCharacterAnimationComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()
	
public:
	UCharacterAnimationComponent();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY()
	UAnimInstance* CachedAnimInstance;

public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayMontageByType(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayInteractMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayUseItemMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayGunReloadMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayEmoteMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayAttackMontage();
private:
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);
	void Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);
	void Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);



public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelMontageByType(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelInteractModntage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelUseItemMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelGunReloadMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelEmoteMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelAttackMontage();

private:
	UFUNCTION(Server, Reliable)
	void Server_CancelMontage(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);
	void Server_CancelMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CancelMontage(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);
	void Multicast_CancelMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype);


public:
	void HandleAnimNotify(EAnimationMontageType Animtype);
};
