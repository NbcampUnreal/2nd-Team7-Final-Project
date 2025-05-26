#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ResourceScoreEntry.generated.h"

class UTextBlock;

UCLASS()
class LASTCANARY_API UResourceScoreEntry : public ULCUserWidgetBase
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ResourceNameText;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreDetailText;
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* RevealEntryAnim;

public:
    UFUNCTION(BlueprintCallable)
    void InitResourceScoreEntry(const FText& ResourceName, const FText& ScoreDescription);
};
