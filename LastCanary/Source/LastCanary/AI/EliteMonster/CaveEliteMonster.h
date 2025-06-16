#pragma once

#include "CoreMinimal.h"
#include "AI/BaseMonsterCharacter.h"
#include "CaveEliteMonster.generated.h"

UCLASS()
class LASTCANARY_API ACaveEliteMonster : public ABaseMonsterCharacter
{
	GENERATED_BODY()
	
public:
	ACaveEliteMonster();

	void FreezeAI();

	void UnfreezeAI();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
	float MaxFreezeTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
	float FreezeCooldown;

	UPROPERTY(BlueprintReadOnly, Category = "Gimmick")
	bool bIsFrozen = false;

	FTimerHandle FreezeTimerHandle;

	/*UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);*/
};