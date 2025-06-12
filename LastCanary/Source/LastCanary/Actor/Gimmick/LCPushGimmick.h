#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCTransformGimmick.h"
#include "LCPushGimmick.generated.h"

class UBoxComponent;
class ALCRotationGimmick;

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCPushGimmick : public ALCTransformGimmick
{
	GENERATED_BODY()
	
public:
	ALCPushGimmick();

protected:
	virtual void BeginPlay() override;
public:
	/** 앞쪽 트리거 박스 (뒤로 민다) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Push")
	UBoxComponent* ForwardTrigger;

	/** 뒤쪽 트리거 박스 (앞으로 민다) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Push")
	UBoxComponent* BackwardTrigger;

	///** 트리거별 이동 방향 (트리거가 있는 반대 방향) */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Push")
	//TMap<UBoxComponent*, FVector> PushDirectionMap;

	/** 상호작용 시 이동 수행 */
	virtual void ActivateGimmick_Implementation() override;

	virtual void StartMovement() override;

	virtual void Interact_Implementation(APlayerController* Interactor) override;

	virtual bool CanActivate_Implementation() override;

protected:
	/** 플레이어가 들어온 트리거를 기준으로 방향 계산 */
	bool DeterminePushDirection(FVector& OutDirection);

};