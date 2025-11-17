#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterSoundComponent.generated.h"


UCLASS()
class LASTCANARY_API UCharacterSoundComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
    UCharacterSoundComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	USoundBase* OnHitSound;

	UFUNCTION(Client, Reliable)
	void Client_PlayHitSound();
	void Client_PlayHitSound_Implementation();
};
