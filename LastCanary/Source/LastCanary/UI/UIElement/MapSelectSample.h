#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapSelectSample.generated.h"

class UComboBoxString;
class UButton;
class ALCRoomGameMode;
class ALCGameState;
class UImage;
class UTextBlock;

UCLASS()
class LASTCANARY_API UMapSelectSample : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* MapComboBox;

	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapName;

	UPROPERTY(meta = (BindWidget))
	UImage* Thumnail;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapDesciprtion;

private:
	UFUNCTION()
	void OnMapSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectType);
	
	UFUNCTION()
	void OnStartButtonClicked();
	
	ALCGameState* GetLCGameState() const;

};
