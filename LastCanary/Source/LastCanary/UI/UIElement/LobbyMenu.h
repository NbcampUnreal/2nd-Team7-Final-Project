#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "LobbyMenu.generated.h"

/**
 * 
 */
class UButton;
class UScrollBox;
class UVerticalBox;

UCLASS()
class LASTCANARY_API ULobbyMenu : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* CreateRoomButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* RefreshButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* RoomListBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* ServerListBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BackButton;

private:
	UFUNCTION()
	void OnCreateRoomButtonClicked();
	UFUNCTION()
	void OnRefreshButtonClicked();
	UFUNCTION()
	void OnBackButtonClicked();

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void RefreshServerList();

	virtual void RefreshServerList_Implementation();

};
