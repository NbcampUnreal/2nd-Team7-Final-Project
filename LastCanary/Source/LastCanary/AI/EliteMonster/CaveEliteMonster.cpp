#include "AI/EliteMonster/CaveEliteMonster.h"
#include "GameFramework/CharacterMovementComponent.h"

ACaveEliteMonster::ACaveEliteMonster()
{
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
}