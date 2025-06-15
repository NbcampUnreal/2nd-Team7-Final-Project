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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 ItemID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ItemIcon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemPrice = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemDescription = FText::FromString(TEXT(""));
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBuy = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Item"))
	FGameplayTag ItemType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachSocketName = TEXT("Rifle");;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* StaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* OverrideMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AItemBase> ItemActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	bool bIsResourceItem = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 BaseScore = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bIgnoreCharacterCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", Meta = (Categories = "Map"))
	FGameplayTagContainer AllowedSpawnMaps;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", Meta = (Categories = "Time"))
	FGameplayTagContainer TimeConditions;


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
