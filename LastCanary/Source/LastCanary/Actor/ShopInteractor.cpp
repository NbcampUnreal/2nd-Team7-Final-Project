#include "Actor/ShopInteractor.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

#include "UI/UIElement/ShopWidget.h"
#include "UI/Manager/LCUIManager.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameManager.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"

#include "LastCanary.h"

AShopInteractor::AShopInteractor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	DesktopWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DesktopWidgetComponent"));
	DesktopWidgetComponent->SetupAttachment(RootComponent);
	DesktopWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	DesktopWidgetComponent->SetDrawSize(FVector2D(800, 600));
	DesktopWidgetComponent->SetVisibility(true);
}

void AShopInteractor::BeginPlay()
{
	Super::BeginPlay();

	if (DesktopWidgetClass)
	{
		DesktopWidgetComponent->SetWidgetClass(DesktopWidgetClass);
	}
}

void AShopInteractor::Interact_Implementation(APlayerController* InteractingPlayerController)
{
	if (InteractingPlayerController == nullptr)
	{
		return;
	}
	if (DesktopCamera == nullptr)
	{
		return;
	}

	ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(InteractingPlayerController);
	if (IsValid(RoomPC) == false)
	{
		return;
	}

	InteractingPlayerController->SetViewTargetWithBlend(DesktopCamera, 0.5f);

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;

	TimerDel.BindLambda([this, RoomPC]()
		{
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				//UGameDataManager* GM = GameInstance->GetSubsystem<UGameDataManager>();
				if (ULCGameInstanceSubsystem* GI = GameInstance->GetSubsystem<ULCGameInstanceSubsystem>())
				{
					if (ULCUIManager* UIManager = GI->GetUIManager())
					{
						UIManager->SetLastShopInteractor(this);
						LOG_Frame_WARNING(TEXT("PC : %s"), *RoomPC->GetActorNameOrLabel());
						//GM->GetGold();
						//RoomPC->Server_ShowShopWidget();
						//UIManager->ShowShopPopup(GM->GetGold());

						UIManager->ShowDesktop();

						DesktopWidgetComponent->SetVisibility(false);
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

FString AShopInteractor::GetInteractMessage_Implementation() const
{
	if (IA_Interact == nullptr)
	{
		return TEXT("No Interact Key Assigned");
	}

	FString InteractKeyName = GetCurrentKeyNameForAction(IA_Interact);

	return FString::Printf(TEXT("Press [%s] to Use Desktop"), *InteractKeyName);
}

FString AShopInteractor::GetCurrentKeyNameForAction(UInputAction* InputAction) const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(PC) == false)
	{
		return TEXT("Invalid");
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (IsValid(LocalPlayer) == false)
	{
		return TEXT("Invalid");
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (IsValid(Subsystem) == false)
	{
		return TEXT("Invalid");
	}
	const TArray<FEnhancedActionKeyMapping> Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();

	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (Mapping.Action == InputAction)
		{
			return Mapping.Key.GetDisplayName().ToString();
		}
	}
	return TEXT("Unbound");
}

UWidgetComponent* AShopInteractor::GetDesktopWidgetComponent() const
{
	return DesktopWidgetComponent;
}
