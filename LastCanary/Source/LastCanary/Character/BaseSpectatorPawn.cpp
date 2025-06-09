#include "Character/BaseSpectatorPawn.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerState.h"

ABaseSpectatorPawn::ABaseSpectatorPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = DefaultZoom;
    SpringArm->bUsePawnControlRotation = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ABaseSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseSpectatorPawn::Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity)
{
	const FVector2f Value{ ActionValue.Get<FVector2D>() };
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (MyPlayerState->InGameState != EPlayerInGameStatus::Spectating)
	{
		return;
	}

	// 현재 컨트롤러 회전
	FRotator CurrentRotation = Controller->GetControlRotation();
	float CurrentPitch = CurrentRotation.GetNormalized().Pitch;

	// 입력값 계산
	const float NewPitchInput = Value.Y * Sensivity;

	// Pitch 제한 적용
	float NewPitch = FMath::Clamp(CurrentPitch + NewPitchInput, MinPitchAngle, MaxPitchAngle);

	// Yaw는 그대로
	float NewYaw = CurrentRotation.Yaw + Value.X * Sensivity;

	// 새 회전값 적용
	FRotator NewRotation = FRotator(NewPitch, NewYaw, 0.f);
	Controller->SetControlRotation(NewRotation);
}

void ABaseSpectatorPawn::AdjustCameraZoom(float ZoomDelta)
{
    float NewLength = FMath::Clamp(SpringArm->TargetArmLength + ZoomDelta, MinZoom, MaxZoom);
    SpringArm->TargetArmLength = NewLength;
}