#include "AI/EliteMonster/TempleEliteMonster.h"
#include "GameFramework/CharacterMovementComponent.h"

ATempleEliteMonster::ATempleEliteMonster()
{
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}