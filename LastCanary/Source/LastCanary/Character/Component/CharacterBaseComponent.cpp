#include "Character/Component/CharacterBaseComponent.h"
#include "Character/BaseCharacter.h"

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
	CachedCharacter = Cast<ABaseCharacter>(GetOwner());
	// ...
	
}


// Called every frame
void UCharacterBaseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

