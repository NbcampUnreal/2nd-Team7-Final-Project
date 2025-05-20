#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Parts.generated.h"

UCLASS()
class LASTCANARY_API AParts : public AActor
{
	GENERATED_BODY()
	
public:	
	AParts();

	FName PartsType;
	FName SocketNameForAttach;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USceneComponent* Scene;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* PartsMesh;
};
