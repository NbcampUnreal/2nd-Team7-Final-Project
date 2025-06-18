#include "Item/ResourceNode.h"
#include "Item/ResourceItem/ResourceItemSpawnManager.h"
#include "Item/ItemBase.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"

#include "LastCanary.h"

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	MaxHarvestCount = 5;
	CurrentHarvestCount = 0;
	bInfiniteHarvest = false;
}

void AResourceNode::BeginPlay()
{
	Super::BeginPlay();

	// 레벨 상에 있는 스폰 매니저 할당
	ResourceItemSpawnManager = Cast<AResourceItemSpawnManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AResourceItemSpawnManager::StaticClass()));
	if (!ResourceItemSpawnManager)
	{
		LOG_Item_WARNING(TEXT("[AResourceNode::BeginPlay] ResourceItemSpawnManager not found in level!"));
	}
}

void AResourceNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceNode, CurrentHarvestCount);
}

void AResourceNode::Interact_Implementation(APlayerController* Interactor)
{
	// 현재는 채취 아이템에서 처리
	// if (ABaseCharacter* Character = Cast<ABaseCharacter>(Interactor->GetPawn()))
	// {
	// 	Character->Server_InteractWithResourceNode(this);
	// }
}

void AResourceNode::Server_RequestInteract_Implementation(APlayerController* Interactor)
{
	// 현재는 채취 아이템에서 처리
	// 서버에서 실제 상호작용 처리
	// HarvestResource(Interactor);
}

void AResourceNode::HarvestResource(APlayerController* Interactor)
{
	if (!CanHarvest())
	{
		LOG_Item_WARNING(TEXT("[AResourceNode::HarvestResource] 채취 불가능 - 자원이 고갈됨"));
		return;
	}

	if (!ResourceItemSpawnManager)
	{
		LOG_Item_WARNING(TEXT("[AResourceNode::HarvestResource] ResourceItemSpawnManager not initialized!"));
		return;
	}

	if (PossibleResourceItems.Num() <= 0)
	{
		LOG_Item_WARNING(TEXT("[AResourceNode::HarvestResource] 스폰할 아이템 목록이 할당되지 않았습니다."));
		return;
	}

	float TotalProbability = 0.0f;
	for (const FResourceItemData& ItemData : PossibleResourceItems)
	{
		TotalProbability += ItemData.Probability;
	}

	if (TotalProbability <= 0.0f)
	{
		LOG_Item_WARNING(TEXT("[AResourceNode::HarvestResource] 유효한 아이템 확률 없음"));
		return;
	}

	const float RandomValue = FMath::FRandRange(0.0f, TotalProbability);

	// 누적 확률로 아이템 선택
	float Accumulated = 0.0f;
	FName SelectedItemRow = NAME_None;
	for (const FResourceItemData& ItemData : PossibleResourceItems)
	{
		Accumulated += ItemData.Probability;
		if (RandomValue <= Accumulated)
		{
			SelectedItemRow = ItemData.ItemRowName;
			break;
		}
	}

	FVector SpawnLocation = CalculateResourceSpawnLocation(Interactor);
	ResourceItemSpawnManager->SpawnItemAtLocation(SelectedItemRow, SpawnLocation);

	if (!bInfiniteHarvest)
	{
		CurrentHarvestCount++;
		LOG_Item_WARNING(TEXT("[ResourceNode] 채취 완료 (%d/%d)"), CurrentHarvestCount, MaxHarvestCount);

		// 최대 채취 수량에 도달했으면 파괴 예약
		if (CurrentHarvestCount >= MaxHarvestCount)
		{
			LOG_Item_WARNING(TEXT("[ResourceNode] 자원 고갈! 파괴 예약"));
			DestroyResourceNode();
		}
	}
	else
	{
		LOG_Item_WARNING(TEXT("[ResourceNode] ✅ 무한 자원 채취 완료"));
	}
}

FText AResourceNode::GetDefaultMessageForType(EResourceInteractionType Type) const
{
	switch (Type)
	{
	case EResourceInteractionType::Harvest:
		return NSLOCTEXT("Interaction", "Harvest", "Press [{Key}] to Harvest");
	case EResourceInteractionType::Mine:
		return NSLOCTEXT("Interaction", "Mine", "Press [{Key}] to Mine");
	case EResourceInteractionType::Loot:
		return NSLOCTEXT("Interaction", "Loot", "Press [{Key}] to Loot");
	case EResourceInteractionType::GetNote:
		return NSLOCTEXT("Interaction", "GetNote", "Press [{Key}] to Get Note");
	default:
		return NSLOCTEXT("Interaction", "Default", "Press [{Key}] to Interact");
	}
}

FString AResourceNode::GetCurrentKeyNameForInteract() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!IsValid(PC) || !IA_Interact)
	{
		return TEXT("Key");
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return TEXT("Key");
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(Subsystem))
	{
		return TEXT("Key");
	}

	const TArray<FEnhancedActionKeyMapping>& Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();
	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (Mapping.Action == IA_Interact)
		{
			return Mapping.Key.GetDisplayName().ToString();
		}
	}
	return TEXT("Key");
}

FString AResourceNode::GetInteractMessage_Implementation() const
{
	FString KeyName = GetCurrentKeyNameForInteract();

	// 플레이어로부터 현재 장착 아이템 태그를 받아와서 비교해야 함
	ABaseCharacter* Character = GetWorld()->GetFirstPlayerController()
		? Cast<ABaseCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn())
		: nullptr;

	if (Character == nullptr)
	{
		return TEXT("No interaction (no players)");
	}

	// Core 타입은 도구 없이도 가능
	if (InteractionType == EResourceInteractionType::Core)
	{
		FText Template = GetDefaultMessageForType(InteractionType);
		return FText::Format(Template, FFormatNamedArguments{ {"Key", FText::FromString(KeyName)} }).ToString();
	}

	if (AItemBase* Equipped = Character->GetToolbarInventoryComponent()->GetCurrentEquippedItem())
	{
		// 곡괭이 태그와 매칭 여부만 간략히 체크
		if (Equipped->ItemData.ItemType.MatchesTag(RequiredToolTag)) // 예: "Tool.Pickaxe"
		{
			FText Template = GetDefaultMessageForType(InteractionType);
			return FText::Format(Template, FFormatNamedArguments{ {"Key", FText::FromString(KeyName)} }).ToString();
		}
	}

	return FString::Printf(TEXT("Cannot interact (requires correct tool)"));
}

FVector AResourceNode::CalculateResourceSpawnLocation(APlayerController* Interactor) const
{
	FVector NodeLocation = GetActorLocation();

	switch (SpawnLocationType)
	{
	case EResourceSpawnLocationType::NodeTop:
		return NodeLocation + FVector(0, 0, 100);

	case EResourceSpawnLocationType::MidpointToCharacter:
	{
		if (Interactor && Interactor->GetPawn())
		{
			FVector CharLocation = Interactor->GetPawn()->GetActorLocation();
			return (NodeLocation + CharLocation) * 0.5f;
		}
		return NodeLocation;
	}

	case EResourceSpawnLocationType::CustomOffset:
		return NodeLocation + CustomSpawnOffset;

	default:
		return NodeLocation;
	}
}

bool AResourceNode::CanHarvest() const
{
	if (bInfiniteHarvest)
	{
		return true;
	}

	return CurrentHarvestCount < MaxHarvestCount;
}

int32 AResourceNode::GetRemainingHarvestCount() const
{
	if (bInfiniteHarvest)
	{
		return -1; // 무한
	}

	return FMath::Max(0, MaxHarvestCount - CurrentHarvestCount);
}

float AResourceNode::GetHarvestProgress() const
{
	if (bInfiniteHarvest || MaxHarvestCount <= 0)
	{
		return 0.0f;
	}

	return static_cast<float>(CurrentHarvestCount) / static_cast<float>(MaxHarvestCount);
}

void AResourceNode::Multicast_PlayDestroyEffect_Implementation()
{
	// 나이아가라 이펙트 재생
	if (DestroyEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DestroyEffect,
			GetActorLocation(),
			GetActorRotation(),
			FVector(3.0f)
		);
	}

	// 사운드 재생
	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DestroySound,
			GetActorLocation()
		);
	}

	LOG_Item_WARNING(TEXT("[ResourceNode] 파괴 연출 재생"));
}

void AResourceNode::DestroyResourceNode()
{
	if (!HasAuthority())
	{
		return;
	}

	Multicast_PlayDestroyEffect();

	LOG_Item_WARNING(TEXT("[ResourceNode] 자원 노드 파괴됨"));
	Destroy();
}