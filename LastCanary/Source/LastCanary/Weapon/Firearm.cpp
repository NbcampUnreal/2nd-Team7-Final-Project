// Copyright Epic Games, Inc. All Rights Reserved.

#include "Firearm.h"
#include "Weapon/Bullet.h"
#include "Weapon/Magazine.h"
#include "Weapon/Suppressor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
//#include "Core/HexboundGameInstance.h"
//#include "Character/MyPlayerController.h"
//#include "Character/MyCharacter.h"
#include "Blueprint/UserWidget.h"
//#include "UI/InGame.h"
//#include "Managers/UIManager.h"

AFirearm::AFirearm()
{
	PrimaryActorTick.bCanEverTick = false;

	BulletMesh = nullptr;										// 프로젝타일에 전달할 탄환 메쉬
	Bullet = nullptr;											// 사용할 프로젝타일 클래스
	Suppressor = nullptr;										// 소음기 클래스
	CrosshairTexture = nullptr;									// 조준십자 텍스쳐
	BulletSpeed = 7000.0f;										// 프로젝타일에 전달할 탄 스피드
	MaxAmmo = 200;												// 최대 탄 소지량
	CurrentAmmo = 0;											// 현재 탄 소지량
	MaxReloadedAmmo = 0;										// 최대 장전 탄수
	ReloadedAmmo = 0;											// 현재 장전 탄수
	ReloadTime = 1.0f;											// 재장전 소요시간
	OriginalAccuracy = 1.0f;									// 명중률 (0.1 ~ 1.0)
	MaxSpreadAngle = 10.0f;										// 탄퍼짐 최대각도
	FinalAccuracy = MaxSpreadAngle * (1 - OriginalAccuracy);	// 보정된 명중률
	bIsLoadingComplete = true;									// 재장전 중 여부
	bIsMagazineAttached = false;								// 탄창 결합 여부
	bIsSuppressorInstalled = false;								// 소음기 결합 여부
	//UIManager = nullptr;
	bCanReload = false;
}

void AFirearm::Attack()
{
	if (ReloadedAmmo > 0 && bIsLoadingComplete)
	{
		if (bIsCooltimeEnd)
		{
			if (AttackNiagara && !bIsSuppressorInstalled)
			{
				UNiagaraComponent* AttackEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(		// 격발 효과
					AttackNiagara,
					WeaponMesh,
					TEXT("MuzzleSocket"),
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget,
					true
				);
			}

			else if (AttackNiagara && bIsSuppressorInstalled)
			{
				UStaticMeshComponent* SuppressorMeshComponent = Suppressor->FindComponentByClass<UStaticMeshComponent>();
				UNiagaraComponent* AttackEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(		// 소음기 격발 효과
					SuppressorFireNiagara,
					SuppressorMeshComponent,
					TEXT("SuppressorMuzzleSocket"),
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget,
					true
				);
				
				this->AttackSound = SuppressorSound;
			}

			Fire();

			if (FireAnim)
			{
				WeaponMesh->PlayAnimation(FireAnim, false);	// 격발 애니메이션
			}

			ReloadedAmmo--;
			UpdateWeaponUI();
		}

		Super::Attack();
	}

	if (ReloadedAmmo <= 0 && bIsCooltimeEnd)
	{
		if (EmptyAmmoSound)
		{
			//UHexboundGameInstance* GameInstance = Cast<UHexboundGameInstance>(GetGameInstance());
			//float FinalVolume = 1.0f;
			//if(GameInstance) FinalVolume = GameInstance->SFXVolume;

			UGameplayStatics::PlaySoundAtLocation(GetWorld(), EmptyAmmoSound, GetActorLocation(), 1.0f);	// 빈총 격발음
		}
	}

}

void AFirearm::Fire()
{
	if (BulletClass)
	{
		FVector ControllerDirection;
		FVector FinalLocation;
		
		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
/*			if (AMyPlayerController* Controller = Cast<AMyPlayerController>(PlayerController))
			{
				FRotator ControlRotation = Controller->GetControlRotation();
				ControllerDirection = ControlRotation.Vector();

				FVector CameraLocation = Controller->PlayerCameraManager->GetCameraLocation();
				FinalLocation = CameraLocation + (ControllerDirection * 50);
				Controller->SetControlRotation(ControlRotation + FRotator(5.0f, 0.0f, 0.0f));
			}	*/		
		}

		FinalAccuracy = MaxSpreadAngle * (1 - OriginalAccuracy);

		FVector SpreadDirection = FMath::VRandCone(ControllerDirection, FMath::DegreesToRadians(FinalAccuracy));
		FRotator FinalRotation = SpreadDirection.Rotation();

		FinalLocation = FVector();

		if (BulletPool.Num() > 0)
		{
			Bullet = BulletPool.Pop();
			if (Bullet && Bullet->IsValidLowLevel())
			{
				Bullet->ActivateBullet(FinalLocation, FinalRotation, BulletSpeed);
			}
		}

		else
		{		
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			Bullet = GetWorld()->SpawnActor<ABullet>(BulletClass, FinalLocation, FinalRotation, SpawnParams);
	
			if (!Bullet)
			{
				UE_LOG(LogTemp, Error, TEXT("Bullet spawn failed!"));
				return;
			}
		}

		if (Bullet)
		{
			Bullet->SetFirearm(this);	
			Bullet->SetBulletMesh(BulletMesh);	
			Bullet->SetBulletSpeed(BulletSpeed);		
		}
	}
}

void AFirearm::BeginPlay()
{
	Super::BeginPlay();

	OriginalAttackSound = AttackSound;	// 소음기 탈부착 대비 사운드 저장

	//if (UHexboundGameInstance* GameInstance = Cast<UHexboundGameInstance>(GetGameInstance()))
	//{
	//	UIManager = GameInstance->GetSubsystem<UUIManager>();
	//	if (UIManager)
	//	{
	//		UpdateWeaponUI(); // 초기 Ammo 상태 반영
	//	}
	//}

}

void AFirearm::Reload()
{
	if (bIsLoadingComplete && bIsMagazineAttached)
	{
		if (CurrentAmmo <= 0)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadFailSound, GetActorLocation());
			return;
		}
		
		if (ReloadSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());	// 재장전 효과음
		}
		bIsLoadingComplete = false;
		GetWorldTimerManager().SetTimer(														// 재장전 시간 소요 후 재장전 처리
			ReloadTimerHandle,
			[this]() {
				int32 ReloadValue = FMath::Min(MaxReloadedAmmo - ReloadedAmmo, CurrentAmmo);
				CurrentAmmo = FMath::Max(CurrentAmmo - ReloadValue, 0);
				ReloadedAmmo = MaxReloadedAmmo;
				bIsLoadingComplete = true;
				UpdateWeaponUI();
			},
			ReloadTime,
			false
		);

	}
}

void AFirearm::SetReloadCondition()
{
	if (bIsLoadingComplete && bIsMagazineAttached && CurrentAmmo > 0)
	{
		bCanReload = true;
	}
	else
	{
		bCanReload = false;
	}
}
bool AFirearm::GetReloadCondition() const
{
	return bCanReload;
}

int32 AFirearm::GetCurrentAmmoValue() const
{
	return CurrentAmmo;
}

int32 AFirearm::GetMaxReloadedAmmoValue() const
{
	return MaxReloadedAmmo;
}

int32 AFirearm::GetReloadedAmmoValue() const
{
	return ReloadedAmmo;
}

float AFirearm::GetFinalAccuracty() const
{
	return 1 - FinalAccuracy;
}

void AFirearm::AddAmmo(int32 AmmoToAdd)
{
	CurrentAmmo = FMath::Clamp(CurrentAmmo + AmmoToAdd, 0, MaxAmmo);
	UpdateWeaponUI();
}

void AFirearm::EquipParts(AParts* Parts) // 파츠를 파츠에 저장된 소켓명에 따라 먼저 소켓에 부착 후 클래스 판별하여 기능 적용
{
	if (Parts)
	{
		DetachParts(Parts->SocketNameForAttach);
		Parts->AttachToComponent(WeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Parts->SocketNameForAttach);

		if (Parts->IsA(AMagazine::StaticClass()))
		{
			AMagazine* Magazine = Cast<AMagazine>(Parts);
			MaxReloadedAmmo = Magazine->GetMagazineCapacity();
			ReloadedAmmo = MaxReloadedAmmo;
			bIsMagazineAttached = true;
			UpdateWeaponUI();
		}

		if (Parts->IsA(ASuppressor::StaticClass()))
		{
			Suppressor = Cast<ASuppressor>(Parts);
			bIsSuppressorInstalled = true;
		}
	}
}

void AFirearm::ReturnBulletToPool(ABullet* UsedBullet) // 프로젝타일 액터 재사용 함수
{
	BulletPool.Add(UsedBullet);
}

void AFirearm::DetachParts(FName SocketName) // 파츠 결합 전 기존 파트 분리
{
	TArray<AActor*> AttachedActors;
	this->GetAttachedActors(AttachedActors);

	for (AActor* Actor : AttachedActors)
	{
		if (Actor && Actor->GetAttachParentSocketName() == SocketName)
		{
			Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			Actor->AddActorLocalOffset(FVector(0.0f, 0.0f, -10.0f));	
			break;
		}
	}	

	if (SocketName == FName(TEXT("MagazineSocket")))
	{
		MaxReloadedAmmo = 0;
		bIsMagazineAttached = false;
	}

	if (SocketName == FName(TEXT("MuzzleSocket")))
	{
		Suppressor = nullptr;
		bIsSuppressorInstalled = false;
		this->AttackSound = OriginalAttackSound;
	}
}
//
//void AFirearm::UpdateWeaponUI()
//{
//	Super::UpdateWeaponUI();
//	if (UIManager)
//	{
//		UUserWidget* InGameWidgetInstance = UIManager->WidgetInstances.FindRef(EHUDState::InGameBase);
//		if (UInGame* InGameWidget = Cast<UInGame>(InGameWidgetInstance))
//		{
//			InGameWidget->UpdateAmmo(ReloadedAmmo, CurrentAmmo);
//		}
//		else
//		{
//			UE_LOG(LogTemp, Warning, TEXT("InGame widget not found in UIManager"));
//		}
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("UIManager is null in AFirearm"));
//	}
//}