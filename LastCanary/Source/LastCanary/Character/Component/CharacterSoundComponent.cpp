#include "Character/Component/CharacterSoundComponent.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

UCharacterSoundComponent::UCharacterSoundComponent()
{
}

void UCharacterSoundComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterSoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCharacterSoundComponent::Client_PlayHitSound_Implementation()
{
	if (GetCharacter()->IsLocallyControlled())
	{
		UGameplayStatics::PlaySoundAtLocation(this, OnHitSound, GetCharacter()->GetActorLocation());
	}
}
