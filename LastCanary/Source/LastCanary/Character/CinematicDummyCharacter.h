#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/CustomizationMeshMap.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomHeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomGloveMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomJacketMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomPantsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomBeltsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomHelmetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomArmorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomBootsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* BackpackMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	UCustomizationMeshMap* CharacterMeshMap;

	void ApplyCustomization(const UCustomizationMeshMap* CharacterMeshData, FCharacterCustomizationData CustomizationData);

	void SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh);

	void SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material);

	void SetCharacterPoseSynchronization();

    // 외형 복사 적용
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyAppearance(FCharacterCustomizationData Data);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyAppearance(FCharacterCustomizationData Data);
	void Multicast_ApplyAppearance_Implementation(FCharacterCustomizationData Data);

    // 이동 및 몽타주 실행
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(const FVector& TargetLocation, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void PlayAnimMontageOnce(UAnimMontage* Montage);

	UPROPERTY(VisibleAnywhere, Replicated)
	int32 CutsceneIndex = -1;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;

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