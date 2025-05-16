#include "Actor/ShopInteractor.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

#include "UI/UIElement/ShopWidget.h"
#include "UI/Manager/LCUIManager.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "LastCanary.h"

AShopInteractor::AShopInteractor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	ShopWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ShopWidgetComponent"));
	ShopWidgetComponent->SetupAttachment(RootComponent);
	ShopWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	ShopWidgetComponent->SetDrawSize(FVector2D(800, 600));
	ShopWidgetComponent->SetVisibility(true);
}

void AShopInteractor::BeginPlay()
{
	Super::BeginPlay();

	if (ShopWidgetClass)
	{
		ShopWidgetComponent->SetWidgetClass(ShopWidgetClass);
	}
}

void AShopInteractor::Interact_Implementation(APlayerController* Interactor)
{
	LOG_Frame_WARNING(TEXT("Interact_Implementation"));

	if (Interactor == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Interactor is nullptr"));
		return;
	}
	if (ShopCamera == nullptr)
	{
		LOG_Frame_WARNING(TEXT("ShopCamera is nullptr"));
		return;
	}

	Interactor->SetViewTargetWithBlend(ShopCamera, 0.5f);

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;
	TimerDel.BindLambda([this, Interactor]()
		{
			if (UGameInstance* GameInstance = Interactor->GetGameInstance())
			{
				if (ULCGameInstanceSubsystem* GI = GameInstance->GetSubsystem<ULCGameInstanceSubsystem>())
				{
					if (ULCUIManager* UIManager = GI->GetUIManager())
					{
						UIManager->SetLastShopInteractor(this); 
						UIManager->ShowShopPopup();

						ShopWidgetComponent->SetVisibility(false);
					}
				}
			}
		});

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle, 
		TimerDel,
		0.5f, 
		false);
}

UWidgetComponent* AShopInteractor::GetShopWidgetComponent() const
{
	return ShopWidgetComponent;
}
