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

UENUM(BlueprintType)
enum class EResourceInteractionType : uint8
{
    Harvest		UMETA(DisplayName = "Harvest"),  
    Mine		UMETA(DisplayName = "Mine"),     
    Loot		UMETA(DisplayName = "Loot"),     
    GetNote		UMETA(DisplayName = "Get Note"), 
};

class AResourceItemSpawnManager;
class UInputAction;
class UNiagaraSystem;
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

    FText GetDefaultMessageForType(EResourceInteractionType Type) const;

    /*UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
    FString GetInteractMessage() const;*/
    virtual FString GetInteractMessage_Implementation() const;

    FVector CalculateResourceSpawnLocation(APlayerController* Interactor) const;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_Interact;

    FString GetCurrentKeyNameForInteract() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    EResourceInteractionType InteractionType = EResourceInteractionType::Harvest;

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

    // 채취 수량 제한
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Harvest", meta = (ClampMin = 1))
    int32 MaxHarvestCount = 5;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource|Harvest", Replicated)
    int32 CurrentHarvestCount = 0;

    // 무한 채취 가능 여부 (MaxHarvestCount 무시)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Harvest")
    bool bInfiniteHarvest = false;

    // 나이아가라를 이용한 파괴 연출 
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    UNiagaraSystem* DestroyEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    USoundBase* DestroySound;

    // 네트워크 함수
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayDestroyEffect();
    void Multicast_PlayDestroyEffect_Implementation();

    // 유틸리티 함수
    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool CanHarvest() const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 GetRemainingHarvestCount() const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    float GetHarvestProgress() const;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    void DestroyResourceNode();
};
