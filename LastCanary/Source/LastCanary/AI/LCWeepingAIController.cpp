// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LCWeepingAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ALCWeepingAIController::ALCWeepingAIController()
{
	// AIPerceptionComponent는 BaseMonsterCharacter에서 이미 생성하므로 여기서 생성하지 않음
	// OnPossess에서 Pawn의 AIPerceptionComponent를 가져와서 사용
	AIPerceptionComp = nullptr;
	SightConfig = nullptr;
}

void ALCWeepingAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Pawn에서 AIPerceptionComponent 가져오기
	if (InPawn)
	{
		AIPerceptionComp = InPawn->FindComponentByClass<UAIPerceptionComponent>();
		
		if (AIPerceptionComp)
		{
			// SightConfig 생성 및 설정
			SightConfig = NewObject<UAISenseConfig_Sight>(this, TEXT("SightConfig"));
			if (SightConfig)
			{
				SightConfig->SightRadius = SightRadius;
				SightConfig->LoseSightRadius = LoseSightRadius;
				SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;

				SightConfig->DetectionByAffiliation.bDetectEnemies = true;
				SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
				SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

				AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
				AIPerceptionComp->ConfigureSense(*SightConfig);
			}

			AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ALCWeepingAIController::OnTargetPerceptionUpdated);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		CheckTimerHandle,
		this,
		&ALCWeepingAIController::CheckAllPlayersLooking,
		PlayerCheckInterval,
		true
	);
}

void ALCWeepingAIController::CheckAllPlayersLooking()
{
	bool bShouldStopMoving = false;
	
	APawn* ClosestPlayer = nullptr;
	float MinDistanceSquared = FLT_MAX;
	
	UWorld* World = GetWorld();
	APawn* MonsterPawn = GetPawn();

	if (!World || !MonsterPawn) return;

	FVector MonsterLocation = MonsterPawn->GetActorLocation();

	// ��� �÷��̾� ��Ʈ�ѷ��� ��ȸ
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->GetPawn())
		{
			APawn* PlayerPawn = PC->GetPawn();

			// ���� �˻� (Dot Product)
			FVector PlayerForwardVector = PlayerPawn->GetActorForwardVector();
			FVector PlayerToMonsterVector = MonsterLocation - PlayerPawn->GetActorLocation();
			PlayerToMonsterVector.Normalize();

			// ���� ����� �÷��̾� Ž��(���� �Ÿ�)
			FVector PlayerLocation = PlayerPawn->GetActorLocation();
			float DistSquared = FVector::DistSquared(MonsterLocation, PlayerLocation);
			if (DistSquared < MinDistanceSquared)
			{
				MinDistanceSquared = DistSquared;
				ClosestPlayer = PlayerPawn;
			}

			// �÷��̾ ���͸� �ٶ󺸰� �ִ��� (�Ӱ谪���� ������ ū��)
			if (FVector::DotProduct(PlayerForwardVector, PlayerToMonsterVector) > LookAtDotProductThreshold)
			{
				// �þ� ���θ��� �˻� (Line Trace)
				FHitResult HitResult;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(MonsterPawn); // ���� �ڽ� ����
				Params.AddIgnoredActor(PlayerPawn); // �÷��̾� ����

				bool bHit = World->LineTraceSingleByChannel(
					HitResult,
					PlayerPawn->GetPawnViewLocation(),
					MonsterLocation,
					ECollisionChannel::ECC_Visibility, 
					Params
				);
				if (!bHit)
				{
					bShouldStopMoving = true;
				}
			}
		}
	}
	if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		BlackboardComp->SetValueAsBool(TEXT("IsAnyPlayerLooking"), bShouldStopMoving);

		if (ClosestPlayer)
		{
			BlackboardComp->SetValueAsObject(TEXT("TargetActor"), ClosestPlayer);

			// 플레이어에게 도달했고, 아무도 안 바라보고 있으면 즉사
			const float DistToClosestPlayer = FMath::Sqrt(MinDistanceSquared);
			if (DistToClosestPlayer <= KillRange && !bShouldStopMoving)
			{
				UE_LOG(LogTemp, Warning, TEXT("[WeepingAngel] Player %s is in kill range (%.1f) and no one is looking! INSTANT KILL!"),
					*ClosestPlayer->GetName(), DistToClosestPlayer);
				
				// TODO: 여기에 플레이어 즉사 로직 추가
				// ClosestPlayer->TakeDamage(...) 또는 Kill 함수 호출
			}
		}
	}
}

void ALCWeepingAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}