#include "CustomizationPreviewScene.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACustomizationPreviewScene::ACustomizationPreviewScene()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Preview 캐릭터 Mesh
	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(Root);
	PreviewMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	PreviewMesh->SetCastShadow(true);

	// SceneCapture (카메라)
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(Root);
	SceneCapture->SetRelativeLocation(FVector(-200.f, 0.f, 100.f));
	SceneCapture->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
	SceneCapture->FOVAngle = 30.f;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->bCaptureEveryFrame = true;

	// ShowFlags: 게임 월드 관련 제거
	SceneCapture->ShowFlags.SetGame(false);
	SceneCapture->ShowFlags.SetPostProcessing(true);
	SceneCapture->ShowFlags.SetLighting(true);
	SceneCapture->ShowFlags.SetDynamicShadows(true);

	// 조명 (방향)
	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
	DirectionalLight->SetupAttachment(Root);
	DirectionalLight->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	DirectionalLight->Intensity = 5.0f;

	// 바닥 플레인 (중립 회색)
	GroundPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundPlane"));
	GroundPlane->SetupAttachment(Root);
	GroundPlane->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GroundPlane->SetRelativeScale3D(FVector(4.f, 4.f, 1.f));

	// 기본 머티리얼/메시 적용
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		GroundPlane->SetStaticMesh(PlaneMesh.Object);
		GroundPlane->SetCastShadow(false);
		GroundPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 머티리얼은 추후 커스터마이징
}

void ACustomizationPreviewScene::BeginPlay()
{
	Super::BeginPlay();
}

void ACustomizationPreviewScene::SetPreviewMesh(USkeletalMesh* NewMesh)
{
	if (NewMesh)
	{
		PreviewMesh->SetSkeletalMesh(NewMesh);
	}
}

void ACustomizationPreviewScene::UpdateCapture()
{
	if (SceneCapture)
	{
		SceneCapture->CaptureScene();
	}
}
