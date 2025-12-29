#include "Item/ResourceNode.h"
#include "Item/ResourceItem/ResourceItemSpawnManager.h"
#include "Item/ItemBase.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "DataTable/MonsterDataTable.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "LastCanary.h"

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	MaxHarvestCount = 5;
	CurrentHarvestCount = 0;
	bInfiniteHarvest = false;

	// Geometry Collection 추가
	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	GeometryCollectionComponent->SetupAttachment(RootComponent);
	GeometryCollectionComponent->SetVisibility(false);
	GeometryCollectionComponent->SetSimulatePhysics(false); // 파괴 전엔 물리 적용 안 함
	GeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	// 현재 객체가 클래스 디폴트 객체(CDO)가 아니라면, 즉, 실제 게임에 배치된 인스턴스일 때만
	if (HasAnyFlags(RF_ClassDefaultObject) == false) // CDO 검사
	{
		switch (InteractionType)
		{
		case EResourceInteractionType::Harvest:
			// falls through
		case EResourceInteractionType::Mine:
			bDestroyOnDepletion = true;
			break;
		case EResourceInteractionType::Core:
			// falls through
		case EResourceInteractionType::Chest:
			// falls through
		case EResourceInteractionType::GetNote:
		default:
			bDestroyOnDepletion = false;
			break;
		}
	}
}

void AResourceNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceNode, CurrentHarvestCount);
}

void AResourceNode::Interact_Implementation(APlayerController* Interactor)
{
	if (bRequireTool)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(Interactor->GetPawn());
		if (!Character)
		{
			return;
		}

		if (AItemBase* Equipped = Character->GetToolbarInventoryComponent()->GetCurrentEquippedItem())
		{
			if (!Equipped->ItemData.ItemType.MatchesTag(RequiredToolTag))
			{
				LOG_Item_WARNING(TEXT("[ResourceNode] 올바른 도구가 필요합니다."));
				return;
			}
		}
		else
		{
			LOG_Item_WARNING(TEXT("[ResourceNode] 도구를 장착해야 합니다."));
			return;
		}
	}

	HarvestResource(Interactor);
}

void AResourceNode::Server_RequestInteract_Implementation(APlayerController* Interactor)
{
	// 서버에서 실제 상호작용 처리
	HarvestResource(Interactor);
}

void AResourceNode::HarvestResource(APlayerController* Interactor)
{
	if (InteractionType == EResourceInteractionType::Chest)
	{
		HandleLootSpawn(Interactor);
		return;
	}

	if (!CanHarvest())
	{
		LOG_Item_WARNING(TEXT("[AResourceNode::HarvestResource] 채취 불가능 - 자원이 고갈됨"));
		return;
	}

	if (bRequireTool)
	{
		ABaseCharacter* Character = Interactor ? Cast<ABaseCharacter>(Interactor->GetPawn()) : nullptr;
		if (!Character)
		{
			LOG_Item_WARNING(TEXT("[ResourceNode] 플레이어를 찾을 수 없습니다."));
			return;
		}

		if (AItemBase* Equipped = Character->GetToolbarInventoryComponent()->GetCurrentEquippedItem())
		{
			if (!Equipped->ItemData.ItemType.MatchesTag(RequiredToolTag))
			{
				LOG_Item_WARNING(TEXT("[ResourceNode] 올바른 도구가 필요합니다."));
				return;
			}
		}
		else
		{
			LOG_Item_WARNING(TEXT("[ResourceNode] 도구를 장착해야 합니다."));
			return;
		}
	}

	if (!ResourceItemSpawnManager)
	{
		LOG_Item_WARNING(TEXT("[ResourceNode::HarvestResource] ResourceItemSpawnManager not initialized!"));
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
	Multicast_OnResourceOpened();

	if (!bInfiniteHarvest)
	{
		CurrentHarvestCount++;
		LOG_Item_WARNING(TEXT("[ResourceNode] 채취 완료 (%d/%d)"), CurrentHarvestCount, MaxHarvestCount);

		// 최대 채취 수량에 도달했으면 파괴 예약
		if (CurrentHarvestCount >= MaxHarvestCount)
		{
			if (bDestroyOnDepletion)
			{
				LOG_Item_WARNING(TEXT("[ResourceNode] 자원 고갈! 파괴 예약"));
				DestroyResourceNode();
			}
			else
			{
				LOG_Item_WARNING(TEXT("[ResourceNode] 자원 고갈! 유지됨 (bDestroyOnDepletion = false)"));
				Multicast_PlayDestroyEffect();
			}
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
	case EResourceInteractionType::Chest:
		return NSLOCTEXT("Interaction", "Loot", "Press [{Key}] to Open Chest");
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

	APlayerController* LocalPC = nullptr;

	// 로컬 플레이어 컨트롤러 찾기
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->IsLocalController())
		{
			LocalPC = PC;
			break;
		}
	}

	if (!LocalPC)
	{
		return TEXT("No interaction (no local player)");
	}

	ABaseCharacter* Character = Cast<ABaseCharacter>(LocalPC->GetPawn());
	if (!Character)
	{
		return TEXT("No interaction (no character)");
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

	if (bRequireTool == false)
	{
		FText Template = GetDefaultMessageForType(InteractionType);
		return FText::Format(Template, FFormatNamedArguments{ {"Key", FText::FromString(KeyName)} }).ToString();
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

void AResourceNode::HandleLootSpawn(APlayerController* Interactor)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (CanHarvest() == false)
	{
		return;
	}
	if (ResourceItemSpawnManager == nullptr)
	{
		return;
	}

	UDataTable* ItemTable = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				ItemTable = Subsystem->GetItemDataTable();
			}
		}
	}
	if (ItemTable == nullptr)
	{
		LOG_Item_WARNING(TEXT("[ResourceNode::HandleLootSpawn] 아이템 데이터 테이블 없음"));
		return;
	}

	TArray<FName> CandidateRows;
	const FGameplayTag RuinsTag = FGameplayTag::RequestGameplayTag("ItemSpawn.Map.Ruins");

	for (const auto& Pair : ItemTable->GetRowMap())
	{
		const FItemDataRow* Row = reinterpret_cast<const FItemDataRow*>(Pair.Value);
		if (Row && Row->bIsResourceItem && Row->AllowedSpawnMaps.HasTagExact(RuinsTag))
		{
			CandidateRows.Add(Pair.Key);
		}
	}

	if (CandidateRows.Num() == 0)
	{
		LOG_Item_WARNING(TEXT("[ResourceNode::HandleLootSpawn] 조건에 맞는 아이템이 없음"));
		return;
	}

	const int32 NumToSpawn = FMath::Min(MaxLootItemTypes, CandidateRows.Num());
	TArray<FName> SelectedRows;

	while (SelectedRows.Num() < NumToSpawn)
	{
		const int32 RandIndex = FMath::RandRange(0, CandidateRows.Num() - 1);
		const FName& Row = CandidateRows[RandIndex];

		if (!SelectedRows.Contains(Row))
		{
			SelectedRows.Add(Row);
		}
	}

	for (const FName& ItemRow : SelectedRows)
	{
		const FVector SpawnLocation = CalculateResourceSpawnLocation(Interactor);
		ResourceItemSpawnManager->SpawnItemAtLocation(ItemRow, SpawnLocation);
	}

	Multicast_OnResourceOpened();
	CurrentHarvestCount = MaxHarvestCount; // 즉시 상호작용 금지 처리

	// 몬스터 스폰: 조건 필터링
	if (MonsterDataTable && FMath::FRand() <= MonsterSpawnProbability)
	{
		TArray<FName> MonsterRowNames = MonsterDataTable->GetRowNames();
		TArray<const FMonsterDataTable*> Candidates;

		for (const FName& RowName : MonsterRowNames)
		{
			const FMonsterDataTable* Row = MonsterDataTable->FindRow<FMonsterDataTable>(RowName, TEXT("ChestMonster"));
			if (Row == nullptr || Row->MonsterActor == nullptr)
			{
				continue;
			}

			// 현재 맵 태그와 일치하는 경우만
			if (Row->Level.IsValid())
			{
				// 참고: 경로 비교 또는 태그 방식으로 대체 가능
				const FString LevelName = Row->Level.GetAssetName(); // 예: "RuinsMap"
				if (LevelName.Contains("Ruins"))
				{
					Candidates.Add(Row);
				}
			}
		}

		if (Candidates.Num() > 0)
		{
			const FMonsterDataTable* Selected = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];

			// 캐릭터 뒤쪽에서 스폰
			APawn* Pawn = Interactor ? Interactor->GetPawn() : nullptr;
			if (Pawn == nullptr)
			{
				LOG_Item_WARNING(TEXT("[LootSpawn] 몬스터 스폰 실패 - 인터랙터 없음"));
				return;
			}

			const FVector CharacterLocation = Pawn->GetActorLocation();
			const FVector Backward = -Pawn->GetActorForwardVector();
			const FVector SpawnLoc = CharacterLocation + Selected->SpawnOffset + (Backward * 150.f);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AActor* Spawned = GetWorld()->SpawnActor<AActor>(Selected->MonsterActor, SpawnLoc, FRotator::ZeroRotator, Params);
			if (IsValid(Spawned))
			{
				Spawned->SetReplicates(true);
				LOG_Item_WARNING(TEXT("[Loot] 몬스터 스폰 성공: %s (뒤쪽에서 등장)"), *Selected->MonsterName.ToString());
			}
		}
	}
}

void AResourceNode::Multicast_PlayDestroyEffect_Implementation()
{
	// 1. Geometry Collection 활성화
	if (InteractionType == EResourceInteractionType::Mine && GeometryCollectionComponent)
	{
		TArray<USceneComponent*> ChildComponents;
		GetRootComponent()->GetChildrenComponents(true, ChildComponents);

		for (USceneComponent* Child : ChildComponents)
		{
			if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Child))
			{
				Mesh->SetVisibility(false, true); // 전체 자식까지
				Mesh->SetHiddenInGame(true, true);
				Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}

		GeometryCollectionComponent->SetVisibility(true);
		GeometryCollectionComponent->SetSimulatePhysics(true);
		GeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		GeometryCollectionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // 바닥에만 충돌

		//GeometryCollectionComponent->ApplyExternalStrain(5000.0f, GetActorLocation());

		GeometryCollectionComponent->AddForceAtLocation(FVector::UpVector * -3000.0f, GetActorLocation());

		//GeometryCollectionComponent->AddRadialImpulse(
		//	GetActorLocation(),  // 위치
		//	300.0f,              // 반경
		//	6000.0f,             // 세기
		//	ERadialImpulseFalloff::RIF_Linear,
		//	false                 // velocity 변경 허용
		//);

		//GeometryCollectionComponent->ApplyLinearVelocity(0, GetActorForwardVector());

		LOG_Frame_WARNING(TEXT("[ResourceNode] Geometry Collection 활성화 및 물리 적용"));
	}

	// 2. 나이아가라 이펙트
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

	// 3. 사운드
	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroySound, GetActorLocation());
	}

	LOG_Item_WARNING(TEXT("[ResourceNode] 파괴 연출 재생 (Mine 포함)"));
}

void AResourceNode::DestroyResourceNode()
{
	if (HasAuthority() == false)
	{
		return;
	}

	Multicast_PlayDestroyEffect();

	if (InteractionType == EResourceInteractionType::Mine)
	{
		// 시각적 파괴만 하고 실제 액터는 파괴 X
		// SetActorEnableCollision(false);
		// SetActorHiddenInGame(true);
		SetLifeSpan(5.0f); // 몇 초 뒤 삭제
	}
	else
	{
		LOG_Item_WARNING(TEXT("[ResourceNode] 자원 노드 파괴됨"));
		Destroy();
	}
}

void AResourceNode::Multicast_OnResourceOpened_Implementation()
{
	OnResourceOpened();
}

void AResourceNode::OnResourceOpened_Implementation()
{
	// 기본 연출 없음. 필요시 블루프린트에서 오버라이드.
}