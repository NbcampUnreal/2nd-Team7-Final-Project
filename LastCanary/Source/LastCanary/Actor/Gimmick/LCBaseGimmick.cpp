#include "Actor/Gimmick/LCBaseGimmick.h"
#include "Interface/GimmickEffectInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Character/BasePlayerController.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"
#include "Camera/CameraActor.h"

ALCBaseGimmick::ALCBaseGimmick()
	: bActivated(false)
	, LastActivatedTime(-999.f)
	, CooldownTime(2.f)
	, bToggleState(true)
	, ReturnDelay(3.f)
	, bDestructibleByGun(false)
	, DestructibleHealth(3.f)
	, CurrentHealth(3.f)
	, InteractMessage(TEXT(""))
	, InteractSound(nullptr)
	, RequiredCount(1.f)
	, ActivationDelay(1.5f)
	, ActivationType(EGimmickActivationType::ActivateOnPress) 
	, bEnableCutscene(false)
	, bCallReturnToInitialStateInsteadOfActivate(false)
	, bCutsceneForAllPlayers(true)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	VisualMesh = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(VisualMesh);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetMobility(EComponentMobility::Movable);

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

	if (bDestructibleByGun)
	{
		CurrentHealth = DestructibleHealth;
	}

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

	// ✅ Trigger용 쿨타임 검사 (WhileStepping 제외)
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float Elapsed = CurrentTime - LastActivatedTime;

	if (ActivationType != EGimmickActivationType::ActivateWhileStepping && Elapsed < CooldownTime)
	{
		//LOG_Art(Log, TEXT("[트리거] 쿨타임 진행 중 → 무시 (%.2f/%.2f)"), Elapsed, CooldownTime);
		return;
	}

	if (!OverlappingActors.Contains(OtherActor))
	{
		OverlappingActors.Add(OtherActor);
	}

	switch (ActivationType)
	{
	case EGimmickActivationType::ActivateOnStep:
	{
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
		LastActivatedTime = CurrentTime;
	}
	break;

	case EGimmickActivationType::ActivateWhileStepping:
	{
		if (OverlappingActors.Num() >= RequiredCount)
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
	{
		if (OverlappingActors.Num() >= RequiredCount)
		{
			GetWorld()->GetTimerManager().SetTimer(
				ActivationDelayHandle,
				[this]()
				{
					if (OverlappingActors.Num() >= RequiredCount)
					{
						const float CurrentTime = GetWorld()->GetTimeSeconds();
						const float Elapsed = CurrentTime - LastActivatedTime;

						// ✅ Delay 후에도 쿨타임 검사
						if (Elapsed >= CooldownTime)
						{
							if (bCallReturnToInitialStateInsteadOfActivate)
							{
								ILCGimmickInterface::Execute_ReturnToInitialState(this);
							}
							else
							{
								ILCGimmickInterface::Execute_ActivateGimmick(this);
							}

							LastActivatedTime = CurrentTime;
						}
						// else { LOG_Art(Log, TEXT("[딜레이] 쿨타임 중 → 작동 안함")); }
					}
				},
				ActivationDelay,
				false
			);
		}
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
	{
		if (bActivated && OverlappingActors.Num() < RequiredCount)
		{
			ILCGimmickInterface::Execute_DeactivateGimmick(this);

			if (!bToggleState)
			{
				ILCGimmickInterface::Execute_ReturnToInitialState(this);
			}
		}
	}
	break;

	case EGimmickActivationType::ActivateAfterDelay:
	{
		GetWorld()->GetTimerManager().ClearTimer(ActivationDelayHandle);

		if (!bToggleState)
		{
			bActivated = false;
		}
	}
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
			//LOG_Art(Log, TEXT("[감지] 감지 성공 - 태그: %s"), *Tag.ToString());
			return true;
		}
	}

	//LOG_Art(Log, TEXT("[감지] 감지 실패 - 태그 없음"));
	return false;
}

#pragma endregion

#pragma region Interact

void ALCBaseGimmick::Interact_Implementation(APlayerController* Interactor)
{
	if (ActivationType != EGimmickActivationType::ActivateOnPress)
	{
		//LOG_Art_WARNING(TEXT("이 기믹은 상호작용 타입이 아님"));
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

	// 컷신 시작 (기믹 활성화와 동시에)
	if (bEnableCutscene && CutsceneCamera)
	{
		if (bCutsceneForAllPlayers)
		{
			// 모든 플레이어에게 컷신 보여주기
			Multicast_StartCutscene();
		}
		else
		{
			// 특정 플레이어에게만 컷신 보여주기 (상호작용한 플레이어)
			if (APawn* InstigatingPawn = Cast<APawn>(GetOwner()))
			{
				if (APlayerController* PC = Cast<APlayerController>(InstigatingPawn->GetController()))
				{
					Multicast_StartCutsceneForSpecificPlayer(PC);
				}
			}
		}
	}

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

#pragma region Cutscene

void ALCBaseGimmick::Multicast_StartCutscene_Implementation()
{
	if (!CutsceneCamera) return;

	// 모든 플레이어 컨트롤러에 대해 컷신 시작
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (APlayerController* PC = Iterator->Get())
			{
				StartCutsceneForPlayer(PC);
			}
		}
	}
}

void ALCBaseGimmick::Multicast_StartCutsceneForSpecificPlayer_Implementation(APlayerController* PC)
{
	if (!CutsceneCamera || !PC) return;

	StartCutsceneForPlayer(PC);
}

void ALCBaseGimmick::StartCutsceneForPlayer(APlayerController* PC)
{
	if (!PC || !CutsceneCamera) return;

	// 로컬 플레이어만 컷신 처리 (네트워크 환경에서 각자의 화면만 제어)
	if (!PC->IsLocalController()) return;

	// 이미 컷신 중이면 무시
	if (IsPlayingCutscene()) return;

	// 현재 카메라 저장 (복원용)
	OriginalViewTarget = PC->GetViewTarget();

	// 컷신 카메라로 전환
	PC->SetViewTargetWithBlend(CutsceneCamera, CameraBlendTime, VTBlend_Linear);

	// 플레이어 입력 비활성화 (옵션)
	if (APawn* PlayerPawn = PC->GetPawn())
	{
		PlayerPawn->DisableInput(PC);
		bPlayerInputDisabled = true;
	}

	// 컷신 플래그 설정
	bIsPlayingCutscene = true;

	// 컷신 종료 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		CutsceneTimer,
		[this, PC]()
		{
			EndCutsceneForPlayer(PC);
		},
		CutsceneDuration,
		false
	);

	// 컷신 시작 이벤트 (블루프린트에서 확장 가능)
	OnCutsceneStarted(PC);
}

void ALCBaseGimmick::EndCutsceneForPlayer(APlayerController* PC)
{
	if (!PC) return;

	// 로컬 플레이어만 처리
	if (!PC->IsLocalController()) return;

	// 컷신 중이 아니면 무시
	if (!IsPlayingCutscene()) return;

	// 원래 카메라로 복원
	if (OriginalViewTarget)
	{
		PC->SetViewTargetWithBlend(OriginalViewTarget, CameraBlendTime, VTBlend_Linear);
	}
	else
	{
		// 원래 뷰 타겟이 없으면 플레이어 폰으로 복원
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			PC->SetViewTargetWithBlend(PlayerPawn, CameraBlendTime, VTBlend_Linear);
		}
	}

	// 플레이어 입력 복원
	if (bPlayerInputDisabled)
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			PlayerPawn->EnableInput(PC);
		}
		bPlayerInputDisabled = false;
	}

	// 컷신 플래그 해제
	bIsPlayingCutscene = false;
	OriginalViewTarget = nullptr;

	// 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(CutsceneTimer);

	// 컷신 종료 이벤트 (블루프린트에서 확장 가능)
	OnCutsceneEnded(PC);
}

bool ALCBaseGimmick::IsPlayingCutscene() const
{
	return bIsPlayingCutscene;
}

void ALCBaseGimmick::StopCutscene()
{
	if (!IsPlayingCutscene()) return;

	// 모든 플레이어에 대해 컷신 강제 종료
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (APlayerController* PC = Iterator->Get())
			{
				EndCutsceneForPlayer(PC);
			}
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
		//LOG_Art_WARNING(TEXT("Server_ActivateGimmick -> Interactor (Owner) 유효하지 않음"));
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

void ALCBaseGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

}

#pragma endregion

float ALCBaseGimmick::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || !bDestructibleByGun)
		return 0.f;

	CurrentHealth -= DamageAmount;

	//LOG_Art(Log, TEXT(" Gimmick 피격: %.1f 데미지 → 남은 체력: %.1f"), DamageAmount, CurrentHealth);

	if (CurrentHealth <= 0.f)
	{
		OnDestroyedByBullet(); 
	}

	return DamageAmount;
}

void ALCBaseGimmick::OnDestroyedByBullet_Implementation()
{
	Multicast_PlayDestroySound();
	Destroy();
}

void ALCBaseGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALCBaseGimmick, CurrentHealth);
}

void ALCBaseGimmick::Multicast_PlayDestroySound_Implementation()
{
	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroySound, GetActorLocation());
	}
}