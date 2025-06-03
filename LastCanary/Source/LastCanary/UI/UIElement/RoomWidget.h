#pragma once

#include "CoreMinimal.h"
#include "DataType/SessionPlayerInfo.h"
#include "UI/LCUserWidgetBase.h"
#include "RoomWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UButton;
class UPlayerSlot;
UCLASS()
class LASTCANARY_API URoomWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category="Room")
	void CreatePlayerSlots();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> PlayerListContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPlayerSlot> PlayerSlotClass;

	UPROPERTY()
	TArray<TObjectPtr<UPlayerSlot>> PlayerSlots;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* InviteButton;

	UFUNCTION(BlueprintCallable)
	void UpdatePlayerLists(const TArray<FSessionPlayerInfo>& PlayerInfos);

protected:
	UFUNCTION()
	void OnInviteButtonClicked();

private:
	TArray<FSessionPlayerInfo> SessionPlayerInfos;
	int MaxPlayerNum = 4;
	
};
