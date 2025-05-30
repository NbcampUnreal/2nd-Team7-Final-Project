#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerSlot.generated.h"

class UTextBlock;
struct FSessionPlayerInfo;

UCLASS()
class LASTCANARY_API UPlayerSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* IndexText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NickNameText;

	void SetSlotIndex(const int Index);
	void SetPlayerInfo(const FSessionPlayerInfo& PlayerInfo);
};
