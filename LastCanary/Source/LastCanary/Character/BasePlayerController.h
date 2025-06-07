#pragma once

#include "CoreMinimal.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "Character/PlayerData/PlayerDataTypes.h"
#include "BasePlayerController.generated.h"

struct FInputActionValue;
class UEnhancedInputComponent;
class UInputMappingContext;
class UInputAction;
class ABaseCharacter;
class ABaseDrone;
class ABasePlayerState;
class ALCBaseGimmick;

UCLASS()
class LASTCANARY_API ABasePlayerController : public ALCPlayerController
{
	GENERATED_BODY()


private:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
private:
	APawn* CachedPawn;  // Pawn을 저장할 멤버 변수
	APawn* CurrentPossessedPawn;
	ABaseCharacter* SpanwedPlayerCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_SpawnedPlayerDrone)
	ABaseDrone* SpawnedPlayerDrone;

	UFUNCTION()
	void OnRep_SpawnedPlayerDrone();

protected:
	UEnhancedInputComponent* EnhancedInput;
	UInputMappingContext* CurrentIMC;
public:
	virtual void SetupInputComponent() override;

	void ApplyInputMappingContext(UInputMappingContext* IMC);

	void RemoveInputMappingContext(UInputMappingContext* IMC);

	UFUNCTION(BlueprintCallable)
	void OnPossess(APawn* InPawn);

	UFUNCTION(BlueprintCallable)
	void OnUnPossess();

	void OnRep_Pawn();

	void ClientRestart(APawn* NewPawn);

	UFUNCTION(BlueprintCallable)
	APawn* GetMyPawn();

	UFUNCTION(BlueprintCallable)
	void SetMyPawn(APawn* NewPawn);

	UFUNCTION(BlueprintCallable)
	void ChangeInputMappingContext(UInputMappingContext* IMC);

	virtual void BeginPlay() override;
	virtual void InitInputComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> LookMouseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ViewModeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ItemUseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ThrowItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> RifleReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> VoiceAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ChangeShootingSettingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> StrafeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ChangeQuickSlotAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SelectQuickSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SelectQuickSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SelectQuickSlot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SelectQuickSlot4Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> OpenPauseMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Als Character Example", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ExitDroneAction;
	// ... 필요한 입력들 추가

public:
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);

	virtual void Input_OnLook(const FInputActionValue& ActionValue);

	virtual void Input_OnMove(const FInputActionValue& ActionValue);

	virtual void Input_OnSprint(const FInputActionValue& ActionValue);

	virtual void Input_OnWalk(const FInputActionValue& ActionValue);

	virtual void Input_OnCrouch(const FInputActionValue& ActionValue);

	virtual void Input_OnJump(const FInputActionValue& ActionValue);

	virtual void Input_OnAim(const FInputActionValue& ActionValue);

	virtual void Input_OnViewMode();

	virtual void Input_OnInteract();

	virtual void Input_OnStrafe(const FInputActionValue& ActionValue);

	virtual void Input_OnItemUse();

	virtual void Input_OnItemThrow();

	virtual void Input_OnStartedVoiceChat();

	virtual void Input_OnCanceledVoiceChat();

	virtual void Input_ChangeShootingSetting();

	virtual void Input_Reload();

	virtual void Input_ChangeQuickSlot(const FInputActionValue& ActionValue);

	virtual void Input_SelectQuickSlot1();

	virtual void Input_SelectQuickSlot2();

	virtual void Input_SelectQuickSlot3();

	virtual void Input_SelectQuickSlot4();

	virtual void Input_OpenPauseMenu();

	virtual void Input_DroneExit();

public:
	void ChangeToNextQuickSlot();
	void ChangeToPreviousQuickSlot();
	void SelectQuickSlot(int32 SlotIndex);
public:
	// 상호작용 가능한 액터 감지
	AActor* TraceInteractable(float TraceDistance = 300.f);

public:
	void UpdateQuickSlotUI();

public:
	bool IsPossessingBaseCharacter() const;

	//빙의된 캐릭터를 반환하는 함수
	UFUNCTION(BlueprintCallable, Category = "Character")
	ABaseCharacter* GetControlledBaseCharacter() const;

public:
	void OnExitGate();

	UFUNCTION(Server, Reliable)
	void Server_OnExitGate();
	void Server_OnExitGate_Implementation();

	void HandleExitGate();

	UFUNCTION()
	void OnPlayerExitActivePlay();

	UFUNCTION(Client, Reliable)
	void Client_OnPlayerExitActivePlay();
	void Client_OnPlayerExitActivePlay_Implementation();

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentSpectatedCharacterIndex = 0;


	void SpectateNextPlayer();
	void SpectatePreviousPlayer();
	TArray<ABasePlayerState*> GetPlayerArray();

	TArray<ABasePlayerState*> SpectatorTargets;

	//관전 컨트롤 전용 변수
	bool bIsSpectatingButtonClicked = false;
public:
	bool bIsSprinting = false;

public:
	void SetHardLandStateToPlayerState(bool flag);
	void SetSprintingStateToPlayerState(bool flag);

	void CameraSetOnScope();

public:
	//총기 발사 세팅(단발 or 점사 or 연사)
	void SetShootingSetting();

public:
	void SpawnDrone();

	UFUNCTION(Server, Reliable)
	void Server_SpawnDrone();
	void Server_SpawnDrone_Implementation();

	//test용
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABaseDrone> DroneClass;

	void PossessOnDrone();



public:
	void InteractGimmick(ALCBaseGimmick* Target);

	UFUNCTION(Server, Reliable)
	void Server_InteractWithGimmick(ALCBaseGimmick* Target);
	void Server_InteractWithGimmick_Implementation(ALCBaseGimmick* Target);
};