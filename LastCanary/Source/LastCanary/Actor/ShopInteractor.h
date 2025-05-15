#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "ShopInteractor.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
UCLASS()
class LASTCANARY_API AShopInteractor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AShopInteractor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* ShopWidgetComponent;

	UPROPERTY(EditAnywhere)
	ACameraActor* ShopCamera;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ShopWidgetClass;


	virtual void BeginPlay() override;

public:
	virtual void Interact_Implementation(APlayerController* Interactor) override;
	
	UWidgetComponent* GetShopWidgetComponent() const;
};
