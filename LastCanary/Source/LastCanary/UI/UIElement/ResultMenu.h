#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/ResourceScoreInfo.h"
#include "Framework/Manager/ChecklistManager.h"
#include "ResultMenu.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class URewardEntry;
class UResourceScoreEntry;

USTRUCT(BlueprintType)
struct FResultRewardEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RewardType; // 보상 종류 (예: "Gold", "Item", "Experience" 등)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description; // 보상 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardGold; // 보상 수량
};

UCLASS()
class LASTCANARY_API UResultMenu : public ULCUserWidgetBase
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* RewardScrollBox;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TotalGoldText;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* RankText;
    UPROPERTY(meta = (BindWidget))
    UButton* AcceptButton;
    UPROPERTY(Transient, meta = (BindWidgetAnim), BlueprintReadOnly)
    UWidgetAnimation* FadeInRankAnim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    TSubclassOf<URewardEntry> RewardEntryClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    TSubclassOf<UResourceScoreEntry> ResourceScoreEntryClass;

    UFUNCTION()
    void OnAcceptClicked();

private:
    int32 CurrentResourceIndex = 0;
    bool bIsAddingResources = false;

    TArray<FResultRewardEntry> CachedEntries;
    TArray<FResourceScoreInfo> CachedResourceDetails;
    int32 CurrentEntryIndex = 0;
    FTimerHandle EntryAddTimerHandle;

    void AddNextEntry();

public:
    void SetRewardEntries(const TArray<FResultRewardEntry>& InEntries);
    void SetResourceScoreDetails(const TArray<FResourceScoreInfo>& InDetails);
    void SetTotalGold(int32 InTotalGold);
    void SetRankText(const FString& InRank);
    void ActivateResultCamera();
    void SetChecklistResult(const FChecklistResultData& ResultData);

};
