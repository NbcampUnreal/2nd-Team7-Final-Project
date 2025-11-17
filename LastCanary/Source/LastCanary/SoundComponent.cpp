#include "SoundComponent.h"

USoundComponent::USoundComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void USoundComponent::BeginPlay()
{
	Super::BeginPlay();

}

void USoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

