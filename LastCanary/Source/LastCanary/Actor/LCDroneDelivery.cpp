#include "LCDroneDelivery.h"
#include "LCDronePath.h"
#include "Item/ItemSpawnerComponent.h"
#include "Item/ItemBase.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "LastCanary.h"

ALCDroneDelivery::ALCDroneDelivery()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SetReplicateMovement(true);

	DroneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	SetRootComponent(DroneMesh);

	DropBoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DropBoxMesh"));
	DropBoxMesh->SetupAttachment(DroneMesh);

	ItemSpawner = CreateDefaultSubobject<UItemSpawnerComponent>(TEXT("ItemSpawner"));
}

void ALCDroneDelivery::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && DropBoxMesh)
	{
		DropBoxMesh->OnComponentHit.AddDynamic(this, &ALCDroneDelivery::OnDropBoxHit);
	}

	InitializeLights(); 

	if (HasAuthority() && DropBoxMesh)
	{
		DropBoxMesh->OnComponentHit.AddDynamic(this, &ALCDroneDelivery::OnDropBoxHit);
	}
}

void ALCDroneDelivery::StartDelivery()
{
	if (HasAuthority()==false)
	{
		return;
	}

	// 가장 가까운 드론 경로 찾기
	DronePathActor = FindNearestDronePath();
	ExternalSpline = DronePathActor ? DronePathActor->SplinePath : nullptr;

	if (ExternalSpline==nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Drone] No valid spline found for delivery."));
		Destroy();
		return;
	}

	BeginDelivery();
}

ALCDronePath* ALCDroneDelivery::FindNearestDronePath()
{
	FVector MyLocation = GetActorLocation();
	float ClosestDistSq = FLT_MAX;
	ALCDronePath* NearestPath = nullptr;

	for (TActorIterator<ALCDronePath> It(GetWorld()); It; ++It)
	{
		float DistSq = FVector::DistSquared(It->GetActorLocation(), MyLocation);
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			NearestPath = *It;
		}
	}

	return NearestPath;
}

void ALCDroneDelivery::Server_StartDeliveryWithPath_Implementation(ALCDronePath* InDronePathActor)
{
	StartDelivery(); // 서버에서 직접 실행
}

void ALCDroneDelivery::BeginDelivery()
{
	if (ExternalSpline == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Drone] Invalid spline!"));
		return;
	}

	ElapsedTime = 0.0f;

	GetWorldTimerManager().SetTimer(
		LightBlinkTimerHandle,
		this,
		&ALCDroneDelivery::ToggleLights,
		LightBlinkInterval,
		true
	);

	GetWorldTimerManager().SetTimer(
		MoveTimerHandle,
		this,
		&ALCDroneDelivery::UpdateLocationOnSpline,
		UpdateInterval,
		true
	);
}

void ALCDroneDelivery::UpdateLocationOnSpline()
{
	if (ExternalSpline == nullptr)
	{
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		return;
	}

	ElapsedTime += UpdateInterval;
	const float Alpha = FMath::Clamp(ElapsedTime / MoveDuration, 0.0f, 1.0f);

	const float Distance = ExternalSpline->GetSplineLength() * Alpha;
	
	const FVector NewLocation = ExternalSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	SetActorLocation(NewLocation);

	const FRotator NewRotation = ExternalSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	SetActorRotation(NewRotation);

	if (Alpha >= 1.0f)
	{
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		DropBox();
	}
}

void ALCDroneDelivery::DropBox()
{
	Multicast_PlayDropEffect();

	GetWorldTimerManager().ClearTimer(LightBlinkTimerHandle);

	if (HasAuthority() == true)
	{
		DropBoxMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DropBoxMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DropBoxMesh->SetCollisionObjectType(ECC_PhysicsBody);
		DropBoxMesh->SetSimulatePhysics(true);
		DropBoxMesh->SetNotifyRigidBodyCollision(true);
	}

	Multicast_DropBoxDetach();

	FTimerHandle EscapeTimerHandle;
	GetWorldTimerManager().SetTimer(
		EscapeTimerHandle,
		this,
		&ALCDroneDelivery::FlyAwayAndDie,
		EscapeDelay,
		false
	);
}

void ALCDroneDelivery::FlyAwayAndDie()
{
	if (ExternalSpline == nullptr)
	{
		return;
	}

	const float SplineLength = ExternalSpline->GetSplineLength();
	const FVector EndLoc = ExternalSpline->GetLocationAtDistanceAlongSpline(SplineLength, ESplineCoordinateSpace::World);
	const FVector Dir = ExternalSpline->GetDirectionAtDistanceAlongSpline(SplineLength, ESplineCoordinateSpace::World);

	TargetLocation = EndLoc + Dir * EscapeDistance;
	StartLocation = GetActorLocation();
	EscapeAlpha = 0.f;

	GetWorldTimerManager().SetTimer(
		EscapeMoveTimerHandle,
		this,
		&ALCDroneDelivery::UpdateEscapeMovement,
		EscapeUpdateInterval,
		true
	);
}

void ALCDroneDelivery::UpdateEscapeMovement()
{
	EscapeAlpha += EscapeUpdateInterval * EscapeSpeed / FVector::Dist(StartLocation, TargetLocation);

	SetActorLocation(FMath::Lerp(StartLocation, TargetLocation, EscapeAlpha));

	if (EscapeAlpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(EscapeMoveTimerHandle);
	}
}

void ALCDroneDelivery::Multicast_PlayDropEffect_Implementation()
{
	// TODO: 이펙트, 사운드 등 연출 삽입 가능
	UE_LOG(LogTemp, Warning, TEXT("[Drone] Drop effect played on all clients"));
}

void ALCDroneDelivery::Multicast_PlayDropExplosionEffect_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[Drone] Drop explosion played on all clients"));
	// Add explosion FX or sound here
}

void ALCDroneDelivery::Multicast_DropBoxDetach_Implementation()
{
	if (!HasAuthority()) // 서버는 이미 처리했으므로 클라만
	{
		DropBoxMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DropBoxMesh->SetSimulatePhysics(true);
		DropBoxMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DropBoxMesh->SetNotifyRigidBodyCollision(true);
	}
}

void ALCDroneDelivery::OnDropBoxHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !OtherComp)
	{
		return;
	}

	DropBoxMesh->OnComponentHit.RemoveAll(this);
	LOG_Frame_WARNING(TEXT("[Drone] OnDropBoxHit Called..."));

	// 1.5초 후 아이템 드랍 연출 및 제거
	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&ALCDroneDelivery::SpawnDroppedItems,
		1.5f,
		false
	);
}

void ALCDroneDelivery::SpawnDroppedItems()
{
	if (HasAuthority()==false)
	{
		return;
	}

	const FVector BaseLocation = DropBoxMesh->GetComponentLocation();
	LOG_Frame_WARNING(TEXT("[Drone] DropBox hit ground! Spawning items..."));

	for (const FItemDropData& Drop : ItemsToDrop)
	{
		// ItemID로 데이터 테이블에서 RowName 찾기(스포너가 행이름 기반 탐색이라..)
		FName ItemRowName = FindItemRowNameByID(Drop.ItemID);
		if (ItemRowName.IsNone())
		{
			LOG_Frame_WARNING(TEXT("[Drone] ItemRowName not found for ItemID: %d"), Drop.ItemID);
			continue;
		}

		LOG_Frame_WARNING(TEXT("[Drone] Spawning %d x %s (ID: %d)"), Drop.Count, *ItemRowName.ToString(), Drop.ItemID);

		for (int32 i = 0; i < Drop.Count; ++i)
		{
			FVector Offset = FVector(
				FMath::FRandRange(-50.f, 50.f),
				FMath::FRandRange(-50.f, 50.f),
				30.f
			);

			const FVector SpawnLocation = BaseLocation + Offset;

			AItemBase* SpawnedItem = ItemSpawner->CreateItem(ItemRowName, SpawnLocation);
		}
	}

	Multicast_PlayDropExplosionEffect();
	Destroy();
}


FName ALCDroneDelivery::FindItemRowNameByID(int32 ItemID)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (ULCGameInstanceSubsystem* GameSubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				UDataTable* ItemDataTable = GameSubsystem->GetItemDataTable();
				if (!ItemDataTable)
				{
					LOG_Frame_WARNING(TEXT("[FindItemRowNameByID] ItemDataTable is null"));
					return NAME_None;
				}

				// ⭐ 데이터 테이블에서 ItemID로 검색
				TArray<FName> RowNames = ItemDataTable->GetRowNames();
				for (const FName& RowName : RowNames)
				{
					FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("FindItemRowNameByID"));
					if (ItemData && ItemData->ItemID == ItemID)
					{
						return RowName;
					}
				}

				LOG_Frame_WARNING(TEXT("[FindItemRowNameByID] No RowName found for ItemID: %d"), ItemID);
				return NAME_None;
			}
		}
	}

	LOG_Frame_WARNING(TEXT("[FindItemRowNameByID] Failed to get GameSubsystem"));
	return NAME_None;
}

void ALCDroneDelivery::InitializeLights()
{
	DroneLights.Empty();

	GetComponents<UPointLightComponent>(DroneLights);
}

void ALCDroneDelivery::ToggleLights()
{
	for (UPointLightComponent* Light : DroneLights)
	{
		if (Light)
		{
			Light->ToggleVisibility();
		}
	}
}
