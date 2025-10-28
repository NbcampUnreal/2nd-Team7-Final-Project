#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterWeaponClippingComponent.generated.h"

class USphereComponent;
class AGunBase;

UCLASS()
class LASTCANARY_API UCharacterWeaponClippingComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

protected:
	UCharacterWeaponClippingComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float WallClipAimOffsetPitch = 0.0;

	AGunBase* GetGun();

	USkeletalMeshComponent* GetGunSkeletalMesh();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceClippingPoint")
	float TraceDistance = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceClippingPoint")
	float TraceRadius = 5.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceClippingPoint")
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;


	void UpdateGunWallClipOffset();


	FVector HandClipLocation = FVector();
	FRotator HandClipRotation = FRotator();


	FVector HandClipLocationOffset = FVector(2.0f, -0.5f, -2.0f);
	

};
