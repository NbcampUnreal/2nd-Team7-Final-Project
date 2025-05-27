#include "Actor/MapSelectInteractor.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"

#include "LastCanary.h"

AMapSelectInteractor::AMapSelectInteractor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	MapSelectWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("MapSelectWidgetComponent"));
	MapSelectWidgetComponent->SetupAttachment(RootComponent);
	MapSelectWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	MapSelectWidgetComponent->SetDrawSize(FVector2D(800, 600));
	MapSelectWidgetComponent->SetVisibility(true);
}

void AMapSelectInteractor::BeginPlay()
{
	Super::BeginPlay();

	if (MapSelectWidgetComponent && MapSelectWidgetClass)
	{
		MapSelectWidgetComponent->SetWidgetClass(MapSelectWidgetClass);
	}
}

void AMapSelectInteractor::Interact_Implementation(APlayerController* InteractingPlayerController)
{
	if (InteractingPlayerController == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Cannot interact: no controller"));
		return;
	}
	if (MapSelectCamera == nullptr)
	{
		LOG_Frame_WARNING(TEXT("Cannot interact: no camera"));
		return;
	}

	InteractingPlayerController->SetViewTargetWithBlend(MapSelectCamera, 0.5f);

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;
	TimerDel.BindLambda([this, InteractingPlayerController]()
		{
			if (UGameInstance* GameInstance = InteractingPlayerController->GetGameInstance())
			{
				if (ULCGameInstanceSubsystem* GI = GameInstance->GetSubsystem<ULCGameInstanceSubsystem>())
				{
					if (ULCUIManager* UIManager = GI->GetUIManager())
					{
						UIManager->SetLastMapSelectInteractor(this);
						UIManager->ShowMapSelectPopup(); 

						MapSelectWidgetComponent->SetVisibility(false);
					}
				}
			}
		});

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		TimerDel,
		0.5f,
		false
	);
}

FString AMapSelectInteractor::GetInteractMessage_Implementation() const
{
	return TEXT("Press [F] to Manipulate Gate");
}

UWidgetComponent* AMapSelectInteractor::GetMapSelectWidgetComponent() const
{
	return MapSelectWidgetComponent;
}