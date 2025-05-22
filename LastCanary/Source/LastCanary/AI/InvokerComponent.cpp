#include "AI/InvokerComponent.h"

UInvokerComponent::UInvokerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	NavGenerationradius = 10.0f;
	NavRemovalradius = 15.0f;

	NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
	NavInvoker->SetGenerationRadii(NavGenerationradius, NavRemovalradius);
}

void UInvokerComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UInvokerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}