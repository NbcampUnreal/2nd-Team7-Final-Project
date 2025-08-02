#include "Character/Component/CharacterAttackComponent.h"
#include "Item/ItemBase.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

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
		KickHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void UCharacterAttackComponent::DisableKickHitBox()
{
	if (KickHitBox)
	{
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
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority()) return;

	ACharacter* TargetCharacter = Cast<ACharacter>(OtherActor);
	if (!TargetCharacter || TargetCharacter == OwnerCharacter) return;

	HandleHit(TargetCharacter);
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