#include "Actor/Gimmick/LCBaseGimmick.h"
#include "Interface/GimmickEffectInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Character/BasePlayerController.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCBaseGimmick::ALCBaseGimmick()
	: bActivated(false)
	, LastActivatedTime(-999.f)
	, CooldownTime(2.f)
	, bToggleState(true)
	, ReturnDelay(3.f)
	, InteractMessage(TEXT("???"))
	, InteractSound(nullptr)
	, RequiredCount(1.f)
	, ActivationDelay(1.5f)
	, ActivationType(EGimmickActivationType::ActivateOnPress) 
	, bCallReturnToInitialStateInsteadOfActivate(false)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	VisualMesh = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(VisualMesh);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);

	DetectionArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionArea"));
	DetectionArea->SetupAttachment(RootComponent);
	DetectionArea->SetBoxExtent(FVector(100.f));
	DetectionArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectionArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionArea->SetHiddenInGame(false); 

	ActivationTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationTrigger"));
	ActivationTrigger->SetupAttachment(RootComponent);
	ActivationTrigger->SetBoxExtent(FVector(100.f));
	ActivationTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActivationTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	ActivationTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ActivationTrigger->SetHiddenInGame(false);

	bEnableActorDetection = false;
}

void ALCBaseGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableActorDetection && IsValid(DetectionArea))
	{
		DetectionArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		DetectionArea->OnComponentBeginOverlap.AddDynamic(this, &ALCBaseGimmick::OnActorEnter);
		DetectionArea->OnComponentEndOverlap.AddDynamic(this, &ALCBaseGimmick::OnActorExit);
	}

	if (IsValid(ActivationTrigger))
	{
		ActivationTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALCBaseGimmick::OnTriggerEnter);
		ActivationTrigger->OnComponentEndOverlap.AddDynamic(this, &ALCBaseGimmick::OnTriggerExit);
	}

	if (ActivationType == EGimmickActivationType::ActivateOnConditionMet)
	{
		GetWorldTimerManager().SetTimer(
			ConditionCheckTimer,
			this,
			&ALCBaseGimmick::CheckConditionAndActivate,
			ConditionCheckInterval,
			true
		);
		CheckConditionAndActivate();
	}

}

#pragma region Overlap

void ALCBaseGimmick::OnActorEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!AttachedActors.Contains(OtherActor))
	{
		AttachedActors.Add(OtherActor);
	}
}

void ALCBaseGimmick::OnActorExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const bool bIsBusy = ILCGimmickInterface::Execute_IsGimmickBusy(this);

	if (!bIsBusy)
	{
		AttachedActors.Remove(OtherActor);
	}
}
#pragma endregion

#pragma region TriggerOverlap

//void ALCBaseGimmick::OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
//	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//	if (!HasAuthority() || !IsValid(OtherActor)) return;
//
//	if (!OverlappingActors.Contains(OtherActor))
//	{
//		OverlappingActors.Add(OtherActor);
//	}
//
//	switch (ActivationType)
//	{
//	case EGimmickActivationType::ActivateOnStep:
//		if (bCallReturnToInitialStateInsteadOfActivate)
//		{
//			ILCGimmickInterface::Execute_ReturnToInitialState(this);
//			for (AActor* Target : LinkedTargets)
//			{
//				ILCGimmickInterface::Execute_ReturnToInitialState(Target);
//			}
//		}
//		else
//		{
//			ILCGimmickInterface::Execute_ActivateGimmick(this);
//			for (AActor* Target : LinkedTargets)
//			{
//				ILCGimmickInterface::Execute_ActivateGimmick(Target);
//			}
//		}
//		break;
//
//	case EGimmickActivationType::ActivateWhileStepping:
//		if (!bActivated && OverlappingActors.Num() >= RequiredCount)
//		{
//			if (ILCGimmickInterface::Execute_CanActivate(this))
//			{
//				if (bCallReturnToInitialStateInsteadOfActivate)
//				{
//					ILCGimmickInterface::Execute_ReturnToInitialState(this);
//				}
//				else
//				{
//					ILCGimmickInterface::Execute_ActivateGimmick(this);
//				}
//			}
//		}
//		break;
//
//	case EGimmickActivationType::ActivateAfterDelay:
//		if (!bActivated && OverlappingActors.Num() >= RequiredCount)
//		{
//			GetWorld()->GetTimerManager().SetTimer(
//				ActivationDelayHandle,
//				[this]()
//				{
//					if (!bActivated && OverlappingActors.Num() >= RequiredCount)
//					{
//						if (bCallReturnToInitialStateInsteadOfActivate)
//						{
//							ILCGimmickInterface::Execute_ReturnToInitialState(this);
//						}
//						else
//						{
//							ILCGimmickInterface::Execute_ActivateGimmick(this);
//						}
//					}
//				},
//				ActivationDelay,
//				false
//			);
//		}
//		break;
//
//	default:
//		break;
//	}
//}
//
//void ALCBaseGimmick::OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
//	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if (!HasAuthority() || !IsValid(OtherActor)) return;
//
//	OverlappingActors.Remove(OtherActor);
//
//	switch (ActivationType)
//	{
//	case EGimmickActivationType::ActivateWhileStepping:
//		if (bActivated && OverlappingActors.Num() < RequiredCount)
//		{
//			ILCGimmickInterface::Execute_DeactivateGimmick(this);
//
//			if (!bToggleState)
//			{
//				ILCGimmickInterface::Execute_ReturnToInitialState(this);
//			}
//		}
//		break;
//
//	case EGimmickActivationType::ActivateAfterDelay:
//		GetWorld()->GetTimerManager().ClearTimer(ActivationDelayHandle);
//		break;
//
//	default:
//		break;
//	}
//
//}

void ALCBaseGimmick::OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !IsValid(OtherActor)) return;
	if (!IsValidActivator(OtherActor)) return;

	if (!OverlappingActors.Contains(OtherActor))
	{
		OverlappingActors.Add(OtherActor);
	}

	switch (ActivationType)
	{
	case EGimmickActivationType::ActivateOnStep:
		if (bCallReturnToInitialStateInsteadOfActivate)
		{
			ILCGimmickInterface::Execute_ReturnToInitialState(this);
			for (AActor* Target : LinkedTargets)
			{
				ILCGimmickInterface::Execute_ReturnToInitialState(Target);
			}
		}
		else
		{
			ILCGimmickInterface::Execute_ActivateGimmick(this);
			for (AActor* Target : LinkedTargets)
			{
				ILCGimmickInterface::Execute_ActivateGimmick(Target);
			}
		}
		break;

	case EGimmickActivationType::ActivateWhileStepping:
		if (!bActivated && OverlappingActors.Num() >= RequiredCount)
		{
			if (ILCGimmickInterface::Execute_CanActivate(this))
			{
				if (bCallReturnToInitialStateInsteadOfActivate)
				{
					ILCGimmickInterface::Execute_ReturnToInitialState(this);
				}
				else
				{
					ILCGimmickInterface::Execute_ActivateGimmick(this);
				}
			}
		}
		break;

	case EGimmickActivationType::ActivateAfterDelay:
		if (!bActivated && OverlappingActors.Num() >= RequiredCount)
		{
			GetWorld()->GetTimerManager().SetTimer(
				ActivationDelayHandle,
				[this]()
				{
					if (!bActivated && OverlappingActors.Num() >= RequiredCount)
					{
						if (bCallReturnToInitialStateInsteadOfActivate)
						{
							ILCGimmickInterface::Execute_ReturnToInitialState(this);
						}
						else
						{
							ILCGimmickInterface::Execute_ActivateGimmick(this);
						}
					}
				},
				ActivationDelay,
				false
			);
		}
		break;

	default:
		break;
	}
}

void ALCBaseGimmick::OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority() || !IsValid(OtherActor)) return;
	if (!IsValidActivator(OtherActor)) return;

	OverlappingActors.Remove(OtherActor);

	switch (ActivationType)
	{
	case EGimmickActivationType::ActivateWhileStepping:
		if (bActivated && OverlappingActors.Num() < RequiredCount)
		{
			ILCGimmickInterface::Execute_DeactivateGimmick(this);

			if (!bToggleState)
			{
				ILCGimmickInterface::Execute_ReturnToInitialState(this);
			}
		}
		break;

	case EGimmickActivationType::ActivateAfterDelay:
		GetWorld()->GetTimerManager().ClearTimer(ActivationDelayHandle);
		break;

	default:
		break;
	}
}

bool ALCBaseGimmick::IsValidActivator(AActor* OtherActor) const
{
	if (!IsValid(OtherActor)) return false;

	for (const FName& Tag : ValidActivatorTags)
	{
		if (OtherActor->ActorHasTag(Tag))
		{
			LOG_Art(Log, TEXT("[감지] 감지 성공 - 태그: %s"), *Tag.ToString());
			return true;
		}
	}

	LOG_Art(Log, TEXT("[감지] 감지 실패 - 태그 없음"));
	return false;
}

#pragma endregion

#pragma region Interact

void ALCBaseGimmick::Interact_Implementation(APlayerController* Interactor)
{
	if (ActivationType != EGimmickActivationType::ActivateOnPress)
	{
		LOG_Art_WARNING(TEXT("❌ 이 기믹은 상호작용 타입이 아님"));
		return;
	}

	if (!HasAuthority())
	{
		if (IsValid(Interactor))
		{
			Cast<ABasePlayerController>(Interactor)->InteractGimmick(this);
		}
		return;
	}

	TArray<AActor*> Targets = LinkedTargets;
	if (Targets.Num() == 0)
	{
		Targets.Add(this);
	}

	for (AActor* Target : Targets)
	{
		if (!IsValid(Target)) continue;

		if (Target->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
		{
			if (bCallReturnToInitialStateInsteadOfActivate)
			{
				ILCGimmickInterface::Execute_ReturnToInitialState(Target);
			}
			else if (ILCGimmickInterface::Execute_CanActivate(Target))
			{
				ILCGimmickInterface::Execute_ActivateGimmick(Target);
			}
		}
	}
}

bool ALCBaseGimmick::IsGimmickBusy_Implementation()
{
	return false;
}

FString ALCBaseGimmick::GetInteractMessage_Implementation() const
{
	return InteractMessage;
}

#pragma endregion

#pragma region Condition / Auto

bool ALCBaseGimmick::IsConditionMet_Implementation() const
{
	return false;
}

void ALCBaseGimmick::CheckConditionAndActivate()
{
	if (!HasAuthority())
	{
		return;
	}

	if (IsConditionMet())
	{
		ILCGimmickInterface::Execute_ActivateGimmick(this);
	}
}

void ALCBaseGimmick::ActivateLoopedGimmick()
{
	if (ILCGimmickInterface::Execute_CanActivate(this))
	{
		ILCGimmickInterface::Execute_ActivateGimmick(this);
	}
}

#pragma endregion

#pragma region Activation

bool ALCBaseGimmick::CanActivate_Implementation()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float Elapsed = CurrentTime - LastActivatedTime;

	if (bActivated && Elapsed >= CooldownTime)
	{
		bActivated = false;
	}

	return !bActivated;
}

void ALCBaseGimmick::ActivateGimmick_Implementation()
{
	if (!HasAuthority())
	{
		Server_ActivateGimmick();
		return;
	}

	if (!ILCGimmickInterface::Execute_CanActivate(this))
	{
		return;
	}

	LastActivatedTime = GetWorld()->GetTimeSeconds();
	bActivated = true;

	Multicast_PlaySound();

	for (AActor* Target : LinkedTargets)
	{
		if (IsValid(Target) && Target->GetClass()->ImplementsInterface(UGimmickEffectInterface::StaticClass()))
		{
			IGimmickEffectInterface::Execute_TriggerEffect(Target);
		}
	}
}

void ALCBaseGimmick::DeactivateGimmick_Implementation()
{
	bActivated = false;

	for (AActor* Target : LinkedTargets)
	{
		if (IsValid(Target) && Target->GetClass()->ImplementsInterface(UGimmickEffectInterface::StaticClass()))
		{
			IGimmickEffectInterface::Execute_StopEffect(Target);
		}
	}
}

void ALCBaseGimmick::ReturnToInitialState_Implementation()
{

}

#pragma endregion

#pragma region Network

void ALCBaseGimmick::Server_ActivateGimmick_Implementation()
{
	APlayerController* Interactor = Cast<APlayerController>(GetOwner());
	if (!IsValid(Interactor))
	{
		LOG_Art_WARNING(TEXT("Server_ActivateGimmick -> Interactor (Owner) 유효하지 않음"));
		return;
	}

	Interact_Implementation(Interactor);
}

void ALCBaseGimmick::Multicast_PlaySound_Implementation()
{
	if (InteractSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, InteractSound, GetActorLocation());
	}
}

#pragma endregion
