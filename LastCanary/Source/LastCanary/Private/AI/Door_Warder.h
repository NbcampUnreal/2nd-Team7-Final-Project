#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Interface/InteractableInterface.h" 
#include "Door_Warder.generated.h"

class AWarderBaseMonster;

UCLASS()
class LASTCANARY_API ADoor_Warder : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ADoor_Warder();

protected:
	bool bIsOpen = false;

public:
	UPROPERTY(EditInstanceOnly, Category = "Link")
	AWarderBaseMonster* WarderMonster;

	virtual void Interact_Implementation(APlayerController* Interactor) override;
};