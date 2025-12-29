#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "CaveEliteGimmickComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTCANARY_API UCaveEliteGimmickComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UCaveEliteGimmickComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick")
	bool bAutoActivateGimmick = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Gimmick")
	void SetGimmickActive(bool bNewActive);

private:
	void CheckOverlap();

private:
	FTimerHandle OverlapCheckTimer;
};
