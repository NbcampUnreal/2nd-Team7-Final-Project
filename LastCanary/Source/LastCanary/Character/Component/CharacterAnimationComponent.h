// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterAnimationComponent.generated.h"

class AItemBase;
struct FGameplayTag;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadNotify);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionNotify);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUseItemNotify);

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
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TMap<EAnimationMontageType, UAnimMontage*> MontageMap;

	USkeletalMeshComponent* CharacterMesh();
protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayMontageByType(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayInteractMontage(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayUseItemMontage(AItemBase* Item);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayGunReloadMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayEmoteMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayAttackMontage(UAnimMontage* _AttackMontage);
private:
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type);
	void Server_PlayMontage_Implementation(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type);
	void Multicast_PlayMontage_Implementation(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type);



public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelAllMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelMontageByType(UAnimMontage* LocalMontageToStop, UAnimMontage* RemoteMontageToStop, EAnimationMontageType Type);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelInteractModntage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelUseItemMontage(AItemBase* Item);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelGunReloadMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelEmoteMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CancelAttackMontage();

private:
	UFUNCTION(Server, Reliable)
	void Server_CancelMontage(UAnimMontage* LocalMontageToStop, UAnimMontage* RemoteMontageToStop, EAnimationMontageType Type);
	void Server_CancelMontage_Implementation(UAnimMontage* LocalMontageToStop, UAnimMontage* RemoteMontageToStop, EAnimationMontageType Type);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CancelMontage(UAnimMontage* LocalMontageToStop, UAnimMontage* RemoteMontageToStop, EAnimationMontageType Type);
	void Multicast_CancelMontage_Implementation(UAnimMontage* LocalMontageToStop, UAnimMontage* RemoteMontageToStop, EAnimationMontageType Type);


public:
	void HandleAnimNotify(EAnimationMontageType Type);
	
	UPROPERTY(BlueprintAssignable, Category="Notify Events")
	FOnReloadNotify OnReloadNotify;

	UPROPERTY(BlueprintAssignable, Category="Notify Events")
	FOnInteractionNotify OnInteractionNotify;

	UPROPERTY(BlueprintAssignable, Category="Notify Events")
	FOnUseItemNotify OnUseItemNotify;

public:  // 각 몽타주 타입별 재생 상태 플래그
	void SetPlayingMontageState(EAnimationMontageType Type, bool bIsPlaying);
private:  	
	bool bIsPlayingInteractionMontage = false;
	bool bIsPlayingUseItemMontage = false;
	bool bIsPlayingGunReloadMontage = false;
	bool bIsPlayingEmoteMontage = false;
	bool bIsPlayingAttackMontage = false;

public:
	bool GetIsPlayingInteractionMontage() { return bIsPlayingInteractionMontage; }
	bool GetIsPlayingUseItemMontage() { return bIsPlayingUseItemMontage; }
	bool GetIsPlayingGunReloadMontage() { return bIsPlayingGunReloadMontage; }
	bool GetIsPlayingEmoteMontage() { return bIsPlayingEmoteMontage; }
	bool GetIsPlayingAttackMontage() { return bIsPlayingAttackMontage; }
	


public:
	void RefreshOverlayLinkedAnimationLayer(FGameplayTag ItemTag);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> RifleAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> PistolOneHandedAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> PistolTwoHandedAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> TorchAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> BinocularsAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> PickaxeAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	USkeletalMesh* SKM_Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	USkeletalMesh* SKM_Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UStaticMesh* SM_Torch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UStaticMesh* RCController;
};
