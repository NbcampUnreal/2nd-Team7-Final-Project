#include "Character/Component/CharacterAnimationComponent.h"
#include "Character/BaseCharacter.h"
#include "Item/ItemBase.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "Character/Component/CharacterInteractionComponent.h"
#include "Net/UnrealNetwork.h"

#include "LastCanary.h"

UCharacterAnimationComponent::UCharacterAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

USkeletalMeshComponent* UCharacterAnimationComponent::CharacterMesh()
{
	return CachedCharacter->GetMesh();
}

void UCharacterAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCharacterAnimationComponent::PlayMontageByType(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type)
{
	if (!IsValid(GetCharacter())) return;
	if (!IsValid(CachedAnimInstance)) return;

	if (GetOwnerRole() < ROLE_Authority) // 로컬에서만 재생하는 버전
	{
		LOG_Char_WARNING(TEXT("로컬에서 애니메이션 재생"));
		if (IsValid(LocalMontage))
		{
			CachedAnimInstance->Montage_Play(LocalMontage);
		}
		Server_PlayMontage(LocalMontage, MulticastMontage, Type);
	}
	else
	{		
		LOG_Char_WARNING(TEXT("서버에서 애니메이션 재생"));
		Multicast_PlayMontage(LocalMontage, MulticastMontage, Type); // 서버에서 멀티캐스트로 전파
	}

	SetPlayingMontageState(Type, true);
}

void UCharacterAnimationComponent::PlayInteractMontage(AActor* TargetActor)
{
	LOG_Char_WARNING(TEXT("애니메이션 컴포넌트에서 애니메이션 재생"));
	if (!IsValid(TargetActor)) return;

	UAnimMontage* Local = MontageMap.FindRef(EAnimationMontageType::Interaction); //기본으로 이 애니메이션을 사용
	UAnimMontage* Remote = MontageMap.FindRef(EAnimationMontageType::Interaction); //기본으로 이 애니메이션을 사용

	if (TargetActor->IsA<AItemBase>())
	{
		AItemBase* Item = Cast<AItemBase>(TargetActor);

		if (!IsValid(Item))
		{
			return;
		}
		
		//MontageToPlay = GetCharacter()->InteractMontageOnUnderObject;
	}
	else if (TargetActor->IsA<ALCBaseGimmick>())
	{
		ALCBaseGimmick* Gimmick = Cast<ALCBaseGimmick>(TargetActor);
		Local = Gimmick->LocalAnimation;
		Remote = Gimmick->RemoteAnimation;
	}

	PlayMontageByType(Local, Remote, EAnimationMontageType::Interaction);
}

void UCharacterAnimationComponent::PlayUseItemMontage(AItemBase* Item)
{
	LOG_Char_WARNING(TEXT("애니메이션 컴포넌트에서 애니메이션 재생"));
	if (!Item) return;
	FItemDataRow Data = Item->ItemData;
	if (Data.bPlayCharacterAnimation == true)
	{
		LOG_Char_WARNING(TEXT("애니메이션 재생 여부 O"));
		PlayMontageByType(Data.LocalAnimation, Data.RemoteAnimation, EAnimationMontageType::UseItem);
		// 예: 사운드나 VFX 연동  //PlayItemUseEffect(Item);
		return;
	}
	else
	{
		OnUseItemNotify.Broadcast(); // 바로 브로드캐스트로 아이템 사용을 지시
	}
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

void UCharacterAnimationComponent::PlayAttackMontage(UAnimMontage* _AttackMontage)
{
	PlayMontageByType(_AttackMontage, _AttackMontage, EAnimationMontageType::Attack);
}


void UCharacterAnimationComponent::Server_PlayMontage_Implementation(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type)
{
	Multicast_PlayMontage(LocalMontage, MulticastMontage, Type);
}

void UCharacterAnimationComponent::Multicast_PlayMontage_Implementation(UAnimMontage* LocalMontage, UAnimMontage* MulticastMontage, EAnimationMontageType Type)
{
	if (!IsValid(CachedAnimInstance)) return;

	AActor* Owner = GetOwner();
	if (Owner && Owner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		CachedAnimInstance->Montage_Play(LocalMontage);
		return;
	}

	if (IsValid(MulticastMontage))
	{
		CachedAnimInstance->Montage_Play(MulticastMontage);
	}
}

void UCharacterAnimationComponent::CancelAllMontage()
{
	if (!IsValid(CachedAnimInstance)) return;
	CachedAnimInstance->Montage_Stop(0.1f);
}

void UCharacterAnimationComponent::CancelMontageByType(UAnimMontage* LocalMontageToStop, UAnimMontage* RemoteMontageToStop, EAnimationMontageType Type)
{
	if (!IsValid(CachedAnimInstance)) return;

	if (GetOwnerRole() < ROLE_Authority)
	{
		// 자기 자신이 보는 애니메이션 정지
		if (IsValid(LocalMontageToStop))
		{
			CachedAnimInstance->Montage_Stop(0.1f, LocalMontageToStop);
		}
		Server_CancelMontage(LocalMontageToStop, RemoteMontageToStop, Type);
	}
	else
	{
		Multicast_CancelMontage(LocalMontageToStop, RemoteMontageToStop, Type);
	}

	SetPlayingMontageState(Type, false);
}

void UCharacterAnimationComponent::CancelInteractModntage()
{
	//상호작용한 물체에서 애니메이션 가져오기  그냥 전체를 멈춰버려도 상관없긴 해보임.
	//UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::Interaction);
	//CancelMontageByType(Montage, Montage, EAnimationMontageType::Interaction);
}

void UCharacterAnimationComponent::CancelUseItemMontage(AItemBase* Item)
{
	if (!Item) return;
	FItemDataRow Data = Item->ItemData;
	if (Data.bPlayCharacterAnimation == true)
	{		
		CancelMontageByType(Data.LocalAnimation, Data.RemoteAnimation, EAnimationMontageType::UseItem);
		return;
	}	
}

void UCharacterAnimationComponent::CancelGunReloadMontage()
{
	UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::GunReload);
	CancelMontageByType(Montage, Montage, EAnimationMontageType::GunReload);
}

void UCharacterAnimationComponent::CancelEmoteMontage()
{
	UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::Emote);
	CancelMontageByType(Montage, Montage, EAnimationMontageType::Emote);
}

void UCharacterAnimationComponent::CancelAttackMontage()
{
	UAnimMontage* Montage = MontageMap.FindRef(EAnimationMontageType::Attack);
	CancelMontageByType(Montage, Montage, EAnimationMontageType::Attack);
}

void UCharacterAnimationComponent::Server_CancelMontage_Implementation(UAnimMontage* LocalMontageToStop, UAnimMontage* RemoteMontageToStop, EAnimationMontageType Type)
{
	Multicast_CancelMontage(LocalMontageToStop, RemoteMontageToStop, Type);
}

void UCharacterAnimationComponent::Multicast_CancelMontage_Implementation(UAnimMontage* LocalMontageToStop, UAnimMontage* RemoteMontageToStop, EAnimationMontageType Type)
{
	if (!IsValid(CachedAnimInstance)) return;

	if (!GetCharacter()->IsLocallyControlled())
	{
		CachedAnimInstance->Montage_Stop(0.1f, RemoteMontageToStop); // 다른 사람에게 보이는 애니메이션만 중단
	}
	else
	{
		CachedAnimInstance->Montage_Stop(0.1f, LocalMontageToStop); // 나에게 보이는 애니메이션만 중단
	}

	SetPlayingMontageState(Type, false);
}

void UCharacterAnimationComponent::HandleAnimNotify(EAnimationMontageType Type)
{
	if (!IsValid(GetPlayerController())) return;

	switch (Type)
	{
	case EAnimationMontageType::GunReload:
		OnReloadNotify.Broadcast();
		SetPlayingMontageState(Type, false);
		break;

	case EAnimationMontageType::Interaction:
		if (GetCharacter()->InteractionComponent)
		{
			GetCharacter()->InteractionComponent->Interact();
		}
		SetPlayingMontageState(Type, false);
		break;

	case EAnimationMontageType::UseItem:
		OnUseItemNotify.Broadcast();
		SetPlayingMontageState(Type, false);
		break;

	case EAnimationMontageType::Attack:
		LOG_Char_WARNING(TEXT("애니메이션 컴포넌트에서 어택 가능 변수 변경"));
		SetPlayingMontageState(Type, false);
		break;

	default:
		break;
	}
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
		LOG_Char_WARNING(TEXT("애니메이션 컴포넌트에서 어택 가능 변수 변경"));

		break;
	default:
		break;
	}
}






//이것도 아이템에다가 애니메이션 등록해놓고 받아오기
void UCharacterAnimationComponent::RefreshOverlayLinkedAnimationLayer(FGameplayTag ItemTag)
{
	TSubclassOf<UAnimInstance> OverlayAnimationInstanceClass;
	if (GetCharacter()->bIsSpawnDrone)  // 태그에 컨트롤러 들 때 사용할 태그 추가해야됨...
	{
		OverlayAnimationInstanceClass = BinocularsAnimationClass;
		if (IsValid(OverlayAnimationInstanceClass))
		{
			CharacterMesh()->LinkAnimClassLayers(OverlayAnimationInstanceClass);
		}
		else
		{
			CharacterMesh()->LinkAnimClassLayers(DefaultAnimationClass);
		}
		return;
	}
	if (!ItemTag.IsValid())
	{
		OverlayAnimationInstanceClass = DefaultAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
	{
		OverlayAnimationInstanceClass = RifleAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.FlashLight")))
	{
		OverlayAnimationInstanceClass = TorchAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")))
	{
		OverlayAnimationInstanceClass = PistolTwoHandedAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
	{
		OverlayAnimationInstanceClass = RifleAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Spawnable.Drone")))
	{
		OverlayAnimationInstanceClass = PistolOneHandedAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
	{
		OverlayAnimationInstanceClass = PickaxeAnimationClass;
	}
	else
	{
		OverlayAnimationInstanceClass = DefaultAnimationClass;
	}

	if (IsValid(OverlayAnimationInstanceClass))
	{
		CharacterMesh()->LinkAnimClassLayers(OverlayAnimationInstanceClass);
	}
	else
	{
		CharacterMesh()->LinkAnimClassLayers(DefaultAnimationClass);
	}
}