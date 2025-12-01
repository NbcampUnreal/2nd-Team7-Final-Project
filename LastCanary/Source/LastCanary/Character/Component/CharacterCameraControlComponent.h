#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterCameraControlComponent.generated.h"


class USpringArmComponent;

UCLASS()
class LASTCANARY_API UCharacterCameraControlComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	
	USpringArmComponent* GetCharacterSpringArm();
	USkeletalMeshComponent* CharacterMesh();

public:
	bool bIsFPSCamera = true;
	bool EmoteMode = false;
public:
	void SwitchToFirstPerson();
	void SwitchToThirdPerson();

	void SetCameraMode(bool bIsFirstPersonView);

	void SetCameraEmoteMode(bool bIsFirstPersonView);

	void Handle_ViewMode();

	void ResetCameraLocationToDefault();

public:
	void SetSpringArmTargetLength(float Distance);
	void SetTransparentHeadMesh(bool bIsTransparent);
	void StartAiming();
	void StopAiming();
};
