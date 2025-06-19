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

void UInventoryComponentBase::ShowTooltipForItem(const FBaseItemSlotData& ItemData, UWidget* TargetWidget)
{
	// UI 매니저에서 관리할 예정
}

void UInventoryComponentBase::HideTooltip()
{
	// UI 매니저에서 관리할 예정
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

FVector UInventoryComponentBase::CalculateDropLocation() const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		LOG_Item_WARNING(TEXT("[CalculateDropLocation] Owner is null"));
		return FVector::ZeroVector;
	}

	ABaseCharacter* Character = Cast<ABaseCharacter>(OwnerActor);
	if (!Character)
	{
		LOG_Item_WARNING(TEXT("[CalculateDropLocation] Owner is not BaseCharacter"));
		return FVector::ZeroVector;
	}

	// ⭐ 카메라 시점 기준으로 던지기 위치 계산
	AController* Controller = Character->GetController();
	if (!Controller)
	{
		LOG_Item_WARNING(TEXT("[CalculateDropLocation] Controller is null"));
		return Character->GetActorLocation();
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// ⭐ 카메라 앞쪽 일정 거리에서 생성 (손에서 던지는 느낌)
	FVector ThrowStartOffset = CameraRotation.Vector() * 150.0f; // 카메라 앞 150cm
	FVector HandOffset = Character->GetActorUpVector() * -20.0f; // 살짝 아래쪽 (손 높이)

	FVector ThrowStartLocation = CameraLocation + ThrowStartOffset + HandOffset;

	// 바닥과 너무 가까우면 조정
	FHitResult HitResult;
	FVector TraceStart = ThrowStartLocation + FVector(0, 0, 100.0f);
	FVector TraceEnd = ThrowStartLocation - FVector(0, 0, 200.0f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	QueryParams.bTraceComplex = true;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		float MinHeight = HitResult.ImpactPoint.Z + 80.0f; // 바닥에서 80cm 위
		ThrowStartLocation.Z = FMath::Max(ThrowStartLocation.Z, MinHeight);
	}

	LOG_Item_WARNING(TEXT("[CalculateDropLocation] 던지기 시작 위치: %s"), *ThrowStartLocation.ToString());
	return ThrowStartLocation;
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

bool UInventoryComponentBase::Internal_TryDropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] Owner is null"));
		return false;
	}

	if (!Owner->HasAuthority())
	{
		LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] Authority가 없습니다. 서버 RPC를 통해 요청하세요."));
		LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] Owner: %s, HasAuthority: %s"),
			*Owner->GetName(), Owner->HasAuthority() ? TEXT("true") : TEXT("false"));

		// ⭐ 클라이언트에서는 자동으로 서버 RPC 호출
		Server_TryDropItemAtSlot(SlotIndex, Quantity);
		return true; // 클라이언트에서는 요청 성공으로 처리
	}

	if (!ItemSlots.IsValidIndex(SlotIndex))
	{
		LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] 유효하지 않은 슬롯 인덱스: %d"), SlotIndex);
		return false;
	}

	if (!IsOwnerCharacterValid())
	{
		LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] CachedOwnerCharacter가 유효하지 않습니다."));
		return false;
	}

	FBaseItemSlotData& SlotData = ItemSlots[SlotIndex];

	// ⭐ Default 아이템 체크를 실제 드롭 전으로 이동
	if (IsDefaultItem(SlotData.ItemRowName))
	{
		LOG_Item_WARNING(TEXT("[Internal_TryDropItemAtSlot] Default 아이템은 드롭할 수 없습니다: 슬롯 %d"), SlotIndex);
		return false;
	}

	if (SlotData.ItemRowName.IsNone() || SlotData.Quantity <= 0)
	{
		LOG_Item_WARNING(TEXT("[Internal_TryDropItemAtSlot] 빈 슬롯입니다: %d"), SlotIndex);
		return false;
	}

	int32 DropQuantity = FMath::Min(Quantity, SlotData.Quantity);
	if (DropQuantity <= 0)
	{
		LOG_Item_WARNING(TEXT("[Internal_TryDropItemAtSlot] 드랍할 수량이 0 이하입니다."));
		return false;
	}

	FVector DropLocation = CalculateDropLocation();
	FBaseItemSlotData DropItemData = SlotData;
	DropItemData.Quantity = DropQuantity;

	if (!ItemSpawner)
	{
		LOG_Item_WARNING(TEXT("[Internal_TryDropItemAtSlot] ItemSpawner is null"));
		return false;
	}

	AItemBase* DroppedItem = ItemSpawner->CreateItemFromData(DropItemData, DropLocation);
	if (!DroppedItem)
	{
		LOG_Item_WARNING(TEXT("[Internal_TryDropItemAtSlot] 아이템 스폰 실패"));
		return false;
	}

	if (AGunBase* DroppedGun = Cast<AGunBase>(DroppedItem))
	{
		DroppedGun->CurrentFireMode = static_cast<EFireMode>(DropItemData.FireMode);
		DroppedGun->bIsAutoFiring = false;
	}

	// ⭐ 중복 제거 및 정리
	SlotData.Quantity -= DropQuantity;
	if (SlotData.Quantity <= 0)
	{
		SetSlotToDefault(SlotIndex); // ⭐ 여기서만 Default 설정
	}

	UpdateWeight(); // ⭐ 무게 갱신 추가
	UpdateWalkieTalkieChannelStatus();

	OnInventoryUpdated.Broadcast();

	LOG_Item_WARNING(TEXT("[Internal_TryDropItemAtSlot] ✅ 드롭 성공: %s (수량: %d)"),
		*DropItemData.ItemRowName.ToString(), DropQuantity);

	return true;
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

bool UInventoryComponentBase::TryDropItem_Internal(FName ItemRowName, int32 Quantity)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		LOG_Item_WARNING(TEXT("[UInventoryComponentBase::TryDropItem_Internal] Authority가 없습니다."));
		return false;
	}

	if (ItemRowName.IsNone() || Quantity <= 0)
	{
		LOG_Item_WARNING(TEXT("[UInventoryComponentBase::TryDropItem_Internal] 유효하지 않은 매개변수"));
		return false;
	}

	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i].ItemRowName == ItemRowName && ItemSlots[i].Quantity > 0)
		{
			int32 DropQuantity = FMath::Min(Quantity, ItemSlots[i].Quantity);
			return Internal_TryDropItemAtSlot(i, DropQuantity);
		}
	}

	LOG_Item_WARNING(TEXT("[UInventoryComponentBase::TryDropItem_Internal] 아이템을 찾을 수 없습니다: %s"), *ItemRowName.ToString());
	return false;
}

void UInventoryComponentBase::UpdateWeight()
{
	float OldWeight = CurrentTotalWeight;
	float NewWeight = 0.0f;

	// ⭐ 간단하게 전체 슬롯 순회해서 무게 계산
	for (const FBaseItemSlotData& Slot : ItemSlots)
	{
		if (!Slot.ItemRowName.IsNone() && Slot.Quantity > 0)
		{
			float ItemWeight = GetItemWeight(Slot.ItemRowName);
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

		// 캐릭터에 알림
		if (AActor* Owner = GetOwner())
		{
			if (ABaseCharacter* Character = Cast<ABaseCharacter>(Owner))
			{
				Character->OnInventoryWeightChanged(WeightDifference);
			}
		}
	}
}

float UInventoryComponentBase::GetItemWeight(FName ItemRowName) const
{
	return UInventoryUtility::GetItemWeight(ItemRowName, ItemDataTable);
}

bool UInventoryComponentBase::IsDefaultItem(FName ItemRowName) const
{
	return UInventoryUtility::IsDefaultItem(ItemRowName, GetInventoryConfig());
}

void UInventoryComponentBase::SetSlotToDefault(int32 SlotIndex)
{
	if (!ItemSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	UInventoryUtility::SetSlotToDefault(ItemSlots[SlotIndex], GetInventoryConfig());
	OnInventoryUpdated.Broadcast();
}

UItemSpawnerComponent* UInventoryComponentBase::GetItemSpawner() const
{
	if (ABaseCharacter* Character = GetCachedOwnerCharacter())
	{
		return Character->ItemSpawner;
	}
	return nullptr;
}

int32 UInventoryComponentBase::GetItemIDFromRowName(FName ItemRowName) const
{
	return UInventoryUtility::GetItemIDFromRowName(ItemRowName, ItemDataTable);
}

FName UInventoryComponentBase::GetItemRowNameFromID(int32 ItemID) const
{
	return UInventoryUtility::GetItemRowNameFromID(ItemID, ItemDataTable);
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
		if (Slot.bIsValid && !IsDefaultItem(Slot.ItemRowName) && Slot.Quantity > 0)
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