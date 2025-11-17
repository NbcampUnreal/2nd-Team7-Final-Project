#include "AI/Summon/UndeadMinion.h"
#include "GameFramework/CharacterMovementComponent.h"

AUndeadMinion::AUndeadMinion()
{
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;
}