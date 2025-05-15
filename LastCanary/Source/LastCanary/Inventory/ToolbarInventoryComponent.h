// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
#include "ToolbarInventoryComponent.generated.h"

class ABaseCharacter;
struct FGameplayTag;

UCLASS()
class LASTCANARY_API UToolbarInventoryComponent : public UInventoryComponentBase
{
	GENERATED_BODY()
	
public:
	UToolbarInventoryComponent();

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
