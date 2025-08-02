#include "Actor/Gimmick/Component/DebuffDamageComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Interface/GimmickDebuffInterface.h"
#include "LastCanary.h"

UDebuffDamageComponent::UDebuffDamageComponent()
	: DamageType(EGimmickDamageType::None)
	, DamageValue(10.0f)
	, DamageInterval(1.0f)
	, bDelayRemoveDamage(false)
	, DamageRemoveDelay(0.0f)
	, DebuffType(EGimmickDebuffType::None)
	, DebuffSlowRate(0.5f)
	, bDelayRemoveDebuff(false)
	, DebuffRemoveDelay(0.0f)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDebuffDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddUniqueDynamic(this, &UDebuffDamageComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddUniqueDynamic(this, &UDebuffDamageComponent::OnOverlapEnd);
}

void UDebuffDamageComponent::ApplyEffectToActor(AActor* OtherActor)
{
	if (!IsValid(OtherActor) || AffectedActors.Contains(OtherActor))
	{
		LOG_Art(Log, TEXT("[DebuffComp]  Already affected or invalid: %s"), *GetNameSafe(OtherActor));
		return;
	}

	const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
	if (!TagInterface)
	{
		//LOG_Art_WARNING(TEXT("[DebuffComp]  Target does not implement GameplayTag interface: %s"), *GetNameSafe(OtherActor));
		return;
	}

	FGameplayTagContainer ActorTags;
	TagInterface->GetOwnedGameplayTags(ActorTags);

	//LOG_Art(Log, TEXT("[DebuffComp] Tags of %s → %s"), *OtherActor->GetName(), *ActorTags.ToStringSimple());

	if (DamageType != EGimmickDamageType::None && ActorTags.HasTagExact(RequiredDamageTag))
	{
		LOG_Art(Log, TEXT("[DebuffComp]  Damage condition passed"));

		if (DamageType == EGimmickDamageType::InstantDamage)
		{
			UGameplayStatics::ApplyDamage(OtherActor, DamageValue, nullptr, GetOwner(), nullptr);
			LOG_Art(Log, TEXT("[DebuffComp] Instant Damage %.1f applied to %s"), DamageValue, *OtherActor->GetName());
		}
		else if (DamageType == EGimmickDamageType::DamageOverTime)
		{
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDel;
			TimerDel.BindUFunction(this, FName("ApplyOverTimeDamage"), OtherActor);

			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, DamageInterval, true);
			DamageTimers.Add(OtherActor, TimerHandle);
			LOG_Art(Log, TEXT("[DebuffComp] DOT started for %s"), *OtherActor->GetName());
		}
	}
	else
	{
		LOG_Art(Log, TEXT("[DebuffComp]  Damage condition failed for %s"), *OtherActor->GetName());
	}

	if (DebuffType != EGimmickDebuffType::None && ActorTags.HasTagExact(RequiredDebuffTag))
	{
		if (IGimmickDebuffInterface* DebuffActor = Cast<IGimmickDebuffInterface>(OtherActor))
		{
			LOG_Art(Log, TEXT("[DebuffComp] ApplyMovementDebuff 호출 → %s"), *GetNameSafe(OtherActor));
			DebuffActor->ApplyMovementDebuff_Implementation(DebuffSlowRate, -1.f);
		}
		else
		{
			LOG_Art_WARNING(TEXT("[DebuffComp] 인터페이스 캐스팅 실패 → %s"), *GetNameSafe(OtherActor));
		}
	}
	else
	{
		LOG_Art(Log, TEXT("[DebuffComp] Debuff condition failed for %s"), *OtherActor->GetName());
	}

	AffectedActors.Add(OtherActor);
}

void UDebuffDamageComponent::OnTargetDestroyed(AActor* DestroyedActor)
{
	if (!IsValid(DestroyedActor)) return;

	if (!AffectedActors.Contains(DestroyedActor))
	{
		return;
	}

	LOG_Art(Log, TEXT("타겟 파괴 감지됨 : 효과 제거: %s"), *GetNameSafe(DestroyedActor));

	StopDamageTimer(DestroyedActor);
	RemoveDebuff(DestroyedActor);
	AffectedActors.Remove(DestroyedActor);
}

void UDebuffDamageComponent::RemoveEffectFromActor(AActor* OtherActor)
{
	if (!IsValid(OtherActor)) return;

	if (DamageType == EGimmickDamageType::DamageOverTime)
	{
		if (bDelayRemoveDamage)
		{
			FTimerHandle DelayHandle;
			FTimerDelegate Del;
			Del.BindUFunction(this, FName("StopDamageTimer"), OtherActor);
			GetWorld()->GetTimerManager().SetTimer(DelayHandle, Del, DamageRemoveDelay, false);
		}
		else
		{
			StopDamageTimer(OtherActor);
		}
	}

	if (DebuffType != EGimmickDebuffType::None)
	{
		if (bDelayRemoveDebuff)
		{
			FTimerHandle DelayHandle;
			FTimerDelegate Del;
			Del.BindUFunction(this, FName("RemoveDebuff"), OtherActor);
			GetWorld()->GetTimerManager().SetTimer(DelayHandle, Del, DebuffRemoveDelay, false);
		}
		else
		{
			RemoveDebuff(OtherActor);
		}
	}

	AffectedActors.Remove(OtherActor);
}

void UDebuffDamageComponent::ApplyOverTimeDamage(AActor* Target)
{
	if (!IsValid(this) || !IsValid(GetWorld()) || !IsValid(GetOwner()))
	{
		LOG_Art_ERROR(TEXT(" ApplyOverTimeDamage : 컴포넌트 또는 월드 또는 오너 유효하지 않음 → 종료"));
		return;
	}

	if (!IsValid(Target) || !AffectedActors.Contains(Target))
	{
		LOG_Art_WARNING(TEXT(" ApplyOverTimeDamage : Target 유효하지 않음 → 타이머 정지"));
		StopDamageTimer(Target);
		return;
	}

	UGameplayStatics::ApplyDamage(Target, DamageValue, nullptr, GetOwner(), nullptr);
	LOG_Art(Log, TEXT("DOT : %s 에게 %.1f 데미지"), *GetNameSafe(Target), DamageValue);
}

void UDebuffDamageComponent::StopDamageTimer(AActor* Target)
{
	if (!IsValid(this) || !IsValid(GetWorld()))
	{
		LOG_Art_ERROR(TEXT("StopDamageTimer : 컴포넌트 또는 월드 유효하지 않아 강제 종료"));
		return;
	}

	if (DamageTimers.Contains(Target))
	{
		GetWorld()->GetTimerManager().ClearTimer(DamageTimers[Target]);
		DamageTimers.Remove(Target);

		LOG_Art(Log, TEXT("DOT 타이머 정지: %s"), *GetNameSafe(Target));
	}
}

void UDebuffDamageComponent::RemoveDebuff(AActor* Target)
{
	if (!IsValid(Target)) return;

	if (IGimmickDebuffInterface* DebuffActor = Cast<IGimmickDebuffInterface>(Target))
	{
		LOG_Art(Log, TEXT("[DebuffComp] RemoveMovementDebuff 호출 → %s"), *GetNameSafe(Target));
		DebuffActor->RemoveMovementDebuff_Implementation();
	}
	else
	{
		LOG_Art_WARNING(TEXT("[DebuffComp] RemoveMovementDebuff 캐스팅 실패 → %s"), *GetNameSafe(Target));
	}
}

void UDebuffDamageComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
		return;

//	LOG_Art(Log, TEXT("[DebuffComp] OnOverlapBegin → %s"), *OtherActor->GetName());

	OtherActor->OnDestroyed.AddUniqueDynamic(this, &UDebuffDamageComponent::OnTargetDestroyed);

	ApplyEffectToActor(OtherActor);
}

void UDebuffDamageComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
		return;

	if (!AffectedActors.Contains(OtherActor))
	{
		return;
	}

	if (OtherActor->OnDestroyed.IsAlreadyBound(this, &UDebuffDamageComponent::OnTargetDestroyed))
	{
		OtherActor->OnDestroyed.RemoveDynamic(this, &UDebuffDamageComponent::OnTargetDestroyed);
	}

	RemoveEffectFromActor(OtherActor);
}

void UDebuffDamageComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (auto& Pair : DamageTimers)
	{
		GetWorld()->GetTimerManager().ClearTimer(Pair.Value);
	}
	DamageTimers.Empty();

	AffectedActors.Empty();
}