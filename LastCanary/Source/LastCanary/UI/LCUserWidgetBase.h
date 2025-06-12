#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Manager/LCUIManager.h"
#include "LCUserWidgetBase.generated.h"

/**
 * UIManager에 접근하기 위한 기본 위젯 클래스
 */
class ULCUIManager;
UCLASS()
class LASTCANARY_API ULCUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	ULCUIManager* ResolveUIManager() const;
};
