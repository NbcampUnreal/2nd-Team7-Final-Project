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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* ShopWidgetComponent;

	UPROPERTY(EditAnywhere)
	ACameraActor* ShopCamera;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ShopWidgetClass;

	//UFUNCTION(Server, Reliable)
	//void Server_ShowPopupShopWidget();
	//void Server_ShowPopupShopWidget_Implementation();
	//UFUNCTION(Client, Reliable)
	//void Client_ShowPopupShopWidget(int Gold);
	//void Client_ShowPopupShopWidget_Implementation(int Gold);

	virtual void BeginPlay() override;

public:
	virtual void Interact_Implementation(APlayerController* Interactor) override;
	virtual FString GetInteractMessage_Implementation() const override;

	UWidgetComponent* GetShopWidgetComponent() const;
};
