#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "GameplayTagContainer.h"
#include "ResourceNode.generated.h"

//-------------------------
// 구조체 및 열거형
//-------------------------

USTRUCT(BlueprintType)
struct FResourceItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ItemRowName = FName(TEXT("Default"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = 0.0, ClampMax = 100.0))
	float Probability = 100.0f;
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
	Core		UMETA(DisplayName = "Core"),
	Mine		UMETA(DisplayName = "Mine"),
	Chest		UMETA(DisplayName = "Chest"),
	GetNote		UMETA(DisplayName = "Get Note"),
};

class AResourceItemSpawnManager;
class UInputAction;
class UNiagaraSystem;
class UGeometryCollectionComponent;
UCLASS()
class LASTCANARY_API AResourceNode : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	//-------------------------
	// 생성자 및 엔진 오버라이드
	//-------------------------

	AResourceNode();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//-------------------------
	// 인터랙션 관련 오버라이드
	//-------------------------

	virtual void Interact_Implementation(APlayerController* Interactor) override;
	virtual FString GetInteractMessage_Implementation() const;

	//-------------------------
	// 상호작용 서버 함수
	//-------------------------

	UFUNCTION(Server, Reliable)
	void Server_RequestInteract(APlayerController* Interactor);
	void Server_RequestInteract_Implementation(APlayerController* Interactor);

	//-------------------------
	// 상호작용 기능
	//-------------------------

	void HarvestResource(APlayerController* Interactor);
	FText GetDefaultMessageForType(EResourceInteractionType Type) const;
	FVector CalculateResourceSpawnLocation(APlayerController* Interactor) const;
	FString GetCurrentKeyNameForInteract() const;

	//-------------------------
	// 수확 관련 함수
	//-------------------------

	UFUNCTION(BlueprintCallable, Category = "Resource")
	bool CanHarvest() const;
	UFUNCTION(BlueprintCallable, Category = "Resource")
	int32 GetRemainingHarvestCount() const;
	UFUNCTION(BlueprintCallable, Category = "Resource")
	float GetHarvestProgress() const;

	/** 몬스터가 등장할 확률 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, Category = "Chest|Monster")
	float MonsterSpawnProbability = 0.3f; // 30% 확률

	/** 몬스터 스폰 오프셋 */
	UPROPERTY(EditAnywhere, Category = "Chest|Monster")
	FVector MonsterSpawnOffset = FVector(0.f, 0.f, 50.f);

	/** 몬스터 데이터 테이블 (Chest에서 몬스터 등장 시 사용) */
	UPROPERTY(EditAnywhere, Category = "Loot|Monster")
	UDataTable* MonsterDataTable;

	/** 현재 맵에서 허용되는 태그 (데이터테이블의 Level 필터용) */
	UPROPERTY(EditAnywhere, Category = "Loot|Monster")
	FGameplayTag CurrentMapTag;

	// Loot 타입 최대 종류 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Loot", meta = (ClampMin = 1, ClampMax = 10))
	int32 MaxLootItemTypes = 3;

	// Loot 핸들링 함수
	void HandleLootSpawn(APlayerController* Interactor);

	//-------------------------
	// 연출 관련 함수
	//-------------------------

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayDestroyEffect();
	void Multicast_PlayDestroyEffect_Implementation();

private:
	void DestroyResourceNode();

public:
	//-------------------------
	// 상호작용 설정
	//-------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Interact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	EResourceInteractionType InteractionType = EResourceInteractionType::Harvest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Message")
	FText InteractSuccessMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Message")
	FText InteractFailMessage;

	//-------------------------
	// 아이템 관련 설정
	//-------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TArray<FResourceItemData> PossibleResourceItems;

	UPROPERTY(EditAnywhere, Category = "Resource")
	FGameplayTag RequiredToolTag;

	UPROPERTY(EditAnywhere)
	AResourceItemSpawnManager* ResourceItemSpawnManager;

	//-------------------------
	// 스폰 위치 설정
	//-------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Spawn")
	EResourceSpawnLocationType SpawnLocationType = EResourceSpawnLocationType::NodeTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Spawn", meta = (EditCondition = "SpawnLocationType==EResourceSpawnLocationType::CustomOffset"))
	FVector CustomSpawnOffset = FVector(0, 0, 100);

	//-------------------------
	// 수확 제한 설정
	//-------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Harvest", meta = (ClampMin = 1))
	int32 MaxHarvestCount = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource|Harvest", Replicated)
	int32 CurrentHarvestCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Harvest")
	bool bInfiniteHarvest = false;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequireTool = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Harvest")
	bool bDestroyOnDepletion = true;

	//-------------------------
	// 이펙트 및 사운드
	//-------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UGeometryCollectionComponent* GeometryCollectionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* DestroyEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	USoundBase* DestroySound;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnResourceOpened();
	void Multicast_OnResourceOpened_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Resource|Interaction")
	void OnResourceOpened();
};
