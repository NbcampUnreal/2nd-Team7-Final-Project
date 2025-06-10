#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "GameplayTagContainer.h"
#include "ResourceNode.generated.h"

USTRUCT(BlueprintType)
struct FResourceItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FName ItemRowName = FName(TEXT("Default"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = 0.0, ClampMax = 100.0))
    float Probability = 25.0;
};

UENUM(BlueprintType)
enum class EResourceSpawnLocationType : uint8
{
    NodeTop UMETA(DisplayName = "ResourceNode 위"),
    MidpointToCharacter UMETA(DisplayName = "캐릭터-ResourceNode 중간"),
    CustomOffset UMETA(DisplayName = "커스텀 오프셋(ResourceNode 기준)")
};

class AResourceItemSpawnManager;

UCLASS()
class LASTCANARY_API AResourceNode : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:
	AResourceNode();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Interact_Implementation(APlayerController* Interactor) override;

    UFUNCTION(Server, Reliable)
    void Server_RequestInteract(APlayerController* Interactor);
    void Server_RequestInteract_Implementation(APlayerController* Interactor);

    void HarvestResource(APlayerController* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
    FString GetInteractMessage() const;
    virtual FString GetInteractMessage_Implementation() const;

    FVector CalculateResourceSpawnLocation(APlayerController* Interactor) const;

    UPROPERTY(EditAnywhere)
    AResourceItemSpawnManager* ResourceItemSpawnManager;

    // 드랍시 선택할 아이템 RowName 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    TArray<FResourceItemData> PossibleResourceItems;

    // 적절한 상호작용 도구 태그
    UPROPERTY(EditAnywhere, Category = "Resource")
    FGameplayTag RequiredToolTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Message")
    FText InteractSuccessMessage = FText::FromString(TEXT("F key를 눌러 채집하세요."));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Message")
    FText InteractFailMessage = FText::FromString(TEXT("상호작용 불가({ToolName} 아이템 필요)"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Spawn")
    EResourceSpawnLocationType SpawnLocationType = EResourceSpawnLocationType::NodeTop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Spawn", meta = (EditCondition = "SpawnLocationType==EResourceSpawnLocationType::CustomOffset"))
    FVector CustomSpawnOffset = FVector(0, 0, 100);
};
