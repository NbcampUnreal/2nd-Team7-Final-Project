#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterADSComponent.generated.h"

class AGunBase;

UCLASS()
class LASTCANARY_API UCharacterADSComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
    UCharacterADSComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    bool bIsADS = false;
    void SwitchADS(bool _bIsADS);

    float ADS_Weight = 0.0f;
    
    FORCEINLINE float Get_ADS_Weight() const { return ADS_Weight; }
    void Set_ADS_Weight(float _ADS_Weight);

    AGunBase* GetGun();
    USkeletalMeshComponent* GetGunSkeletalMesh();
    FName GetADSSocketName(AGunBase* gun);
    float GetADS_Distance(AGunBase* gun);

    float ADS_InterpSpeed = 3.0f;
    float ADS_Distance = 5.0f;


    void SetADSMode(float _ADS_Weight);

    void SetLeftHandIK();
    void SetRightHandIK();
    void SetikHandgunLocation();

    void CalculateAimSocket();
    void CalculateAimPoint();
    
    void SetGunItemSocketTransform();

    FVector GetAimTargetLocation(float Distance);
    
};
