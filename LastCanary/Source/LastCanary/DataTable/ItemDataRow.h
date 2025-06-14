#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataType/ResourceCategory.h"
#include "GameplayTagContainer.h"
#include "ItemDataRow.generated.h"

UENUM(BlueprintType)
enum class ENoteType : uint8
{
	Truth			UMETA(DisplayName = "Truth"),		
	Lie				UMETA(DisplayName = "Lie"),			
	Noise			UMETA(DisplayName = "Noise"),		
};

/**
 * 
 */
class AItemBase;
USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

    // [1. 기본 속성]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic Properties")
    int32 ItemID = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic Properties")
    FName ItemName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic Properties")
    FText ItemDescription = FText::FromString(TEXT(""));
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic Properties")
    int32 MaxStack = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Basic Properties")
    float Weight = 1.0f;

    // [2. 경제 및 상호작용]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Economy & Interaction")
    int32 ItemPrice = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Economy & Interaction")
    bool bCanBuy = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Economy & Interaction")
    bool bUsable = false;

    // [3. 시각적 요소]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Visual Elements")
    UTexture2D* ItemIcon = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Visual Elements")
    UStaticMesh* StaticMesh = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Visual Elements")
    USkeletalMesh* SkeletalMesh = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Visual Elements")
    UMaterialInterface* OverrideMaterial = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. Visual Elements", Meta = (Tooltip = "아이템 부착 위치 지정"))
    FName AttachSocketName = TEXT("Rifle");

    // [4. 게임플레이 메커니즘]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Gameplay Mechanics")
    float MaxDurability = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Gameplay Mechanics", Meta = (Categories = "Item"))
    FGameplayTag ItemType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Gameplay Mechanics")
    TSubclassOf<AItemBase> ItemActorClass;

    // [5. 리소스 설정]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5. Resource Settings")
    bool bIsResourceItem = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5. Resource Settings")
    int32 BaseScore = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5. Resource Settings")
    EResourceCategory Category;

    // [6. 물리 설정]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6. Physics Settings", Meta = (Tooltip = "캐릭터와의 충돌 무시 여부"))
    bool bIgnoreCharacterCollision = false;

    // [7. 스폰 조건]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "7. Spawn Conditions", Meta = (Categories = "Map"))
    FGameplayTagContainer AllowedSpawnMaps;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "7. Spawn Conditions", Meta = (Categories = "Time"))
    FGameplayTagContainer TimeConditions;

    // [8. 사운드 설정]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "8. Sound Settings")
    USoundBase* UseStartSound = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "8. Sound Settings")
    USoundBase* UseEndSound = nullptr;


public:
	// 쪽지 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note")
	ENoteType NoteType = ENoteType::Truth;

	// 쪽지 내용 (UI에 표시될 정보)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note", meta = (MultiLine = true))
	FText NoteContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note")
	TArray<TSoftObjectPtr<UTexture2D>> CandidateNoteImages;
};
