// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Drone/BaseDrone.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Net/UnrealNetwork.h"

ABaseDrone::ABaseDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	// 루트 컴포넌트 설정
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// 스태틱 메시 생성
	DroneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	DroneMesh->SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 0.f;
	

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	PrimaryActorTick.bCanEverTick = true;
	SetReplicatingMovement(true); // redundancy safety
	bReplicates = true;

	SpringArm->bUsePawnControlRotation = false;
	Camera->bUsePawnControlRotation = false;

}

void ABaseDrone::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// --- 수평 이동 ---
	if (!CurrentVelocity.IsNearlyZero())
	{
		CurrentVelocity = FMath::VInterpConstantTo(CurrentVelocity, FVector::ZeroVector, DeltaTime, HorizontalDeceleration);
	}
	const FVector HorizontalOffset = CurrentVelocity * DeltaTime;

	// --- 수직 이동 ---
	if (FMath::Abs(VerticalInputAxis) < 0.5f)
	{
		// 입력이 없으면 감속
		VerticalVelocity = FMath::FInterpTo(VerticalVelocity, 0.f, DeltaTime, VerticalDeceleration);
	}
	else
	{
		// 입력 방향으로 가속
		VerticalVelocity += VerticalInputAxis * VerticalAcceleration * DeltaTime;
		VerticalVelocity = FMath::Clamp(VerticalVelocity, -VerticalMaxSpeed, VerticalMaxSpeed);
	}

	const FVector VerticalOffset = FVector::UpVector * VerticalVelocity * DeltaTime;

	// 최종 이동
	AddActorWorldOffset(HorizontalOffset + VerticalOffset, true);
}

void ABaseDrone::Input_Move(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();
	Server_Move(InputVector);
}
void ABaseDrone::Server_Move_Implementation(FVector2D InputVector)
{
	FInputActionValue WrappedValue(InputVector);
	Move(WrappedValue);
}
void ABaseDrone::Move(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();
	// 단위 방향 (이동 방향을 나타냄)	
	const FVector Forward = GetActorForwardVector();
	const FVector Right = GetActorRightVector();

	const FVector DesiredDir = (Forward * InputVector.Y + Right * InputVector.X).GetSafeNormal();
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 현재 속도에 가속도 추가
	CurrentVelocity += DesiredDir * HorizontalAcceleration * DeltaTime;

	// 최대 속도 제한
	if (CurrentVelocity.Size() > HorizontalMaxSpeed)
	{
		CurrentVelocity = CurrentVelocity.GetClampedToMaxSize(HorizontalMaxSpeed);
	}
}

void ABaseDrone::Input_MoveUp(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	Server_MoveUp(Input);
	// Space 누를 때는 +1, 뗄 때는 0
	//VerticalInputAxis = FMath::Clamp(Value.Get<float>(), 0.f, 1.f);
}

void ABaseDrone::Server_MoveUp_Implementation(float Value)
{
	// Space 누를 때는 +1, 뗄 때는 0
	VerticalInputAxis = FMath::Clamp(Value, 0.f, 1.f);
}

void ABaseDrone::MoveUp(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();

	// Space 누를 때는 +1, 뗄 때는 0
	VerticalInputAxis = FMath::Clamp(Value.Get<float>(), 0.f, 1.f);
}

void ABaseDrone::Input_MoveDown(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	// Ctrl 누를 때는 -1, 뗄 때는 0
	Server_MoveDown(Input);
	//VerticalInputAxis = FMath::Clamp(-Value.Get<float>(), -1.f, 0.f);
}

void ABaseDrone::Server_MoveDown_Implementation(float Value)
{
	VerticalInputAxis = FMath::Clamp(-Value, -1.f, 0.f);
}
void ABaseDrone::MoveDown(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	// Ctrl 누를 때는 -1, 뗄 때는 0
	VerticalInputAxis = FMath::Clamp(-Value.Get<float>(), -1.f, 0.f);
}

void ABaseDrone::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();
	Server_Look(LookInput);
}

void ABaseDrone::Server_Look_Implementation(FVector2D InputVector)
{
	FInputActionValue WrappedValue(InputVector);
	Look(WrappedValue);
}

void ABaseDrone::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	// 드론 좌우 회전 (Yaw)
	TargetDroneRotation.Yaw += LookInput.X * LookSensitivity * GetWorld()->GetDeltaSeconds();

	// 카메라 위아래 회전 (Pitch) - Clamp로 제한
	CameraPitch = FMath::Clamp(CameraPitch + LookInput.Y * LookSensitivity * GetWorld()->GetDeltaSeconds(), -60.f, 60.f);

	// 드론 회전 적용
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw = TargetDroneRotation.Yaw;
	SetActorRotation(NewRotation);

	// 카메라가 드론에 붙어있으면, 카메라 컴포넌트의 Pitch만 업데이트
	if (Camera)
	{
		FRotator CameraRot = Camera->GetRelativeRotation();
		CameraRot.Pitch = CameraPitch;
		Camera->SetRelativeRotation(CameraRot);
	}
}	

void ABaseDrone::OnRep_CameraPitch()
{
	if (Camera)
	{
		FRotator CameraRot = Camera->GetRelativeRotation();
		CameraRot.Pitch = CameraPitch;
		Camera->SetRelativeRotation(CameraRot);
	}
}


void ABaseDrone::Server_ReturnAsItem_Implementation()
{
	Destroy();
}

void ABaseDrone::ReturnAsItem()
{
	Server_ReturnAsItem();
	UE_LOG(LogTemp, Warning, TEXT("Drone return to Item"));
	//TODO: Spawn Drone Item Class
	
}

void ABaseDrone::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseDrone, CameraPitch);
}