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
	UFUNCTION(BlueprintCallable)
	void OnBackButtonClicked();

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void RefreshServerList();

	virtual void RefreshServerList_Implementation();

private:
	/** 자동 갱신 간격 (초) */
	UPROPERTY(EditAnywhere, Category = "Lobby")
	float RefreshInterval = 10.0f;

	/** 서버 리스트 자동 갱신용 타이머 핸들 */
	FTimerHandle ServerListRefreshTimer;

	/** 자동 갱신 시작/중지 */
	void StartAutoRefresh();
	UFUNCTION(BlueprintCallable)
	void StopAutoRefresh();

};
