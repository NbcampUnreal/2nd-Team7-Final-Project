#include "AI/BaseMonsterCharacter.h"

ABaseMonsterCharacter::ABaseMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABaseMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

