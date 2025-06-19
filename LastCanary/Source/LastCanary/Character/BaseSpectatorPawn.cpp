#include "Character/BaseSpectatorPawn.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerState.h"
#include "Character/BaseCharacter.h"

ABaseSpectatorPawn::ABaseSpectatorPawn()
{
    PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = DefaultZoom;
    SpringArm->bUsePawnControlRotation = false;
	SpringArm->bDoCollisionTest = true;
	SpringArm->ProbeChannel = ECC_Camera;
	SpringArm->ProbeSize = 12.0f;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ABaseSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
	
}


void ABaseSpectatorPawn::CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo)
{
	if (!IsLocallyControlled()) return;
	Super::CalcCamera(DeltaTime, ViewInfo);
	
	ViewInfo.Rotation.Roll = 0.0f;
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
	float NewPitch = FMath::Clamp(CurrentPitch + NewPitchInput, -MaxPitchAngle, MaxPitchAngle);

	// Yaw는 그대로
	float NewYaw = CurrentRotation.Yaw + Value.X * Sensivity;

	// 새 회전값 적용
	FRotator NewRotation = FRotator(NewPitch, NewYaw, 0.f);
	Controller->SetControlRotation(NewRotation);
}

void ABaseSpectatorPawn::Handle_VoiceChatting(const FInputActionValue& ActionValue)
{
	const float Value = ActionValue.Get<float>();

	if (Value > 0.5f)
	{
		StartVoiceChat();
	}
	else
	{
		CancelVoiceChat();
	}
}

void ABaseSpectatorPawn::AdjustCameraZoom(float ZoomDelta)
{
    float NewLength = FMath::Clamp(SpringArm->TargetArmLength + ZoomDelta * 10.0f, MinZoom, MaxZoom);
    SpringArm->TargetArmLength = NewLength;
}

void ABaseSpectatorPawn::SpectateOtherUser(ABaseCharacter* TargetCharacter)
{
	if (TargetCharacter)
	{
		if (TargetCharacter->GetMesh())
		{
			AttachToComponent(TargetCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform); 
			SetActorRelativeLocation(FVector(0, 0, 200));  // 예: 위로 50만큼 띄우기
		}
	}
}
