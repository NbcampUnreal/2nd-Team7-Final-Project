#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "ShopInteractor.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class UInputAction;
UCLASS()
class LASTCANARY_API AShopInteractor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AShopInteractor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* ShopWidgetComponent;

	UPROPERTY(EditAnywhere)
	ACameraActor* ShopCamera;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ShopWidgetClass;

	virtual void BeginPlay() override;

public:
	virtual void Interact_Implementation(APlayerController* Interactor) override;
	virtual FString GetInteractMessage_Implementation() const override;

	UWidgetComponent* GetShopWidgetComponent() const;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Interact;

	UFUNCTION(BlueprintCallable, Category = "Input")
	FString GetCurrentKeyNameForAction(UInputAction* InputAction) const;
};
