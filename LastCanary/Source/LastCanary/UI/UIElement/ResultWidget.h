
#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "Framework/Manager/ChecklistManager.h"
#include "ResultWidget.generated.h"

class UPlayerResultWidget;
class UHorizontalBox;
class UTextBlock;
class UButton;
UCLASS()
class LASTCANARY_API UResultWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetTotalResultData(const FTotalResultData& TotalResult);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultHeader;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MVPText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RankText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalResourceText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PaymentText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* IncomeText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExpText;

	UPROPERTY(meta = (BindWidget))
	UButton* AcceptButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WaitHostText;

	//UPROPERTY(EditAnywhere, Category = "UI")
	//TSubclassOf<UPlayerResultWidget> PlayerResultEntryClass;

	TArray<UPlayerResultWidget*> PlayerResults;

	UPROPERTY(meta = (BindWidget))
	UPlayerResultWidget* PlayerResult1;

	UPROPERTY(meta = (BindWidget))
	UPlayerResultWidget* PlayerResult2;

	UPROPERTY(meta = (BindWidget))
	UPlayerResultWidget* PlayerResult3;

	UPROPERTY(meta = (BindWidget))
	UPlayerResultWidget* PlayerResult4;

private:
	void ClearPlayerResult();

	UFUNCTION()
	void OnAcceptClicked();

	int32 IncomeGold;
};
