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

	OnComponentBeginOverlap.AddDynamic(this, &UDebuffDamageComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &UDebuffDamageComponent::OnOverlapEnd);
}

void UDebuffDamageComponent::ApplyEffectToActor(AActor* OtherActor)
{
	if (!IsValid(OtherActor) || AffectedActors.Contains(OtherActor))
	{
		LOG_Art(Log, TEXT("[DebuffComp] ❌ Already affected or invalid: %s"), *GetNameSafe(OtherActor));
		return;
	}

	const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
	if (!TagInterface)
	{
		LOG_Art_WARNING(TEXT("[DebuffComp] ❌ Target does not implement GameplayTag interface: %s"), *GetNameSafe(OtherActor));
		return;
	}

	FGameplayTagContainer ActorTags;
	TagInterface->GetOwnedGameplayTags(ActorTags);

	LOG_Art(Log, TEXT("[DebuffComp] Tags of %s → %s"), *OtherActor->GetName(), *ActorTags.ToStringSimple());

	// 데미지 조건 확인
	if (DamageType != EGimmickDamageType::None && ActorTags.HasTagExact(RequiredDamageTag))
	{
		LOG_Art(Log, TEXT("[DebuffComp] ✅ Damage condition passed"));

		if (DamageType == EGimmickDamageType::InstantDamage)
		{
			UGameplayStatics::ApplyDamage(OtherActor, DamageValue, nullptr, GetOwner(), nullptr);
			LOG_Art(Log, TEXT("[DebuffComp] → Instant Damage %.1f applied to %s"), DamageValue, *OtherActor->GetName());
		}
		else if (DamageType == EGimmickDamageType::DamageOverTime)
		{
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDel;
			TimerDel.BindUFunction(this, FName("ApplyOverTimeDamage"), OtherActor);

			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, DamageInterval, true);
			DamageTimers.Add(OtherActor, TimerHandle);
			LOG_Art(Log, TEXT("[DebuffComp] → DOT started for %s"), *OtherActor->GetName());
		}
	}
	else
	{
		LOG_Art(Log, TEXT("[DebuffComp] ❌ Damage condition failed for %s"), *OtherActor->GetName());
	}

	// 디버프 조건 확인
	if (DebuffType != EGimmickDebuffType::None && ActorTags.HasTagExact(RequiredDebuffTag))
	{
		if (OtherActor->GetClass()->ImplementsInterface(UGimmickDebuffInterface::StaticClass()))
		{
			LOG_Art(Log, TEXT("[DebuffComp] ✅ Debuff condition passed → ApplyMovementDebuff"));
			IGimmickDebuffInterface::Execute_ApplyMovementDebuff(OtherActor, DebuffSlowRate, -1.f);
		}
		else
		{
			LOG_Art_WARNING(TEXT("[DebuffComp] Actor has debuff tag but does not implement interface: %s"), *OtherActor->GetName());
		}
	}
	else
	{
		LOG_Art(Log, TEXT("[DebuffComp] Debuff condition failed for %s"), *OtherActor->GetName());
	}

	AffectedActors.Add(OtherActor);
}

void UDebuffDamageComponent::RemoveEffectFromActor(AActor* OtherActor)
{
	if (!IsValid(OtherActor)) return;

	// 데미지 제거 처리
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

	//  디버프 제거 처리
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
	if (IsValid(Target))
	{
		UGameplayStatics::ApplyDamage(Target, DamageValue, nullptr, GetOwner(), nullptr);
	}
}

void UDebuffDamageComponent::StopDamageTimer(AActor* Target)
{
	if (FTimerHandle* Handle = DamageTimers.Find(Target))
	{
		GetWorld()->GetTimerManager().ClearTimer(*Handle);
		DamageTimers.Remove(Target);
	}
}

void UDebuffDamageComponent::RemoveDebuff(AActor* Target)
{
	if (Target->GetClass()->ImplementsInterface(UGimmickDebuffInterface::StaticClass()))
	{
		IGimmickDebuffInterface::Execute_RemoveMovementDebuff(Target);
	}
}

void UDebuffDamageComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
		return;

	LOG_Art(Log, TEXT("[DebuffComp] ▶ OnOverlapBegin → %s"), *OtherActor->GetName());

	ApplyEffectToActor(OtherActor);
}

void UDebuffDamageComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
		return;

	RemoveEffectFromActor(OtherActor);
}
