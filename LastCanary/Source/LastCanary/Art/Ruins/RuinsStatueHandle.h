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
	// 연결된 회전 동상 참조 (블루프린트에서 설정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	TObjectPtr<AActor> TargetStatue;

	// 핸들 쿨타임 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	float CooldownTime;

	// 마지막 상호작용 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Handle")
	float LastActivatedTime;

public:
	// 인터랙션 처리 (동상 회전 요청)
	virtual void ActivateGimmick_Implementation() override;

	// 상호작용 메시지 반환
	virtual FString GetInteractMessage_Implementation() const override;
};