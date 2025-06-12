#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "LCGateActor.generated.h"

UENUM(BlueprintType)
enum class EGateTravelType : uint8
{
	ToBaseCamp,
	ToInGame
};

UCLASS()
class LASTCANARY_API ALCGateActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	ALCGateActor();

protected:
	virtual void BeginPlay() override;

public:
    /** 이동할 맵 경로 */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gate")
    int32 TargetMapID;

    /** [F] UI 텍스트 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate")
    FText InteractionPromptText;

    /** 목적지 유형 */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Gate")
    EGateTravelType TravelType;

    /** 상호작용 처리 */
    virtual void Interact_Implementation(APlayerController* Controller) override;
    virtual FString GetInteractMessage_Implementation() const override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

    void ReturnToBaseCamp(APlayerController* Controller);
    void IntoGameLevel(APlayerController* Controller);
};