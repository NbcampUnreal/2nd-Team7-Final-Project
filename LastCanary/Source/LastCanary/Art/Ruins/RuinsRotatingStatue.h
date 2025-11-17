#pragma once

#include "CoreMinimal.h"
#include "Art/Ruins/RuinsGimmickBase.h"
#include "RuinsRotatingStatue.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ARuinsRotatingStatue : public ARuinsGimmickBase
{
	GENERATED_BODY()

public:
	ARuinsRotatingStatue();

protected:
	virtual void BeginPlay() override;

public:
	/** 상호작용 호출 시 (핸들이 호출할 수 있음) */
	virtual void Interact_Implementation(APlayerController* Interactor) override;

	/** 쿨타임 없이 회전만 수행 */
	virtual void ActivateGimmick_Implementation() override;
};