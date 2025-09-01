#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomizationPreviewScene.generated.h"

UCLASS()
class LASTCANARY_API ACustomizationPreviewScene : public AActor
{
	GENERATED_BODY()

public:
	ACustomizationPreviewScene();

protected:
	virtual void BeginPlay() override;

public:
	void SetPreviewMesh(USkeletalMesh* NewMesh);

	void UpdateCapture();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	class USkeletalMeshComponent* PreviewMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	class USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	class UDirectionalLightComponent* DirectionalLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	class UStaticMeshComponent* GroundPlane;
};
