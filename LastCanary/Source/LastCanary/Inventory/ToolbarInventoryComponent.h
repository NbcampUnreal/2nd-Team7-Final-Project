// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
#include "DataType/ToolbarItemSlotData.h"
#include "ToolbarInventoryComponent.generated.h"

class ABaseCharacter;
struct FGameplayTag;

UCLASS()
class LASTCANARY_API UToolbarInventoryComponent : public UInventoryComponentBase
{
	GENERATED_BODY()
	
public:
	UToolbarInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual bool TryAddItemSlot(FName ItemRowName, int32 Amount);

	UFUNCTION(Blueprintcallable, Category = Inventory)
	virtual bool TryDecreaseItem(FName ItemRowName, int32 Amount);

	UFUNCTION(Blueprintcallable, Category = Inventory)
	virtual int32 GetItemCount(FName ItemRowName) const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool TrySwapItemSlots(int32 FromIndex, int32 ToIndex);

	UFUNCTION(Blueprintcallable, Category = Inventory)
	bool TryRemoveItemAtSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool TryAddItem(AItemBase * ItemActor);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AttachItem(FName InItemRowName, FName SocketName);

	void OnRepItemSlots();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY()
	ABaseCharacter* CachedOwnerCharacter;

protected:
	virtual void BeginPlay() override;

	bool CanAddItem(AItemBase* ItemActor) override;
	bool TryStoreItem(AItemBase* ItemActor) override;
	void PostAddProcess() override;
	
	bool IsSocketAvailable(ABaseCharacter* OwnerChar, FName SocketName) const;
	bool IsEquipped(ABaseCharacter* OwnerChar, const FGameplayTag& Tag) const;

	void RegisterEquippedItem(AItemBase* Item, FName SocketName);
	void UnregisterEquippedItem(FName SocketName);
};
