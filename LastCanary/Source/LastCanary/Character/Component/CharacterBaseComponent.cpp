#include "Character/Component/CharacterBaseComponent.h"
#include "Character/BaseCharacter.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIElement/InGameHUD.h"

// Sets default values for this component's properties
UCharacterBaseComponent::UCharacterBaseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCharacterBaseComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedPawn = Cast<APawn>(GetOwner());
	
	if (IsValid(CachedPawn))
	{
		CachedController = Cast<APlayerController>(CachedPawn->GetController());
	}


	CachedCharacter = Cast<ACharacter>(CachedPawn);

	CachedBaseCharacter = Cast<ABaseCharacter>(CachedCharacter);

	if (IsValid(GetBaseCharacter()))
	{
		USkeletalMeshComponent* Mesh = CachedCharacter->GetMesh();
		if (IsValid(Mesh))
		{
			CachedAnimInstance = Mesh->GetAnimInstance();
		}
	}
}


// Called every frame
void UCharacterBaseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UInGameHUD* UCharacterBaseComponent::GetInGameHUD()
{
	if (!IsValid(GetBaseCharacter()) || !IsValid(GetPlayerController()))
	{
		return nullptr;
	}
	ULCGameInstanceSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!IsValid(Subsystem))
	{
		return nullptr;
	}
	ULCUIManager* UIManager = Subsystem->GetUIManager();
	if (!IsValid(UIManager))
	{
		return nullptr;
	}
	UInGameHUD* HUD = UIManager->GetInGameHUD();
	if (!IsValid(HUD))
	{
		return nullptr;
	}

	return HUD;
}