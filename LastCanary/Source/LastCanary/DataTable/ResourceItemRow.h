#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataType/ResourceCategory.h"
#include "ResourceItemRow.generated.h"

/**
 * 개별 자원 정보 테이블
 */
USTRUCT(BlueprintType)
struct FResourceItemRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ResourceID; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BaseScore = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ResourceIcon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* Material = nullptr;
};
