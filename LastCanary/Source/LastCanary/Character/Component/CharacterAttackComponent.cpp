#include "Character/Component/CharacterAttackComponent.h"
#include "Character/BaseCharacter.h"
#include "Item/ItemBase.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Component/CharacterAnimationComponent.h"
#include "AI/BaseBossMonsterCharacter.h"

#include "LastCanary.h"

UCharacterAttackComponent::UCharacterAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupHandHitBox();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		KickHitBox = NewObject<UBoxComponent>(OwnerCharacter, TEXT("KickHitBox"));
		KickHitBox->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("foot_l"));
		KickHitBox->SetBoxExtent(FVector(20, 30, 30));
		KickHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		KickHitBox->SetCollisionObjectType(ECC_WorldDynamic);
		KickHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		KickHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		KickHitBox->RegisterComponent();

		KickHitBox->OnComponentBeginOverlap.AddDynamic(this, &UCharacterAttackComponent::OnKickHitBoxOverlap);
	}
}

void UCharacterAttackComponent::SetEquippedWeapon(AItemBase* Weapon)
{
	EquippedWeapon = Weapon;
}

void UCharacterAttackComponent::Handle_Attack(EAttackType _AttackType)
{
	switch (_AttackType)
	{
	case EAttackType::Kick:
		Handle_Kick();
		break;
	case EAttackType::Punch:
		break;
	case EAttackType::ItemAttack:
		break;
	default:
		break;
	}
}

void UCharacterAttackComponent::Handle_Kick()
{
	GetCharacter()->AnimationComponent->PlayAttackMontage(KickMontage);
}

void UCharacterAttackComponent::SetupHandHitBox()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	HandHitBox = NewObject<UBoxComponent>(OwnerChar, TEXT("HandHitBox"));
	HandHitBox->RegisterComponent();
	HandHitBox->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Hand_RSocket"));
	HandHitBox->SetBoxExtent(FVector(10.f, 10.f, 10.f));
	HandHitBox->SetCollisionProfileName(TEXT("OverlapAll"));
	HandHitBox->SetGenerateOverlapEvents(true);
	HandHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HandHitBox->OnComponentBeginOverlap.AddDynamic(this, &UCharacterAttackComponent::OnHandHitBoxOverlap);
}

void UCharacterAttackComponent::StartAttack()
{
	HitActors.Empty();

	if (EquippedWeapon)
	{
		//EquippedWeapon->StartMeleeAttack();
	}
	else if (HandHitBox)
	{
		HandHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void UCharacterAttackComponent::EndAttack()
{
	if (EquippedWeapon)
	{
		//EquippedWeapon->EndMeleeAttack();
	}
	else if (HandHitBox)
	{
		HandHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UCharacterAttackComponent::OnHandHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner() || HitActors.Contains(OtherActor)) return;

	HitActors.Add(OtherActor);
	UGameplayStatics::ApplyDamage(OtherActor, UnarmedDamage, nullptr, GetOwner(), nullptr);
}


void UCharacterAttackComponent::PerformKickAttack()
{
	if (!GetOwner()->HasAuthority()) return;

	if (!OwnerCharacter) return;

	const FVector Start = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 50.f + FVector(0, 0, 50.f);
	const FVector End = Start;
	const FVector Extent(KickBoxExtent, KickBoxExtent, KickBoxExtent);

	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(), Start, End, Extent, OwnerCharacter->GetActorRotation(),
		{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		false, { OwnerCharacter },
		EDrawDebugTrace::None, HitResults, true);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* Target = Hit.GetActor();
		if (Target && Target != OwnerCharacter)
		{
			FVector Dir = OwnerCharacter->GetActorForwardVector();
			Dir.Z = 0.f;
			Dir.Normalize();
			ApplyDamageAndEffects(Target, Dir);
		}
	}
}

void UCharacterAttackComponent::PerformWeaponAttack(UPrimitiveComponent* WeaponCollider)
{
	if (!GetOwner()->HasAuthority()) return;

	TArray<AActor*> OverlappingActors;
	WeaponCollider->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Target : OverlappingActors)
	{
		if (Target && Target != GetOwner())
		{
			FVector Dir = GetOwner()->GetActorForwardVector();
			Dir.Z = 0.f;
			Dir.Normalize();
			ApplyDamageAndEffects(Target, Dir);
		}
	}
}

void UCharacterAttackComponent::ApplyDamageAndEffects(AActor* Target, const FVector& KnockbackDirection)
{
	ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (!TargetCharacter) return;

	// 데미지 처리
	UGameplayStatics::ApplyDamage(TargetCharacter, 10.f, nullptr, GetOwner(), nullptr);

	// 넉백
	TargetCharacter->LaunchCharacter(KnockbackDirection * KnockbackStrength + FVector(0, 0, UpwardKnockback), true, true);

	// 상태이상 or Hit Reaction 애니메이션 실행
	// ex) UAnimInstance::Montage_Play(HitReactMontage);
}

void UCharacterAttackComponent::EnableKickHitBox()
{
	if (KickHitBox)
	{
		LOG_Char_WARNING(TEXT("충돌 시작"));
		KickHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void UCharacterAttackComponent::DisableKickHitBox()
{
	if (KickHitBox)
	{
		LOG_Char_WARNING(TEXT("충돌 해제"));

		KickHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UCharacterAttackComponent::SetWeaponHitBox(UPrimitiveComponent* WeaponHitBox)
{
	if (CurrentWeaponHitBox)
	{
		CurrentWeaponHitBox->OnComponentBeginOverlap.RemoveAll(this);
	}

	CurrentWeaponHitBox = WeaponHitBox;

	if (CurrentWeaponHitBox)
	{
		CurrentWeaponHitBox->OnComponentBeginOverlap.AddDynamic(this, &UCharacterAttackComponent::OnKickHitBoxOverlap); // 재사용
	}
}

void UCharacterAttackComponent::OnKickHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	
	// 서버에서만 처리
	if (!GetCharacter()->HasAuthority())
	{
		return;
	}
	const FVector Start = GetCharacter()->GetActorLocation() + GetCharacter()->GetActorForwardVector() * 50.f + FVector(0, 0, 50.f);
	const FVector End = Start; // 박스는 이동하지 않음

	const FVector BoxExtent = FVector(100.f, 100.f, 100.f); // 크기 조절 가능
	const FRotator Rotation = GetCharacter()->GetActorRotation();

	TArray<FHitResult> HitResults;

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		Start,
		End,
		BoxExtent,
		Rotation,
		{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		false,
		{ GetCharacter()},
		EDrawDebugTrace::None,
		HitResults,
		true // ignore self
	);

	for (const FHitResult& Hit : HitResults)
	{
		ACharacter* TargetCharacter = Cast<ACharacter>(Hit.GetActor());
		if (!TargetCharacter || TargetCharacter == GetCharacter()) continue;
		if (TargetCharacter->IsA<ABaseBossMonsterCharacter>())
		{
			continue;
		}

		// 넉백 처리
		FVector KnockbackDir = GetCharacter()->GetActorForwardVector();
		KnockbackDir.Z = 0;
		KnockbackDir.Normalize();

		TargetCharacter->LaunchCharacter(KnockbackDir * KnockbackStrength + FVector(0, 0, UpwardKnockback), true, true);
	}

	/*
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority()) return;

	ACharacter* TargetCharacter = Cast<ACharacter>(OtherActor);
	if (!TargetCharacter || TargetCharacter == OwnerCharacter) return;

	HandleHit(TargetCharacter);
	*/
}

void UCharacterAttackComponent::HandleHit(ACharacter* TargetCharacter)
{
	if (!TargetCharacter) return;

	// 넉백 처리
	FVector KnockbackDir = OwnerCharacter->GetActorForwardVector();
	KnockbackDir.Z = 0.f;
	KnockbackDir.Normalize();

	const float UpwardStrength = 200.f;

	TargetCharacter->LaunchCharacter(KnockbackDir * KnockbackStrength + FVector(0, 0, UpwardStrength), true, true);

	// 상태이상, 피격 애니메이션 등은 여기서 처리 가능
	// 예: UGameplayStatics::ApplyDamage(TargetCharacter, 20.f, OwnerCharacter->GetController(), OwnerCharacter, nullptr);
}