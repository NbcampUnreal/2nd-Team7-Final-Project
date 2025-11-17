#pragma once

#include "CoreMinimal.h"
#include "Art/Ruins/RuinsGimmickBase.h"
#include "RuinsStatueHandle.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ARuinsStatueHandle : public ARuinsGimmickBase
{
	GENERATED_BODY()

public:
	ARuinsStatueHandle();

protected:
	virtual void BeginPlay() override;

public:
	/** 회전 대상이 될 동상 액터 (ARuinsRotatingStatue) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Handle")
	TObjectPtr<AActor> TargetStatue;

	/** 상호작용 처리 → 쿨타임 검사 → 동상 회전 요청 */
	virtual void ActivateGimmick_Implementation() override;

	/** UI 상호작용 메시지 오버라이드 */
	virtual FString GetInteractMessage_Implementation() const override;
};