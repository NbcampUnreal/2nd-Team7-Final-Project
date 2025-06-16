// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Drone/BaseDrone.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/BasePlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Net/UnrealNetwork.h"
#include "Item/ItemSpawnerComponent.h"

ABaseDrone::ABaseDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	// 스태틱 메시 생성
	DroneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	RootComponent = DroneMesh; // 이 줄 추가!

	// SpringArm과 Camera 설정
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(DroneMesh);
	SpringArm->TargetArmLength = 0.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// 기타 설정
	SetReplicatingMovement(true);
	bReplicates = true;

	SpringArm->bUsePawnControlRotation = false;
	Camera->bUsePawnControlRotation = false;
	

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = DroneMesh; // RootComponent 또는 DroneMesh로 설정

	ItemSpawner = CreateDefaultSubobject<UItemSpawnerComponent>(TEXT("ItemSpawner"));
}

void ABaseDrone::BeginPlay()
{
	Super::BeginPlay();

	DroneMesh->SetNotifyRigidBodyCollision(true); // 충돌 이벤트 사용하려면 true여야 함
	DroneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DroneMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// 충돌 이벤트 바인딩 (OnComponentHit)
	DroneMesh->OnComponentHit.AddDynamic(this, &ABaseDrone::OnDroneHit);

	MoveDirection = FVector::ForwardVector;

	TargetDroneRotation.Yaw = GetActorRotation().Yaw;
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


	//check Dist to PlayCharacter

	FVector MyLocation = GetActorLocation();
	float Distance = FVector::Dist(MyLocation, CharacterLocation);


	//UE_LOG(LogTemp, Log, TEXT("Distance to CharacterLocation: %.2f"), Distance);

	if (Distance > MaxDistanceToPlayer)
	{
		Multicast_ReturnToPlayer();
	}
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

void ABaseDrone::Input_Look(const FInputActionValue& Value, float Sensivity)
{
	const FVector2D LookInput = Value.Get<FVector2D>();
	Server_Look(LookInput * Sensivity);
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
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server_ReturnAsItem] Authority가 없습니다."));
		return;
	}

	// ⭐ 드론 아이템 스폰
	SpawnDroneItemAtCurrentLocation();
	Destroy();
}

void ABaseDrone::ReturnAsItem()
{
	if (HasAuthority())
	{
		// 서버에서 직접 실행
		Server_ReturnAsItem_Implementation();
	}
	else
	{
		// 클라이언트에서는 서버 RPC 호출
		Server_ReturnAsItem();
	}

	UE_LOG(LogTemp, Warning, TEXT("Drone return to Item"));
}

void ABaseDrone::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseDrone, CameraPitch);
}


void ABaseDrone::SetCharacterLocation(FVector Location)
{
	CharacterLocation = Location;
}

void ABaseDrone::Multicast_ReturnToPlayer_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (ABasePlayerController* MyPC = Cast<ABasePlayerController>(PC))
	{
		MyPC->Input_DroneExit();
	}
}
// CPP 구현
void ABaseDrone::OnDroneHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	FVector Normal = Hit.ImpactNormal;
	FVector CurrVelocity = CurrentVelocity;
	
	// 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("Drone hit %s"), *OtherActor->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Impact Normal: X=%.3f Y=%.3f Z=%.3f"), Normal.X, Normal.Y, Normal.Z);
	UE_LOG(LogTemp, Warning, TEXT("Current Velocity Before Reflection: X=%.3f Y=%.3f Z=%.3f"), CurrVelocity.X, CurrVelocity.Y, CurrVelocity.Z);
	// 기존 벡터 크기 저장
	float Speed = CurrVelocity.Size();

	// 반사 벡터 계산 (방향만)
	FVector ReflectedDir = CurrVelocity.MirrorByVector(Normal).GetSafeNormal();

	// 크기 유지
	FVector Reflected = ReflectedDir * Speed * 0.5f; // 반감 크기

	CurrentVelocity = Reflected;
	if (Normal.Z > 0.0f)
	{
		VerticalVelocity = -VerticalVelocity * Normal.Z * 0.5f;
	}
	else
	{
		VerticalVelocity = VerticalVelocity * Normal.Z * 0.5f;
	}
	// 반사 후 속도 로그
	UE_LOG(LogTemp, Warning, TEXT("Reflected Velocity: X=%.3f Y=%.3f Z=%.3f (Speed=%.3f)"), Reflected.X, Reflected.Y, Reflected.Z, Reflected.Size());


	// 디버그 선으로 노말 벡터 시각화 (초록색, 1초간 표시)
	const float DebugLineLength = 100.0f;
	FVector Start = Hit.ImpactPoint;
	FVector End = Start + Normal * DebugLineLength;
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 2.0f);
}

void ABaseDrone::SpawnDroneItemAtCurrentLocation()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDroneItemAtCurrentLocation] Authority가 없습니다."));
		return;
	}

	if (!ItemSpawner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDroneItemAtCurrentLocation] ItemSpawner가 없습니다."));
		return;
	}

	FVector SpawnLocation = GetActorLocation();
	AItemBase* DroneItem = ItemSpawner->CreateItemAtLocation(DroneItemRowName, SpawnLocation);
	if (DroneItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDroneItemAtCurrentLocation] 드론 아이템 스폰 성공: %s"),
			*DroneItem->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDroneItemAtCurrentLocation] 드론 아이템 스폰 실패"));
	}
}
