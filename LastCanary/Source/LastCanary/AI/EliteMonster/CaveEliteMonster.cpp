#include "AI/EliteMonster/CaveEliteMonster.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SphereComponent.h"
#include "Item/EquipmentItem/FlashlightItem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/BaseAIController.h"
#include "AIController.h"
//#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ACaveEliteMonster::ACaveEliteMonster()
{
	PrimaryActorTick.bCanEverTick = false;

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetGenerateOverlapEvents(true);
		CapsuleComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		/*CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &ACaveEliteMonster::BeginOverlap);
		CapsuleComp->OnComponentEndOverlap.AddDynamic(this, &ACaveEliteMonster::EndOverlap);*/
	}

}

//void ACaveEliteMonster::BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//
//	if (!HasAuthority())
//	{
//		return;
//	}
//
//	/*if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(OtherComp))
//	{
//		if (SpotLight->IsVisible() && !bIsFrozen)
//		{
//			FreezeAI();
//		}
//	}*/
//	// StaticMeshComponent와 오버랩하는지 확인
//	if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(OtherComp))
//	{
//		if (AFlashlightItem* Flashlight = Cast<AFlashlightItem>(OtherActor))
//		{
//			if (Flashlight->bIsLightOn && !bIsFrozen)
//			{
//				FreezeAI();
//			}
//		}
//	}
//
//}
//void ACaveEliteMonster::EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if (!HasAuthority())
//	{
//		return;
//	}
//
//	// SphereComponent와 오버랩 종료인지 확인
//	if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(OtherComp))
//	{
//		// 손전등 아이템인지 확인
//		if (AFlashlightItem* Flashlight = Cast<AFlashlightItem>(OtherActor))
//		{
//			if (bIsFrozen)
//			{
//				UnfreezeAI();
//			}
//		}
//	}
//	/*if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(OtherComp))
//	{
//		if (bIsFrozen)
//		{
//			UnfreezeAI();
//		}
//	}*/
//}

void ACaveEliteMonster::FreezeAI()
{
	UE_LOG(LogTemp, Warning, TEXT("Cave Elite Monster now Freeze!!!!!!!!!"));

	if (bIsFrozen)
	{
		return;
	}
	bIsFrozen = true;

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->SetMovementMode(MOVE_None);//moveto 무시
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		
	}

	if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(GetController()))
	{
		BaseAIController->SetStop();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FreezeTimerHandle,
			this,
			&ACaveEliteMonster::UnfreezeAI,
			MaxFreezeTime,
			false
		);
	}
}
void ACaveEliteMonster::UnfreezeAI()
{
	if (!bIsFrozen)
	{
		return;
	}
	bIsFrozen = false;

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
	}

	if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(GetController()))
	{
		BaseAIController->SetPatrolling();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FreezeTimerHandle);
	}
}