#include "Item/ResourceItem/ResourceItemBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

AResourceItemBase::AResourceItemBase()
{
    bReplicates = true;
    ResourceDataTable = nullptr;
}

void AResourceItemBase::BeginPlay()
{
    Super::BeginPlay();
    ApplyResourceDataFromTable();
}

void AResourceItemBase::ApplyResourceDataFromTable()
{
    if (ResourceDataTable==nullptr)
    {
        LOG_Frame_WARNING(TEXT("[ResourceItem] ResourceDataTable is null"));
        return;
    }

    const FString ContextString(TEXT("Resource Data Lookup"));

    // ItemRowName은 AItemBase에서 상속받음
    FResourceItemRow* FoundRow = ResourceDataTable->FindRow<FResourceItemRow>(ItemRowName, ContextString);
    if (FoundRow)
    {
        ResourceData = *FoundRow;
        LOG_Frame_WARNING(TEXT("[ResourceItem] Successfully loaded resource data: %s"), *ItemRowName.ToString());
    }
    else
    {
        LOG_Frame_WARNING(TEXT("[ResourceItem] Failed to find resource data for: %s"), *ItemRowName.ToString());
    }

    if (FoundRow->Mesh && MeshComponent)
    {
        MeshComponent->SetStaticMesh(FoundRow->Mesh);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ResourceItem] Mesh not set or MeshComponent missing."));
    }

    if (FoundRow->Material && MeshComponent)
    {
        MeshComponent->SetMaterial(0, FoundRow->Material);
    }
}

int32 AResourceItemBase::GetResourceScore() const
{
    return ResourceData.BaseScore;
}

FString AResourceItemBase::GetInteractMessage_Implementation() const
{
    return FString::Printf(TEXT("[F] 채집하기 (%s)"), *ItemRowName.ToString());
}

void AResourceItemBase::UseItem()
{
    // 자원 아이템은 일반적으로 직접 사용하지 않음
    LOG_Frame_WARNING(TEXT("[ResourceItem] 자원 아이템은 직접 사용되지 않음: %s"), *ItemRowName.ToString());
}