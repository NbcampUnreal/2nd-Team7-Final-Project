#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ResultMenu.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class URewardEntry;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    TSubclassOf<URewardEntry> RewardEntryClass;

    UFUNCTION()
    void OnAcceptClicked();

private:
    TArray<FResultRewardEntry> CachedEntries;

public:
    void SetRewardEntries(const TArray<FResultRewardEntry>& InEntries);
    void SetTotalGold(int32 InTotalGold);
    void SetRankText(const FString& InRank);
    void ActivateResultCamera();
};
