#include "AI/HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/DamageType.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    AActor* Owner = GetOwner();
    if (Owner)
    {
        Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OnTakeAnyDamage);
    }
}

void UHealthComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f || bIsDead)
        return;
    if (GetOwnerRole() != ROLE_Authority)
        return;
    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(this, CurrentHealth, -Damage, DamageCauser);
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath.Broadcast(GetOwner(), InstigatedBy, DamageCauser);
        ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
        if (OwnerCharacter)
        {
            OwnerCharacter->GetMesh()->SetSimulatePhysics(true);
            OwnerCharacter->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
            OwnerCharacter->SetActorEnableCollision(false);
        }
    }
}

void UHealthComponent::TakeDamage(float Damage, AController* InstigatedBy, AActor* DamageCauser)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        ServerTakeDamage(Damage, InstigatedBy, DamageCauser);
        return;
    }

    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Make sure to create a properly defined FDamageEvent
        //FDamageEvent DamageEvent;
        //Owner->TakeDamage(Damage, DamageEvent, InstigatedBy, DamageCauser);
    }
}

void UHealthComponent::ServerTakeDamage_Implementation(float Damage, AController* InstigatedBy, AActor* DamageCauser)
{
    TakeDamage(Damage, InstigatedBy, DamageCauser);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UHealthComponent, CurrentHealth);
    DOREPLIFETIME(UHealthComponent, MaxHealth);
    DOREPLIFETIME(UHealthComponent, bIsDead);
}