#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "MapSelectInteractor.generated.h"

class ALCGateActor;
class UWidgetComponent;
class ACameraActor;
UCLASS()
class LASTCANARY_API AMapSelectInteractor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	AMapSelectInteractor();

protected:
	virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UWidgetComponent* MapSelectWidgetComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MapSelect")
    ACameraActor* MapSelectCamera;
    UPROPERTY(EditAnywhere, Category = "MapSelect")
    TSubclassOf<UUserWidget> MapSelectWidgetClass;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gate")
    ALCGateActor* TargetGateActor;

    virtual void Interact_Implementation(APlayerController* Controller) override;
    virtual FString GetInteractMessage_Implementation() const override;

    UWidgetComponent* GetMapSelectWidgetComponent() const;
};