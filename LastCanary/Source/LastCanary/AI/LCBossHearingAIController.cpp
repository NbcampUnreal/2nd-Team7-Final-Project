#include "AI/LCBossHearingAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

ALCBossHearingAIController::ALCBossHearingAIController()
{
	// 1) PerceptionComponent(부모)에 청각만 등록
	if (!PerceptionComponent)
	{
		PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	}

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 1500.f;
	HearingConfig->SetMaxAge(5.f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	PerceptionComponent->ConfigureSense(*HearingConfig);
	PerceptionComponent->SetDominantSense(HearingConfig->GetSenseImplementation());
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALCBossHearingAIController::OnTargetPerceptionUpdated);

	// 2) Blackboard·BehaviorTree 컴포넌트 생성
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ALCBossHearingAIController::BeginPlay()
{
	Super::BeginPlay();

	// 3) BehaviorTreeAsset이 지정되어 있으면 블랙보드 초기화하고 트리 실행
	if (BehaviorTreeAsset)
	{
		BlackboardComp->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);
		BehaviorComp->StartTree(*BehaviorTreeAsset);
	}
}

void ALCBossHearingAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 청각 자극 ID
	static const FAISenseID HearingID = HearingConfig->GetSenseID();

	// 소리를 성공적으로 감지했을 때
	if (Stimulus.Type == HearingID && Stimulus.WasSuccessfullySensed())
	{
		UE_LOG(LogTemp, Log, TEXT("[BansheeAI] 소리 감지: %s"), *Actor->GetName());
		BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
	}
	// 소리 상실 시
	else if (Stimulus.Type == HearingID && !Stimulus.WasSuccessfullySensed())
	{
		UE_LOG(LogTemp, Log, TEXT("[BansheeAI] 소리 상실: %s"), *Actor->GetName());
		BlackboardComp->ClearValue(TEXT("TargetActor"));
	}
}