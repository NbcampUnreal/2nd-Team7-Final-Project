#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/GameMode/LCRoomGameMode.h"

#include "Actor/LCDroneDelivery.h"

#include "UI/UIElement/RoomWidget.h"

#include "Engine/World.h"
#include "Misc/PackageName.h"

#include "UI/Manager/LCUIManager.h"
#include "Blueprint/UserWidget.h"
#include "DataType/SessionPlayerInfo.h"

void ALCRoomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CreateAndShowRoomUI();
}


void ALCRoomPlayerController::Client_UpdatePlayerList_Implementation(const TArray<FSessionPlayerInfo>& PlayerInfos)
{
	Super::Client_UpdatePlayerList_Implementation(PlayerInfos);

	FTimerHandle TimerHandle;
	TWeakObjectPtr<ALCRoomPlayerController> WeakPtr = this;
	TArray<FSessionPlayerInfo> InfosCopy = PlayerInfos;

	GetWorld()->GetTimerManager().SetTimer
	(
		TimerHandle,
		[WeakPtr, InfosCopy]()
		{
			if (WeakPtr.IsValid())
			{
				if (WeakPtr->RoomWidgetInstance)
				{
					UE_LOG(LogTemp, Warning, TEXT("Update Lobby UI!!"));
					WeakPtr->RoomWidgetInstance->UpdatePlayerLists(InfosCopy);
				}
			}
		},
		1.0f,
		false
	);

}

void ALCRoomPlayerController::Client_UpdatePlayers_Implementation()
{
	Super::Client_UpdatePlayers_Implementation();

	FTimerHandle TimerHandle;
	TWeakObjectPtr<ALCRoomPlayerController> WeakPtr = this;

	GetWorld()->GetTimerManager().SetTimer
	(
		TimerHandle,
		[WeakPtr]()
		{
			if (WeakPtr.IsValid())
			{
				if (WeakPtr->RoomWidgetInstance)
				{
					UE_LOG(LogTemp, Warning, TEXT("Update Lobby UI!!"));
					WeakPtr->RoomWidgetInstance->UpdatePlayerNames();
				}
			}
		},
		1.0f,
		false
	);

}

void ALCRoomPlayerController::Server_RequestPurchase_Implementation(const TArray<FItemDropData>& DropList)
{
	if (DropList.IsEmpty() || !DroneDeliveryClass)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;

	FVector SpawnLoc = FVector::ZeroVector;

	ALCDroneDelivery* Drone = GetWorld()->SpawnActor<ALCDroneDelivery>(DroneDeliveryClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (Drone)
	{
		Drone->ItemsToDrop = DropList;
		Drone->StartDelivery();
	}
}

void ALCRoomPlayerController::CreateAndShowRoomUI()
{
	if (IsLocalPlayerController())
	{
		if (RoomWidgetClass)
		{
			RoomWidgetInstance = CreateWidget<URoomWidget>(this, RoomWidgetClass);
			if (RoomWidgetInstance)
			{
				RoomWidgetInstance->AddToViewport();
			}
		}
	}
}
