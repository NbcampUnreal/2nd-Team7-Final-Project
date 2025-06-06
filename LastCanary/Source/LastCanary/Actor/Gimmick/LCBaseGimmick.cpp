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
	, InteractMessage(TEXT("Press F"))
	, InteractSound(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	SetRootComponent(VisualMesh);

	DetectionArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionArea"));
	DetectionArea->SetupAttachment(RootComponent);
	DetectionArea->SetBoxExtent(FVector(100.f));
	DetectionArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectionArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionArea->SetHiddenInGame(false); 

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
}

#pragma region Overlap

void ALCBaseGimmick::OnActorEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!AttachedActors.Contains(OtherActor))
	{
		AttachedActors.Add(OtherActor);
	}
	LOG_Art(Log, TEXT("%s ▶ 감지 영역 진입: %s"), *GetName(), *GetNameSafe(OtherActor));
}

void ALCBaseGimmick::OnActorExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 회전/이동 중일 때는 제거하지 않음
	const bool bIsBusy = ILCGimmickInterface::Execute_IsGimmickBusy(this);

	if (!bIsBusy)
	{
		AttachedActors.Remove(OtherActor);
		LOG_Art(Log, TEXT("%s ▶ 감지 영역 이탈: %s"), *GetName(), *GetNameSafe(OtherActor));
	}
	else
	{
		LOG_Art(Log, TEXT("%s ▶ 감지 영역 이탈 시도 → 기믹 동작 중이므로 유지: %s"), *GetName(), *GetNameSafe(OtherActor));
	}
}

#pragma endregion

#pragma region Interact

void ALCBaseGimmick::Interact_Implementation(APlayerController* Interactor)
{
	if (!HasAuthority())
	{
		if (IsValid(Interactor))
		{
			Cast<ABasePlayerController>(Interactor)->InteractGimmick(this);
		}
		return;
	}

	for (AActor* Target : LinkedTargets)
	{
		if (IsValid(Target) && Target->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
		{
			if (ILCGimmickInterface::Execute_CanActivate(Target))
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
