#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CameraRecoilComponent.generated.h"

UCLASS()
class LASTCANARY_API UCameraRecoilComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()
	

protected:
	UPROPERTY()
	APlayerController* CachedController;
	
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	APlayerController* GetPlayerController() const { return CachedController; }

public:
	// Header 파일에 추가할 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float BaseRecoilPitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float BaseRecoilYaw = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float RecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float RecoilRecoverySpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (ClampMin = "0.01"))
	float RecoilRecoveryAmount = 0.4f;


	// 현재 연사 상태
	int32 CurrentShotCount = 0;
	FVector2D AccumulatedRecoil = FVector2D::ZeroVector;
	FVector2D TargetRecoil = FVector2D::ZeroVector;
	FTimerHandle RecoilRecoveryTimer;
	FTimerHandle ShotResetTimer;

	// 반동 패턴 (선택사항 - CS:GO 스타일)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	TArray<FVector2D> RecoilPattern;

	// 반동
	void ApplySmoothRecoil(float Vertical, float Horizontal);
	void ApplySmoothRecoilStep();
	void ResetShotCounter();
	bool HasActiveRecoil() const;
	void ReduceRecoil(float ReductionFactor = 0.5f);
	void ResetRecoilYaw();
	void ResetRecoilPitch();
	void ResetRecoil();
	FVector2D GetCurrentRecoil() const;
};
