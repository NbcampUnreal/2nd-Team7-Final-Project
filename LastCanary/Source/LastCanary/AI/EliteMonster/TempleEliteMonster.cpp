#include "AI/EliteMonster/TempleEliteMonster.h"
#include "GameFramework/CharacterMovementComponent.h"

ATempleEliteMonster::ATempleEliteMonster()
{
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
}