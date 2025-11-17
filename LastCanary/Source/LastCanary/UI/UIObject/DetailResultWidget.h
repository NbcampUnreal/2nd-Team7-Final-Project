#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/ResourceScoreInfo.h"
#include "Framework/Manager/ChecklistManager.h"
#include "DetailResultWidget.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class URewardEntry;
class UResourceScoreEntry;

UCLASS()
class LASTCANARY_API UDetailResultWidget : public ULCUserWidgetBase
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
	UTextBlock* SurviveTimeText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AccuracyText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GatheredResourceText;
	UPROPERTY(meta = (BindWidget))
	UScrollBox* RewardScrollBox;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UResourceScoreEntry> ResourceEntryClass;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* SurvivalPointText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SurviveTimePointText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AccuracyPointText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillPointText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ClueText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CorrectClueText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WorngClueText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NoiseClueText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExpText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResourceText;
	//UPROPERTY(meta = (BindWidget))
	//UTextBlock* ScoreText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RankText;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;


public:
	void SetChecklistResult(const FPlayerResultData& ResultData);
	//void ClearPlayerResult();

private:
	int32 CurrentResourceIndex = 0;
	bool bIsAddingResources = false;

	TArray<FResultRewardEntry> CachedEntries;
	TArray<FResourceScoreInfo> CachedResourceDetails;
	int32 CurrentEntryIndex = 0;
	FTimerHandle EntryAddTimerHandle;

	void AddNextEntry();

public:
	UFUNCTION()
	void OnExitButtonClicked();

};
