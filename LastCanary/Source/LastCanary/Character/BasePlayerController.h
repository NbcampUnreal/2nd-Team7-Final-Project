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
	void InitInputComponent();

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


	//인풋모드 변경(Toggle, Hold)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EInputMode SprintInputMode = EInputMode::Hold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EInputMode WalkInputMode = EInputMode::Hold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EInputMode CrouchInputMode = EInputMode::Hold;

	bool bIsWalkToggled = false;

	bool bIsCrouchToggled = false;
	bool bIsCrouchKeyReleased = true;

	bool bIsRunKeyHeld = false;
	bool bWantsToRun = false;

public:
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);

	virtual void Input_OnLook(const FInputActionValue& ActionValue);

	virtual void Input_OnMove(const FInputActionValue& ActionValue);

	virtual void Input_OnSprint(const FInputActionValue& ActionValue);
	
	virtual void End_OnSprint(const FInputActionValue& ActionValue);

	UFUNCTION()
	void Complete_OnSprint();

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
	UFUNCTION()
	void OnCharacterDamaged(float CurrentHP);

	UFUNCTION()
	void OnCharacterDied();

	UFUNCTION(Client, Reliable)
	void Client_OnCharacterDied();
	void Client_OnCharacterDied_Implementation();




	//SpectatorMode
	UFUNCTION(BlueprintImplementableEvent)
	void TEST_CallSpectatorWidget();

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentSpectatedCharacterIndex = 0;

	void SpectateNextPlayer();
	void SpectatePreviousPlayer();
	TArray<ABasePlayerState*> GetPlayerArray();

	TArray<ABasePlayerState*> SpectatorTargets;

public:
	bool bIsSprinting = false;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Test")
	float TestStamina = 100.0f;

	UFUNCTION()
	void OnStaminaUpdated(float NewStamina);

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	float TestHP = 100.0f;


public:
	void SetHardLandStateToPlayerState(bool flag);
	void SetSprintingStateToPlayerState(bool flag);

public:

	// Recoil 상태
	FTimerHandle RecoilTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite);
	float YawRecoilRange = 1.0f;

	int32 RecoilStep = 0;
	int32 RecoilMaxSteps = 5;
	float RecoilStepPitch = 0.f;
	float RecoilStepYaw = 0.f;

	void CameraShake();
	void ApplyRecoilStep();

	void CameraSetOnScope();
	

	UFUNCTION(BlueprintCallable)
	void SetPlayerMovementSetting();

	UFUNCTION(BlueprintCallable)
	void ChangePlayerMovementSetting(float _WalkForwardSpeed, float _WalkBackwardSpeed, float _RunForwardSpeed, float _RunBackwardSpeed, float _SprintSpeed);

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
};