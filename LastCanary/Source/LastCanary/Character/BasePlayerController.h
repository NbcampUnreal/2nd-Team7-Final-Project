#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

struct FInputActionValue;
class UEnhancedInputComponent;
class UInputMappingContext;
class UInputAction;
class ABaseCharacter;
class ABaseDrone;

UCLASS()
class LASTCANARY_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:
	APawn* CachedPawn;  // Pawn을 저장할 멤버 변수
	ABaseCharacter* SpanwedPlayerCharacter;
	ABaseDrone* SpawnedPlayerDrone;

	UEnhancedInputComponent* EnhancedInput;
	UInputMappingContext* CurrentIMC;
public:
protected:
	virtual void SetupInputComponent() override;

	void ApplyInputMappingContext(UInputMappingContext* IMC);

	void RemoveInputMappingContext(UInputMappingContext* IMC);

	UFUNCTION(BlueprintCallable)
	void OnPossess(APawn* InPawn);

	UFUNCTION(BlueprintCallable)
	void OnUnPossess();

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
	TObjectPtr<UInputMappingContext> DroneInputMappingContext;

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
	
	// ... 필요한 입력들 추가

private:
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);

	virtual void Input_OnLook(const FInputActionValue& ActionValue);

	virtual void Input_OnMove(const FInputActionValue& ActionValue);

	virtual void Input_OnSprint(const FInputActionValue& ActionValue);
	
	virtual void End_OnSprint(const FInputActionValue& ActionValue);

	UFUNCTION()
	void Complete_OnSprint();

	virtual void Input_OnWalk(const FInputActionValue& ActionValue);

	virtual void Input_OnCrouch();

	virtual void Input_OnJump(const FInputActionValue& ActionValue);

	virtual void Input_OnAim(const FInputActionValue& ActionValue);

	virtual void Input_OnViewMode();

	virtual void Input_OnInteract();

	virtual void Input_OnStrafe(const FInputActionValue& ActionValue);

	virtual void Input_OnItemUse();

	virtual void Input_ChangeQuickSlot(const FInputActionValue& ActionValue);

	virtual void Input_SelectQuickSlot1();

	virtual void Input_SelectQuickSlot2();

	virtual void Input_SelectQuickSlot3();

	virtual void Input_SelectQuickSlot4();	

	virtual void Input_OpenPauseMenu();

public:
	void ChangeToNextQuickSlot();
	void ChangeToPreviousQuickSlot();

public:
	// 상호작용 가능한 액터 감지
	AActor* TraceInteractable(float TraceDistance = 300.f);

public:
	//퀵슬롯 칸 최대 칸 수
	int32 MaxQuickSlotCount = 4;
	//현재 퀵슬롯 인덱스
	int32 CurrentQuickSlotIndex = 0;

	void UpdateQuickSlotUI();
	void RequestChangeItem(int Itemindex);

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
	void CameraShake();

};