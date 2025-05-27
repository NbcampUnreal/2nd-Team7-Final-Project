#pragma once

#include "CoreMinimal.h"
#include "DataTable/MapDataRow.h"
#include "GameFramework/GameStateBase.h"
#include "LCGameState.generated.h"

UCLASS()
class LASTCANARY_API ALCGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_ReplicatedHasBegunPlay() override;

	virtual void PostInitializeComponents() override;

public:
	UDataTable* GetMapData();

	int32 GetSelectedMapIndex();
	void SetMapIndex(int32 Index);

	void SetPlayerNum();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* MapData;

private:
	UPROPERTY(ReplicatedUsing = OnRep_SelectMapChanged)
	int32 SelectedMapIndex;

	UFUNCTION()
	void OnRep_SelectMapChanged();

	UPROPERTY(ReplicatedUsing = OnRep_PlayerNumChanged)
	int32 PlayerNum;

	UFUNCTION()
	void OnRep_PlayerNumChanged() const;

};
