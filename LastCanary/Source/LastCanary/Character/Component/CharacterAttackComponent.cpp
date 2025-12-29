#include "Character/Component/CharacterAttackComponent.h"
#include "Character/BaseCharacter.h"
#include "Item/ItemBase.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Component/CharacterAnimationComponent.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Item/EquipmentItem/Pickaxe.h"

#include "LastCanary.h"

UCharacterAttackComponent::UCharacterAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupHandHitBox();

	if (GetCharacter())
	{
		KickHitBox = NewObject<UBoxComponent>(GetCharacter(), TEXT("KickHitBox"));
		KickHitBox->AttachToComponent(GetCharacter()->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("foot_l"));
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
		Handle_Pickaxe_Attack();
		break;
	default:
		break;
	}
}

void UCharacterAttackComponent::Handle_Kick()
{
	GetBaseCharacter()->AnimationComponent->PlayAttackMontage(KickMontage);
}

void UCharacterAttackComponent::Handle_Pickaxe_Attack()
{
	if (GetBaseCharacter()->AnimationComponent->GetIsPlayingAttackMontage() == false)
	{
		GetBaseCharacter()->AnimationComponent->PlayAttackMontage(PickAxeMontage);
	}
}

void UCharacterAttackComponent::SetupHandHitBox()
{
	if (!IsValid(GetCharacter()))
	{
		return; 
	}

	HandHitBox = NewObject<UBoxComponent>(GetCharacter(), TEXT("HandHitBox"));
	HandHitBox->RegisterComponent();
	HandHitBox->AttachToComponent(GetCharacter()->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Hand_RSocket"));
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
	if (!GetCharacter()->HasAuthority()) return;

	if (!GetCharacter()) return;

	const FVector Start = GetCharacter()->GetActorLocation() + GetCharacter()->GetActorForwardVector() * 50.f + FVector(0, 0, 50.f);
	const FVector End = Start;
	const FVector Extent(KickBoxExtent, KickBoxExtent, KickBoxExtent);

	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(), Start, End, Extent, GetCharacter()->GetActorRotation(),
		{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		false, { GetCharacter() },
		EDrawDebugTrace::None, HitResults, true);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* Target = Hit.GetActor();
		if (Target && Target != GetCharacter())
		{
			FVector Dir = GetCharacter()->GetActorForwardVector();
			Dir.Z = 0.f;
			Dir.Normalize();
			ApplyDamageAndEffects(Target, Dir);
		}
	}
}

void UCharacterAttackComponent::PerformWeaponAttack(UPrimitiveComponent* WeaponCollider)
{
	if (!GetCharacter()->HasAuthority()) return;

	TArray<AActor*> OverlappingActors;
	WeaponCollider->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Target : OverlappingActors)
	{
		if (Target && Target != GetCharacter())
		{
			FVector Dir = GetCharacter()->GetActorForwardVector();
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

void UCharacterAttackComponent::StartItemAttack()
{
	if (GetBaseCharacter()->AnimationComponent)
	{
		AItemBase* Item = GetBaseCharacter()->GetCurrentItem();
		
		if (!IsValid(Item))
		{
			return;
		}

		AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(Item);
		if (!EquipmentItem)
		{
			return;
		}
		if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
		{
			APickaxe* axe = Cast<APickaxe>(EquipmentItem);
			axe->SetAttackCollisionEnabled(true);
		}
	}
}

void UCharacterAttackComponent::EndItemAttack()
{
	if (GetBaseCharacter()->AnimationComponent)
	{
		GetBaseCharacter()->AnimationComponent->HandleAnimNotify(EAnimationMontageType::Attack);
		AItemBase* Item = GetBaseCharacter()->GetCurrentItem();

		if (!IsValid(Item))
		{
			return;
		}

		AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(Item);
		if (!EquipmentItem)
		{
			return;
		}
		if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
		{
			APickaxe* axe = Cast<APickaxe>(EquipmentItem);
			axe->SetAttackCollisionEnabled(false);
		}
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
	if (!GetBaseCharacter()->HasAuthority())
	{
		return;
	}
	const FVector Start = GetBaseCharacter()->GetActorLocation() + GetBaseCharacter()->GetActorForwardVector() * 50.f + FVector(0, 0, 50.f);
	const FVector End = Start; // 박스는 이동하지 않음

	const FVector BoxExtent = FVector(100.f, 100.f, 100.f); // 크기 조절 가능
	const FRotator Rotation = GetBaseCharacter()->GetActorRotation();

	TArray<FHitResult> HitResults;

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		Start,
		End,
		BoxExtent,
		Rotation,
		{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		false,
		{ GetBaseCharacter()},
		EDrawDebugTrace::None,
		HitResults,
		true // ignore self
	);

	for (const FHitResult& Hit : HitResults)
	{
		ACharacter* TargetCharacter = Cast<ACharacter>(Hit.GetActor());
		if (!TargetCharacter || TargetCharacter == GetBaseCharacter()) continue;
		if (TargetCharacter->IsA<ABaseBossMonsterCharacter>())
		{
			continue;
		}

		// 넉백 처리
		FVector KnockbackDir = GetBaseCharacter()->GetActorForwardVector();
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
	FVector KnockbackDir = GetCharacter()->GetActorForwardVector();
	KnockbackDir.Z = 0.f;
	KnockbackDir.Normalize();

	const float UpwardStrength = 200.f;

	TargetCharacter->LaunchCharacter(KnockbackDir * KnockbackStrength + FVector(0, 0, UpwardStrength), true, true);

	// 상태이상, 피격 애니메이션 등은 여기서 처리 가능
	// 예: UGameplayStatics::ApplyDamage(TargetCharacter, 20.f, OwnerCharacter->GetController(), OwnerCharacter, nullptr);
}