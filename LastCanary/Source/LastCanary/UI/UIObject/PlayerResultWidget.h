#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/ResourceScoreInfo.h"
#include "Framework/Manager/ChecklistManager.h"
#include "PlayerResultWidget.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class URewardEntry;
class UResourceScoreEntry;
class UDetailResultWidget;


UCLASS()
class LASTCANARY_API UPlayerResultWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SurvivalText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RankText;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* AccuracyText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResourceText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExpText;

	//UPROPERTY(meta = (BindWidget))
	//UTextBlock* GatheredResourceText;

	//UPROPERTY(meta = (BindWidget))
	//UTextBlock* SurviveTimeText;

	//UPROPERTY(meta = (BindWidget))
	//UTextBlock* KillText;

	//UPROPERTY(meta = (BindWidget))
	//UTextBlock* ClueText;

	//UPROPERTY(meta = (BindWidget))
	//UScrollBox* RewardScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDetailResultWidget> DetailWidgetClass;

public:
	void SetChecklistResult(const FPlayerResultData& ResultData);

	void ClearPlayerResult();

private:
	FPlayerResultData CachedResultData;

public:
	UPROPERTY(meta = (BindWidget))
	UButton* DetailButton;
	UFUNCTION()
	void OnDetailButtonClicked();
};
