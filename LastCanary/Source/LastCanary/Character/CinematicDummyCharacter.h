#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CinematicDummyCharacter.generated.h"

USTRUCT(BlueprintType)
struct FCharacterAppearanceData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* BodyMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<UMaterialInterface*> Materials;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UAnimInstance> AnimBP;
};

UCLASS()
class LASTCANARY_API ACinematicDummyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
    ACinematicDummyCharacter();

    // 외형 복사 적용
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyAppearance(const FCharacterAppearanceData& Data);

    // 이동 및 몽타주 실행
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(const FVector& TargetLocation, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void PlayAnimMontageOnce(UAnimMontage* Montage);

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle MoveTimerHandle;
    FVector MoveStart;
    FVector MoveEnd;
    float MoveDuration;
    float MoveElapsed;
    bool bIsMoving;

    void TickMove(float DeltaTime);
    virtual void Tick(float DeltaSeconds) override;
};