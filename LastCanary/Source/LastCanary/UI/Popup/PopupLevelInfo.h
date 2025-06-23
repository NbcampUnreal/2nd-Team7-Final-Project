#pragma once

#include "CoreMinimal.h"
#include "DataTable/MapDataRow.h"
#include "UI/LCUserWidgetBase.h"
#include "PopupLevelInfo.generated.h"

class UTextBlock;

UCLASS()
class LASTCANARY_API UPopupLevelInfo : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
	void InitLevelInfoWidget(FMapDataRow* MapDataRow);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextMapName;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextDescription;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* LevelInfoAnim;

	UFUNCTION()
	void OnInfoAnimationFinished();
};
