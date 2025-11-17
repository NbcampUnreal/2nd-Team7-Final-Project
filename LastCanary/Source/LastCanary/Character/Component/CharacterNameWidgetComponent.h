// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterNameWidgetComponent.generated.h"

class UWidgetComponent;

UCLASS()
class LASTCANARY_API UCharacterNameWidgetComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
	UCharacterNameWidgetComponent();

	void UpdateWidget(); // PlayerState 갱신 시 호출
	void TurnOffWidget(); // 죽음/관전 시 호출

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* WidgetComponent;

	/** 내부 WidgetComponent에서 UUserWidget 가져오는 함수 */
	UUserWidget* GetWidget() const;


	// 플레이어 이름 세팅
	void SetPlayerName(const FString& PlayerName);

	// 위젯 가시성 설정
	void SetWidgetVisibility(bool bVisible);

	// 그림자 설정
	void SetCastShadowEnabled(bool bEnable);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> WidgetClass;

	void InitializeWidget();
	void UpdateRotation();

	// 내부 적용
	void ApplyName();

public:
	void InitializeNameWidget();

private:
	FTimerHandle RetryInitializeHandle;

private:
	float TimeAccumulator = 0.f;
	float MaxVisibleDistance = 2500.f;

public:
	void UpdateNameFromPlayerState(APlayerState* PlayerState);
	void HideNameWidget();
	void TryInitializeOnPlayerState(APlayerState* PS, bool bIsLocallyControlled, bool bHasAuthority);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> DefaultWidgetClass;



public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetPlayerName(const FString& PlayerName);
	void Multicast_SetPlayerName_Implementation(const FString& PlayerName);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetPlayerName(const FString& PlayerName);
	bool Server_SetPlayerName_Validate(const FString& PlayerName);
	void Server_SetPlayerName_Implementation(const FString& PlayerName);
};
