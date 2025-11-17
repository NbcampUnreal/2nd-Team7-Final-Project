#include "Item/EquipmentItem/HarvestingTool.h"
#include "Item/ResourceNode.h"
#include "Character/BaseCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "GameplayTagContainer.h"
#include "LastCanary.h"

AHarvestingTool::AHarvestingTool()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AHarvestingTool::BeginPlay()
{
    Super::BeginPlay();
}

void AHarvestingTool::UseItem()
{
    if (!HasAuthority())
    {
        // 클라이언트에서는 서버에 요청만 전송
        Server_RequestHarvest();
        return;
    }

    // 서버에서 직접 실행
    Server_RequestHarvest_Implementation();
}

void AHarvestingTool::Server_RequestHarvest_Implementation()
{
    if (!IsHarvestReady())
    {
        LOG_Item_WARNING(TEXT("[Server_RequestHarvest] 채취 준비되지 않음 (쿨다운 또는 내구도)"));
        return;
    }

    // 가장 가까운 자원 노드 찾기
    AResourceNode* TargetNode = FindNearestResourceNode();
    if (!TargetNode)
    {
        LOG_Item_WARNING(TEXT("[Server_RequestHarvest] 채취 가능한 자원 노드가 범위 내에 없음"));
        return;
    }

    // 채취 가능 여부 확인
    if (!CanHarvestNode(TargetNode))
    {
        LOG_Item_WARNING(TEXT("[Server_RequestHarvest] 이 도구로 해당 자원을 채취할 수 없음"));
        return;
    }

    // 소유자(플레이어) 확인
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        LOG_Item_WARNING(TEXT("[Server_RequestHarvest] 소유자가 없음"));
        return;
    }

    ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor);
    if (!OwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[Server_RequestHarvest] 소유자가 BaseCharacter가 아님"));
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PlayerController)
    {
        LOG_Item_WARNING(TEXT("[Server_RequestHarvest] PlayerController를 찾을 수 없음"));
        return;
    }

    // 자원 채취 실행
    TargetNode->HarvestResource(PlayerController);

    // 내구도 소모
    ConsumeDurability();

    // 쿨다운 설정
    LastHarvestTime = GetWorld()->GetTimeSeconds();

    // 사운드 재생
    PlaySoundByType();

    LOG_Item_WARNING(TEXT("[Server_RequestHarvest] ✅ 자원 채취 완료"));
}

AResourceNode* AHarvestingTool::FindNearestResourceNode() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FHitResult HitResult;
    FVector StartLocation, EndLocation;

    // 라인트레이스 수행
    bool bHit = PerformHarvestTrace(HitResult, StartLocation, EndLocation);

    // 디버그 라인 표시
    if (bShowDebugTrace)
    {
        if (bHit)
        {
            // 시작점에서 히트 지점까지 녹색 라인
            DrawDebugLine(World, StartLocation, HitResult.ImpactPoint, FColor::Green, false, 0.5f, 0, 2.0f);
            // 히트 지점에 노란색 구체 표시
            DrawDebugSphere(World, HitResult.ImpactPoint, 15.0f, 12, FColor::Yellow, false, 0.5f);
        }
        else
        {
            // 전체 라인을 빨간색으로 표시
            DrawDebugLine(World, StartLocation, EndLocation, FColor::Red, false, 0.5f, 0, 2.0f);
        }
    }

    // 히트한 액터가 ResourceNode인지 확인
    if (bHit && HitResult.GetActor())
    {
        AResourceNode* ResourceNode = Cast<AResourceNode>(HitResult.GetActor());
        if (ResourceNode)
        {
            return ResourceNode;
        }
    }

    return nullptr;
}

bool AHarvestingTool::CanHarvestNode(AResourceNode* Node) const
{
    if (!Node)
    {
        return false;
    }

    // 도구 태그와 자원 노드의 필요 도구 태그 매칭 확인
    if (!Node->RequiredToolTag.IsValid())
    {
        // 특별한 도구가 필요하지 않은 자원일 경우 그냥 실행
        return true;
    }

    // 이 도구의 태그가 자원 노드가 요구하는 태그와 일치하는지 확인
    return ItemData.ItemType.MatchesTag(Node->RequiredToolTag);
}

void AHarvestingTool::ConsumeDurability()
{
    if (DurabilityConsumption <= 0.0f)
    {
        return;
    }

    Durability = FMath::Max(0.0f, Durability - DurabilityConsumption);

    if (FMath::IsNearlyZero(Durability))
    {
        LOG_Item_WARNING(TEXT("[ResourceHarvestingTool] 도구 내구도 소진! 교체 필요"));

        // 내구도가 0이 되면 인벤토리에서 제거하거나 사용 불가 상태로 만들기
        TryRemoveFromInventory();
    }

    OnItemStateChanged.Broadcast();
}

bool AHarvestingTool::IsHarvestReady() const
{
    // 내구도 체크
    if (Durability <= 0.0f)
    {
        return false;
    }

    // 쿨다운 체크
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastHarvestTime < HarvestCooldown)
    {
        return false;
    }

    return true;
}

bool AHarvestingTool::PerformHarvestTrace(FHitResult& OutHit, FVector& StartLocation, FVector& EndLocation) const
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        LOG_Item_WARNING(TEXT("[HarvestingTool] PerformHarvestTrace: Owner is NULL"));
        return false;
    }

    ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor);
    if (!OwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[HarvestingTool] PerformHarvestTrace: Owner is not a BaseCharacter - Actual class: %s"),
            *OwnerActor->GetClass()->GetName());

        // 다른 캐스팅 시도
        APawn* OwnerPawn = Cast<APawn>(OwnerActor);
        if (OwnerPawn)
        {
            LOG_Item_WARNING(TEXT("[HarvestingTool] PerformHarvestTrace: Owner is a Pawn, trying to get controller"));
            AController* Controller = OwnerPawn->GetController();
            if (Controller)
            {
                LOG_Item_WARNING(TEXT("[HarvestingTool] PerformHarvestTrace: Controller found: %s"),
                    *Controller->GetClass()->GetName());
            }
        }
        return false;
    }

    // 컨트롤러가 있는지 확인
    AController* Controller = OwnerCharacter->GetController();
    if (!Controller)
    {
        LOG_Item_WARNING(TEXT("[HarvestingTool] PerformHarvestTrace: No controller found"));
        return false;
    }

    // 플레이어 시점 정보 가져오기
    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // 카메라 방향 벡터 계산
    FVector TraceDirection = CameraRotation.Vector();

    // 트레이스 시작점과 끝점 설정
    StartLocation = CameraLocation;
    EndLocation = StartLocation + TraceDirection * HarvestRange;

    // 트레이스 파라미터 설정
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(OwnerActor);
    QueryParams.bTraceComplex = true;

    // 라인트레이스 실행
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );

    return bHit;
}
