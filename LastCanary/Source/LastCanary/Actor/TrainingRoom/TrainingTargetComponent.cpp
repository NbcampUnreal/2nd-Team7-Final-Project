#include "Actor/TrainingRoom/TrainingTargetComponent.h"
#include "Actor/TrainingRoom/TrainingRoomManager.h"
#include "Item/Component/DamageReceiverComponent.h"
#include "GameFramework/Actor.h"
#include "LastCanary.h"

UTrainingTargetComponent::UTrainingTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UTrainingTargetComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeInternal();
}

void UTrainingTargetComponent::InitializeInternal()
{
	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return;
	}

	InitialLocation = Owner->GetActorLocation();

	DamageReceiver = Owner->FindComponentByClass<UDamageReceiverComponent>();

	if (DamageReceiver)
	{
		DamageReceiver->OnDamageReceived.AddDynamic(this, &UTrainingTargetComponent::OnDamageReceived);
		DamageReceiver->OnHealthDepleted.AddDynamic(this, &UTrainingTargetComponent::OnHealthDepleted);
	}
}


void UTrainingTargetComponent::SetTrainingManager(ATrainingRoomManager* NewManager)
{
	TrainingManager = NewManager;
}

void UTrainingTargetComponent::UpdateInitialLocation(const FVector& NewLocation)
{
	InitialLocation = NewLocation;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->SetActorLocation(NewLocation);
	}
}


void UTrainingTargetComponent::SetTargetEnabled(bool bEnabled)
{
	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return;
	}

	Owner->SetActorHiddenInGame(!bEnabled);
	Owner->SetActorEnableCollision(bEnabled);
	Owner->SetActorTickEnabled(bEnabled);
}


void UTrainingTargetComponent::ResetTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 체력 리셋
	if (DamageReceiver)
	{
		DamageReceiver->SetHealth(DamageReceiver->MaxHealth);
	}

	// 위치 초기화
	Owner->SetActorLocation(InitialLocation);

	// 활성화
	SetTargetEnabled(true);
}


void UTrainingTargetComponent::OnDamageReceived(AActor* DamagedActor, float DamageAmount, const FHitResult& HitInfo, AActor* DamageCauser)
{
	if (TrainingManager)
	{
		TrainingManager->OnDummyHit(GetOwner(), DamageAmount, HitInfo);
	}
}

void UTrainingTargetComponent::OnHealthDepleted(AActor* DamagedActor)
{
	if (TrainingManager && DamageReceiver)
	{
		TrainingManager->OnDummyDestroyed(GetOwner(), DamageReceiver->MaxHealth);
	}

	SetTargetEnabled(false);
}

