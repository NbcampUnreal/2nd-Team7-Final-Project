#include "Inventory/InventoryComponentBase.h"
#include "Inventory/InventoryUtility.h"
#include "Inventory/InventoryDropSystem.h"
#include "Inventory/InventoryConfig.h"
#include "Item/ItemSpawnerComponent.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Character/BaseCharacter.h"
#include "DataType/BaseItemSlotData.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

UInventoryComponentBase::UInventoryComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	ItemSpawner = nullptr;
}

void UInventoryComponentBase::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerCharacter();

	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] World is null!"));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] GameInstance is null!"));
		return;
	}

	ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!GISubsystem)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] LCGameInstance is null"));
		return;
	}

	ItemDataTable = GISubsystem->ItemDataTable;
	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] ItemDataTable is null"));
		return;
	}

	if (ABaseCharacter* Character = GetCachedOwnerCharacter())
	{
		ItemSpawner = Character->ItemSpawner;
	}

	// 서버에서만 슬롯 초기화
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InitializeSlots();
	}
}

void UInventoryComponentBase::InitializeSlots()
{
	// 이미 초기화되었다면 건너뛰기
	if (ItemSlots.Num() > 0)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::InitializeSlots] 슬롯이 이미 초기화되어 있습니다."));
		return;
	}

	ItemSlots.Empty();
	ItemSlots.Reserve(MaxSlots);

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		FBaseItemSlotData DefaultSlot;
		DefaultSlot.ItemRowName = DefaultItemRowName;
		DefaultSlot.Quantity = 1;
		DefaultSlot.Durability = 100.0f;
		DefaultSlot.bIsValid = true;
		DefaultSlot.bIsEquipped = false;

		ItemSlots.Add(DefaultSlot);
	}

	// UI 업데이트
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponentBase::CacheOwnerCharacter()
{
	CachedOwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!CachedOwnerCharacter)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::CacheOwnerCharacter] 소유자가 BaseCharacter가 아닙니다."));
	}
}

bool UInventoryComponentBase::IsOwnerCharacterValid() const
{
	return CachedOwnerCharacter && IsValid(CachedOwnerCharacter);
}

ABaseCharacter* UInventoryComponentBase::GetCachedOwnerCharacter() const
{
	return CachedOwnerCharacter;
}

void UInventoryComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponentBase, ItemSlots);
	DOREPLIFETIME(UInventoryComponentBase, MaxSlots);
}

void UInventoryComponentBase::OnRep_ItemSlots()
{
	OnInventoryUpdated.Broadcast();
}

FItemDataRow* UInventoryComponentBase::GetItemRowByName(FName RowName)
{
	if (!ItemDataTable)
	{
		LOG_Item_ERROR(TEXT("[InventoryComponentBase::GetItemRowByName] ItemDataTable이 설정되지 않았습니다."));
		return nullptr;
	}

	if (RowName.IsNone())
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::GetItemRowByName] RowName이 None입니다."));
		return nullptr;
	}

	FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("GetItemRowByName"));

	if (!ItemData)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::GetItemRowByName] '%s' 행을 찾을 수 없습니다."), *RowName.ToString());
		return nullptr;
	}

	return ItemData;
}

int32 UInventoryComponentBase::GetMaxSlots() const
{
	return MaxSlots;
}

bool UInventoryComponentBase::TryDropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		return UInventoryDropSystem::ExecuteDropItem(this, SlotIndex, Quantity);
	}
	else
	{
		Server_TryDropItemAtSlot(SlotIndex, Quantity);
		return true;
	}
}

void UInventoryComponentBase::Server_TryDropItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity)
{
	UInventoryDropSystem::ExecuteDropItem(this, SlotIndex, Quantity);
}

bool UInventoryComponentBase::TryDropItem(FName ItemRowName, int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		return UInventoryDropSystem::ExecuteDropItemByName(this, ItemRowName, Quantity);
	}
	else
	{
		Server_TryDropItem(ItemRowName, Quantity);
		return true;
	}
}

void UInventoryComponentBase::Server_TryDropItem_Implementation(FName ItemRowName, int32 Quantity)
{
	UInventoryDropSystem::ExecuteDropItemByName(this, ItemRowName, Quantity);
}

void UInventoryComponentBase::UpdateWeight()
{
	float OldWeight = CurrentTotalWeight;
	float NewWeight = 0.0f;

	// 전체 슬롯 순회해서 무게 계산
	for (const FBaseItemSlotData& Slot : ItemSlots)
	{
		if (!UInventoryUtility::IsDefaultItem(Slot.ItemRowName, GetInventoryConfig()) && Slot.Quantity > 0)
		{
			float ItemWeight = UInventoryUtility::GetItemWeight(Slot.ItemRowName, ItemDataTable);
			NewWeight += ItemWeight * Slot.Quantity;
		}
	}

	CurrentTotalWeight = NewWeight;
	float WeightDifference = NewWeight - OldWeight;

	// 유의미한 변화가 있을 때만 알림
	if (FMath::Abs(WeightDifference) > 0.01f)
	{
		LOG_Item_WARNING(TEXT("[UpdateWeight] %s 무게 변경: %.2f -> %.2f (차이: %.2f)"),
			*GetClass()->GetName(), OldWeight, NewWeight, WeightDifference);

		OnWeightChanged.Broadcast(NewWeight, WeightDifference);

		if (AActor* Owner = GetOwner())
		{
			if (ABaseCharacter* Character = Cast<ABaseCharacter>(Owner))
			{
				Character->OnInventoryWeightChanged(WeightDifference);
			}
		}
	}
}

UItemSpawnerComponent* UInventoryComponentBase::GetItemSpawner() const
{
	if (ABaseCharacter* Character = GetCachedOwnerCharacter())
	{
		return Character->ItemSpawner;
	}
	return nullptr;
}

void UInventoryComponentBase::ClearInventorySlots()
{
	LOG_Item_WARNING(TEXT("[ClearInventorySlots] 인벤토리 초기화 시작"));

	ItemSlots.Empty();
	CurrentTotalWeight = 0.0f;

	LOG_Item_WARNING(TEXT("[ClearInventorySlots] 인벤토리 초기화 완료"));
}

bool UInventoryComponentBase::HasWalkieTalkieInToolbar() const
{
	for (const FBaseItemSlotData& Slot : ItemSlots)
	{
		if (Slot.bIsValid && UInventoryUtility::IsDefaultItem(Slot.ItemRowName, GetInventoryConfig()) && Slot.Quantity > 0)
		{
			if (IsWalkieTalkieItem(Slot.ItemRowName))
			{
				return true;
			}
		}
	}
	return false;
}

void UInventoryComponentBase::UpdateWalkieTalkieChannelStatus()
{
	if (!IsOwnerCharacterValid())
	{
		return;
	}

	bool bHasWalkieTalkie = HasWalkieTalkieInToolbar();

	UE_LOG(LogTemp, Log, TEXT("[UpdateWalkieTalkieChannelStatus] 워키토키 상태 업데이트: %s"),
		bHasWalkieTalkie ? TEXT("있음") : TEXT("없음"));

	// 서버에서 실행 중인지 확인
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UpdateWalkieTalkieChannelStatus] 클라이언트에서 호출됨 - 무시"));
		return;
	}

	// 소유자의 컨트롤러 확인
	ABaseCharacter* OwnerCharacter = CachedOwnerCharacter;
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UpdateWalkieTalkieChannelStatus] OwnerCharacter가 없음"));
		return;
	}

	APlayerController* PC = OwnerCharacter->GetController<APlayerController>();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UpdateWalkieTalkieChannelStatus] PlayerController가 없음"));
		return;
	}

	// 로컬 플레이어인지 확인 (리슨 서버의 호스트 플레이어)
	if (PC->IsLocalController())
	{
		UE_LOG(LogTemp, Log, TEXT("[UpdateWalkieTalkieChannelStatus] 리슨 서버 호스트 플레이어 - 직접 업데이트"));
		CachedOwnerCharacter->SetWalkieTalkieChannelStatus(bHasWalkieTalkie);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[UpdateWalkieTalkieChannelStatus] 원격 클라이언트 - Client RPC 전송"));
		// 원격 클라이언트에게 RPC 전송
		CachedOwnerCharacter->Client_SetWalkieTalkieChannelStatus(bHasWalkieTalkie);
	}
}

bool UInventoryComponentBase::IsWalkieTalkieItem(FName ItemRowName) const
{
	return UInventoryUtility::IsWalkieTalkieItem(ItemRowName, ItemDataTable);
}

const UInventoryConfig* UInventoryComponentBase::GetInventoryConfig() const
{
	return InventoryConfig; // nullptr이어도 유틸리티에서 기본값 처리
}

bool UInventoryComponentBase::IsDefaultItem(FName ItemRowName) const
{
	return UInventoryUtility::IsDefaultItem(ItemRowName, GetInventoryConfig());
}

void UInventoryComponentBase::SetSlotToDefault(int32 SlotIndex)
{
	if (ItemSlots.IsValidIndex(SlotIndex))
		UInventoryUtility::SetSlotToDefault(ItemSlots[SlotIndex], GetInventoryConfig());
}

int32 UInventoryComponentBase::GetItemIDFromRowName(FName ItemRowName) const
{
	return UInventoryUtility::GetItemIDFromRowName(ItemRowName, ItemDataTable);
}

FName UInventoryComponentBase::GetItemRowNameFromID(int32 ItemID) const
{
	return UInventoryUtility::GetItemRowNameFromID(ItemID, ItemDataTable);
}
