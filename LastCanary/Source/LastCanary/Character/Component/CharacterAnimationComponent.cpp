#include "Character/Component/CharacterAnimationComponent.h"
#include "Character/BaseCharacter.h"

#include "LastCanary.h"

UCharacterAnimationComponent::UCharacterAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(CachedCharacter))
	{
		USkeletalMeshComponent* Mesh = CachedCharacter->GetMesh();
		if (IsValid(Mesh))
		{
			CachedAnimInstance = Mesh->GetAnimInstance(); // 필요 시 폴링 기법 사용 ( 지속적인 테스트 필요 )
		}
	}
}

void UCharacterAnimationComponent::PlayMontageByType(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type)
{
	if (!IsValid(GetCharacter())) return;
	if (!IsValid(CachedAnimInstance)) return;

	if (GetOwnerRole() < ROLE_Authority) // 로컬에서만 재생하는 버전
	{
		if (IsValid(LocalMontage))
		{
			CachedAnimInstance->Montage_Play(LocalMontage);
		}
		Server_PlayMontage(MulticastMontage, Type);
	}
	else
	{		
		Multicast_PlayMontage(MulticastMontage, Type); // 서버에서 멀티캐스트로 전파
	}

	SetPlayingMontageState(Type, true);
}

void UCharacterAnimationComponent::PlayInteractMontage()
{
	UAnimMontage* Local = MontageMap.FindRef(EAnimationMontageType::Interaction);
	UAnimMontage* Remote = MontageMap.FindRef(EAnimationMontageType::Interaction);
	PlayMontageByType(Local, Remote, EAnimationMontageType::Interaction);
}

void UCharacterAnimationComponent::PlayUseItemMontage(UAnimMontage* LocalMontage, UAnimMontage* RemoteMontage)
{
	PlayMontageByType(LocalMontage, RemoteMontage, EAnimationMontageType::UseItem);
}

void UCharacterAnimationComponent::PlayGunReloadMontage()
{
	UAnimMontage* Local = MontageMap.FindRef(EAnimationMontageType::GunReload);
	UAnimMontage* Remote = MontageMap.FindRef(EAnimationMontageType::GunReload);
	PlayMontageByType(Local, Remote, EAnimationMontageType::GunReload);
}

void UCharacterAnimationComponent::PlayEmoteMontage()
{
	UAnimMontage* Local = MontageMap.FindRef(EAnimationMontageType::Emote);
	UAnimMontage* Remote = MontageMap.FindRef(EAnimationMontageType::Emote);
	PlayMontageByType(Local, Remote, EAnimationMontageType::Emote);
}

void UCharacterAnimationComponent::PlayAttackMontage()
{
	UAnimMontage* Local = MontageMap.FindRef(EAnimationMontageType::Attack);
	UAnimMontage* Remote = MontageMap.FindRef(EAnimationMontageType::Attack);
	PlayMontageByType(Local, Remote, EAnimationMontageType::Attack);
}


void UCharacterAnimationComponent::Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Type)
{
	Multicast_PlayMontage(MontageToPlay, Type);
}

void UCharacterAnimationComponent::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Type)
{
	if (!IsValid(CachedAnimInstance)) return;

	// 로컬 플레이어는 이미 재생했으므로 생략
	AActor* Owner = GetOwner();
	if (Owner && Owner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		return;
	}

	if (IsValid(MontageToPlay))
	{
		CachedAnimInstance->Montage_Play(MontageToPlay);
	}
}

void UCharacterAnimationComponent::CancelMontageByType(UAnimMontage* MontageToStop, EAnimationMontageType Type)
{
	if (!IsValid(CachedAnimInstance)) return;

	if (GetOwnerRole() < ROLE_Authority)
	{
		CachedAnimInstance->Montage_Stop(0.1f, MontageToStop);
		Server_CancelMontage(MontageToStop, Type);
	}
	else
	{
		Multicast_CancelMontage(MontageToStop, Type);
	}

	SetPlayingMontageState(Type, false);
}

void UCharacterAnimationComponent::CancelInteractModntage()
{
	UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::Interaction);
	CancelMontageByType(Montage, EAnimationMontageType::Interaction);
}

void UCharacterAnimationComponent::CancelUseItemMontage()
{
	UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::UseItem);
	CancelMontageByType(Montage, EAnimationMontageType::UseItem);
}

void UCharacterAnimationComponent::CancelGunReloadMontage()
{
	UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::GunReload);
	CancelMontageByType(Montage, EAnimationMontageType::GunReload);
}

void UCharacterAnimationComponent::CancelEmoteMontage()
{
	UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::Emote);
	CancelMontageByType(Montage, EAnimationMontageType::Emote);
}

void UCharacterAnimationComponent::CancelAttackMontage()
{
	UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::Attack);
	CancelMontageByType(Montage, EAnimationMontageType::Attack);
}

void UCharacterAnimationComponent::Server_CancelMontage_Implementation(UAnimMontage* MontageToStop, EAnimationMontageType Type)
{
	Multicast_CancelMontage(MontageToStop, Type);
}

void UCharacterAnimationComponent::Multicast_CancelMontage_Implementation(UAnimMontage* MontageToStop, EAnimationMontageType Type)
{
	if (!IsValid(CachedAnimInstance)) return;

	// 로컬 플레이어는 이미 캔슬했을 수 있음 → 필요 시 생략
	AActor* Owner = GetOwner();
	if (Owner && Owner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		return;
	}

	if (IsValid(MontageToStop))
	{
		CachedAnimInstance->Montage_Stop(0.1f, MontageToStop);
	}
}

void UCharacterAnimationComponent::HandleAnimNotify(EAnimationMontageType Type)
{
	// 노티파이에서 해당 타입으로 뭔가를 처리하고 싶다면 여기에 정의
}

void UCharacterAnimationComponent::SetPlayingMontageState(EAnimationMontageType Type, bool bIsPlaying)
{
	switch (Type)
	{
	case EAnimationMontageType::Interaction:
		bIsPlayingInteractionMontage = bIsPlaying;
		break;
	case EAnimationMontageType::UseItem:
		bIsPlayingUseItemMontage = bIsPlaying;
		break;
	case EAnimationMontageType::GunReload:
		bIsPlayingGunReloadMontage = bIsPlaying;
		break;
	case EAnimationMontageType::Emote:
		bIsPlayingEmoteMontage = bIsPlaying;
		break;
	case EAnimationMontageType::Attack:
		bIsPlayingAttackMontage = bIsPlaying;
		break;
	default:
		break;
	}
}