#include "AI/LCBossAIController.h"
#include "AI/LCBossMonsterCharacter.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "TimerManager.h"

/* ---------- 정적 BB 키 ---------- */
const FName ALCBossAIController::BBKey_TargetActor(TEXT("TargetActor"));
const FName ALCBossAIController::BBKey_HasLOS(TEXT("HasLOS"));

ALCBossAIController::ALCBossAIController()
{
	/* 컴포넌트 생성 */
	BTComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BTComp"));
	BBComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BBComp"));
	Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));

	/* Sight 설정 */
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = SightFOV;
	SightConfig->DetectionByAffiliation = { true, false, false }; // Enemies only

	Perception->ConfigureSense(*SightConfig);
	Perception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ALCBossAIController::BeginPlay()
{
	Super::BeginPlay();

	Perception->OnTargetPerceptionUpdated.AddDynamic(
		this, &ALCBossAIController::OnTargetPerceptionUpdated);
}

void ALCBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	/* 블랙보드 & BT 실행 */
	if (DefaultBT && DefaultBT->BlackboardAsset)
	{
		BBComp->InitializeBlackboard(*DefaultBT->BlackboardAsset);
		RunBehaviorTree(DefaultBT);
	}

	/* 0.3초마다 LockedTarget 체크 */
	GetWorldTimerManager().SetTimer(
		ValidateTimer, this, &ALCBossAIController::ValidateLockedTarget,
		0.3f, true, 0.3f);
}

/*──────────────── Perception 이벤트 ────────────────*/
void ALCBossAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stim)
{
	/* 최초 감지 → 고정 */
	if (!LockedTarget && Stim.WasSuccessfullySensed())
	{
		LockedTarget = Actor;
		BBComp->SetValueAsObject(BBKey_TargetActor, Actor);
	}

	/* LockedTarget 만 관리 */
	if (Actor != LockedTarget) return;

	const bool bCanSee = Stim.WasSuccessfullySensed() &&
		(Stim.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()));

	BBComp->SetValueAsBool(BBKey_HasLOS, bCanSee);
}

/*──────────────── LockedTarget 유효성 검사 ────────────────*/
void ALCBossAIController::ValidateLockedTarget()
{
	if (!LockedTarget || !IsValid(LockedTarget))
	{
		LockedTarget = nullptr;
		BBComp->ClearValue(BBKey_TargetActor);
		BBComp->SetValueAsBool(BBKey_HasLOS, false);
	}
}