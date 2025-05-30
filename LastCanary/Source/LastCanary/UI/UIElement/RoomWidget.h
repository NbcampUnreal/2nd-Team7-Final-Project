#pragma once

#include "CoreMinimal.h"
#include "DataType/SessionPlayerInfo.h"
#include "UI/LCUserWidgetBase.h"
#include "RoomWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UPlayerSlot;
UCLASS()
class LASTCANARY_API URoomWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category="Room")
	void CreatePlayerSlots();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> PlayerListContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPlayerSlot> PlayerSlotClass;

	UPROPERTY()
	TArray<TObjectPtr<UPlayerSlot>> PlayerNameTextBlocks;
	
	// TODO : 별도의 Widget을 최대 인원수 만큼 만들기
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText_0;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText_1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText_2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText_3;

	UFUNCTION(BlueprintCallable)
	void UpdatePlayerNames();

	UFUNCTION(BlueprintCallable)
	void UpdatePlayerLists(const TArray<FSessionPlayerInfo>& PlayerInfos);
	
};
