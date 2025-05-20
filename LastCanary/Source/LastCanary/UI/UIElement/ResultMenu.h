#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ResultMenu.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;

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

public:
    virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

    /** 정산 항목 데이터를 UI에 표시 */
    void SetRewardEntries(const TArray<FResultRewardEntry>& InEntries);

    /** 총 골드 설정 함수 */
    void SetTotalGold(int32 InTotalGold);

    /** 카메라 뷰 관련 함수 (필요 시 외부 호출) */
    void ActivateResultCamera();

protected:
    UPROPERTY(meta = (BindWidget))
    UScrollBox* RewardScrollBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TotalGoldText;

    UPROPERTY(meta = (BindWidget))
    UButton* AcceptButton;

    UFUNCTION()
    void OnAcceptClicked();

private:
    TArray<FResultRewardEntry> CachedEntries;
};
