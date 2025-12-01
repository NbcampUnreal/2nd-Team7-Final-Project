#include "Framework/Manager/LCCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#include "Character/BasePlayerState.h"
#include "Character/BaseCharacter.h"

#include "Actor/LCGateActor.h"

#include "AI/MonsterSpawnComponent.h"
#include "AIController.h"

#include "Item/ItemBase.h"
#include "Item/ItemSpawner.h"
#include "DataTable/ItemDataRow.h"
#include "DataTable/MonsterDataTable.h"

#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameInstance.h"

#include "Components/PostProcessComponent.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"


DEFINE_LOG_CATEGORY(LogCheat);

void ULCCheatManager::InitCheatManager()
{
	Super::InitCheatManager();
}

void ULCCheatManager::GiveAllResources()
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance()))
		{
			if (ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				const UDataTable* ItemTable = Subsystem->GetItemDataTable();
				if (ItemTable == nullptr)
				{
					UE_LOG(LogCheat, Warning, TEXT("ItemDataTable이 비어 있습니다."));
					return;
				}

				const FString ContextString(TEXT("Cheat: GiveAllResources"));
				TArray<FItemDataRow*> AllRows;
				ItemTable->GetAllRows(ContextString, AllRows);

				for (const FItemDataRow* Row : AllRows)
				{
					if (Row && Row->bIsResourceItem)
					{
						UE_LOG(LogCheat, Warning, TEXT("[치트] 자원 아이템 지급: %s"), *Row->ItemName.ToString());

						// 인벤토리에 추가하는 로직을 여기에 구현
						// 예: InventoryComponent->AddItem(Row->ItemID, 1);
					}
				}
			}
		}
	}
}

void ULCCheatManager::Heal()
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (ABasePlayerState* PS = Cast<ABasePlayerState>(PC->PlayerState))
		{
			PS->CurrentHP = PS->InitialStats.MaxHP;
			UE_LOG(LogCheat, Warning, TEXT("[치트] 체력 완전 회복"));
			PS->UpdateHPUI();
		}
		else
		{
			UE_LOG(LogCheat, Warning, TEXT("PlayerState가 BasePlayerState가 아님"));
		}
	}
}

void ULCCheatManager::DamageSelf(int32 Amount)
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (ABasePlayerState* PS = Cast<ABasePlayerState>(PC->PlayerState))
		{
			PS->ApplyDamage(Amount);
			const float CurrentHP = PS->CurrentHP;
			if (CurrentHP <= 0)
			{
				if (ABaseCharacter* Char = Cast<ABaseCharacter>(PC->GetPawn()))
				{
					Char->HandlePlayerDeath();
				}
			}
			UE_LOG(LogCheat, Warning, TEXT("[치트] 셀프 데미지 %d 가함! 남은 체력 : %f"), Amount, CurrentHP);
		}
		else
		{
			UE_LOG(LogCheat, Warning, TEXT("PlayerState가 BasePlayerState가 아님"));
		}
	}
}

void ULCCheatManager::SetPlayerSpeed(float NewSpeed)
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (ABasePlayerState* PS = Cast<ABasePlayerState>(PC->PlayerState))
		{
			PS->SetPlayerMovementSetting(NewSpeed * 0.5f, NewSpeed * 0.5f, NewSpeed, NewSpeed, NewSpeed * 2);
			UE_LOG(LogCheat, Warning, TEXT("[치트] 이동 속도 설정: %.1f"), NewSpeed);
		}
		else
		{
			UE_LOG(LogCheat, Warning, TEXT("BaseCharacter가 아닙니다."));
			return;
		}
	}
}

void ULCCheatManager::AddGold(int32 Amount)
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance()))
		{
			if (ULCGameManager* LCGM = GI->GetSubsystem<ULCGameManager>())
			{
				LCGM->UpdateGold(FString::Printf(TEXT("치트매니저를 활용해 골드 추가!")), Amount);
			}
		}
	}
}

void ULCCheatManager::KillAllEnemies()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	int32 KilledCount = 0;

	for (TActorIterator<AAIController> It(World); It; ++It)
	{
		if (APawn* ControlledPawn = It->GetPawn())
		{
			ControlledPawn->Destroy();
			++KilledCount;
		}
	}

	UE_LOG(LogCheat, Warning, TEXT("[치트] AI 제거 완료: %d마리 제거됨"), KilledCount);
}

void ULCCheatManager::StopSpawning()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("World가 유효하지 않습니다."));
		return;
	}

	int32 FoundCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor) == false)
		{
			continue;
		}

		TArray<UMonsterSpawnComponent*> SpawnComponents;
		Actor->GetComponents(SpawnComponents);

		for (UMonsterSpawnComponent* Comp : SpawnComponents)
		{
			if (IsValid(Comp))
			{
				Comp->StopSpawning();
				FoundCount++;
			}
		}
	}

	UE_LOG(LogCheat, Warning, TEXT("[치트] 스폰 중지됨 - %d개 MonsterSpawnComponent 처리됨"), FoundCount);
}

void ULCCheatManager::StartSpawning()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("World가 유효하지 않습니다."));
		return;
	}

	int32 FoundCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor) == false)
		{
			continue;
		}

		TArray<UMonsterSpawnComponent*> SpawnComponents;
		Actor->GetComponents(SpawnComponents);

		for (UMonsterSpawnComponent* Comp : SpawnComponents)
		{
			if (IsValid(Comp))
			{
				Comp->StartSpawning();
				FoundCount++;
			}
		}
	}

	UE_LOG(LogCheat, Warning, TEXT("[치트] 스폰 중지됨 - %d개 MonsterSpawnComponent 처리됨"), FoundCount);
}

void ULCCheatManager::SpawnItem(FName ItemRowName)
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		APawn* PlayerPawn = PC->GetPawn();
		if (IsValid(PlayerPawn) == false)
		{
			UE_LOG(LogCheat, Warning, TEXT("플레이어가 존재하지 않음"));
			return;
		}

		if (ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance()))
		{
			if (ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				const UDataTable* ItemTable = Subsystem->GetItemDataTable();
				if (ItemTable == nullptr)
				{
					UE_LOG(LogCheat, Warning, TEXT("ItemDataTable이 존재하지 않습니다."));
					return;
				}

				const FItemDataRow* Row = ItemTable->FindRow<FItemDataRow>(ItemRowName, TEXT("Cheat SpawnItem"));
				if (Row == nullptr || Row->ItemActorClass == nullptr)
				{
					UE_LOG(LogCheat, Warning, TEXT("잘못된 ItemRow: %s"), *ItemRowName.ToString());
					return;
				}

				// 스폰 위치
				FVector SpawnLoc = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 100.f;

				// 치트 전용 임시 스포너 생성
				FActorSpawnParameters Params;
				Params.Owner = PC;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AItemSpawner* TempSpawner = PC->GetWorld()->SpawnActor<AItemSpawner>(AItemSpawner::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
				if (TempSpawner == nullptr)
				{
					UE_LOG(LogCheat, Warning, TEXT("임시 스포너 생성 실패"));
					return;
				}

				// 아이템 설정 후 즉시 스폰
				FSpawnableItemInfo Info;
				Info.ItemClass = Row->ItemActorClass;
				Info.ItemRowName = ItemRowName;
				Info.MinQuantity = 1;
				Info.MaxQuantity = 1;
				Info.MinDurability = 100.f;
				Info.MaxDurability = 100.f;
				Info.SpawnProbability = 1.0f;

				TempSpawner->SpawnableItems.Add(Info);
				TempSpawner->SpawnItems();

				UE_LOG(LogCheat, Warning, TEXT("[치트] 아이템 생성 완료: %s"), *ItemRowName.ToString());

				// 필요 시 스포너 제거
				TempSpawner->Destroy();
			}
		}
	}
}

void ULCCheatManager::SpawnItemByRowHandle(FDataTableRowHandle ItemRowHandle)
{
	if (ItemRowHandle.DataTable == nullptr || ItemRowHandle.RowName.IsValid() == false)
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] ItemRowHandle이 유효하지 않습니다."));
		return;
	}

	// 기존 SpawnItem 함수 재사용
	SpawnItem(ItemRowHandle.RowName);
}

void ULCCheatManager::ToggleGodMode()
{
	bGodMode = !bGodMode;

	FString Status = bGodMode ? TEXT("ON") : TEXT("OFF");
	UE_LOG(LogCheat, Warning, TEXT("[치트] GodMode 전환됨: %s"), *Status);

	if (ABasePlayerState* PlayerState = GetPlayerController()->GetPlayerState<ABasePlayerState>())
	{
		bGodMode ? PlayerState->bInfiniteHP = true : PlayerState->bInfiniteHP = false;
	}
}

void ULCCheatManager::RunningMan()
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (ABasePlayerState* PS = Cast<ABasePlayerState>(PC->PlayerState))
		{
			PS->InitialStats.StaminaDrainRate = 0.0f;
			UE_LOG(LogCheat, Warning, TEXT("[치트] 무한 달리기"));
		}
		else
		{
			UE_LOG(LogCheat, Warning, TEXT("PlayerState가 BasePlayerState가 아님"));
		}
	}
}

void ULCCheatManager::PrintLocation()
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			const FVector Loc = Pawn->GetActorLocation();
			const FString Message = FString::Printf(TEXT("[치트] 현재 위치: X=%.1f Y=%.1f Z=%.1f"), Loc.X, Loc.Y, Loc.Z);

			// 로그 출력
			UE_LOG(LogCheat, Warning, TEXT("%s"), *Message);

			// 화면에 표시
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, Message);
			}
		}
		else
		{
			UE_LOG(LogCheat, Warning, TEXT("Pawn이 존재하지 않습니다."));
		}
	}
}

void ULCCheatManager::TravelToMap(FName MapName)
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (ULCGameInstanceSubsystem* Subsystem = PC->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			FString MapNameStr = MapName.ToString(); // 수정 포인트
			const int32 ID = FCrc::StrCrc32(*MapNameStr);
			Subsystem->ChangeLevelByMapID(ID);
			UE_LOG(LogCheat, Warning, TEXT("[치트] 맵 이동: %s"), *MapName.ToString());
		}
		else
		{
			UE_LOG(LogCheat, Warning, TEXT("GameInstanceSubsystem이 존재하지 않습니다."));
		}
	}
}

void ULCCheatManager::ShowPlayerFrameworkInfo()
{
	APlayerController* PC = GetOuterAPlayerController();
	if (PC == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] PlayerController 없음"));
		return;
	}
	ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance());
	if (GI == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] GameInstance 없음"));
		return;
	}
	ULCGameManager* LCGM = GI->GetSubsystem<ULCGameManager>();
	if (LCGM == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] LCGameManager 없음"));
		return;
	}

	// PlayerState
	ABasePlayerState* PS = PC->GetPlayerState<ABasePlayerState>();
	// Pawn
	APawn* Pawn = PC->GetPawn();

	// 문자열 구성
	FString InfoString = FString::Printf(TEXT("▶ PlayerFramework Info"));
	InfoString += FString::Printf(TEXT("\n - Controller : %s"), *PC->GetName());

	if (Pawn)
	{
		InfoString += FString::Printf(TEXT("\n - Pawn       : %s"), *Pawn->GetName());
	}
	else
	{
		InfoString += TEXT("\n - Pawn       : 없음");
	}

	if (PS)
	{
		InfoString += FString::Printf(TEXT("\n - PlayerState: %s"), *PS->GetName());
		InfoString += FString::Printf(TEXT("\n   HP         : %.1f"), PS->GetHP());
		InfoString += FString::Printf(TEXT("\n   Stamina    : %.1f"), PS->GetStamina());
		InfoString += FString::Printf(TEXT("\n   Gold       : %d"), LCGM->GetGold());
		InfoString += FString::Printf(TEXT("\n   Exp        : %d"), PS->GetTotalExp());
	}
	else
	{
		InfoString += TEXT("\n - PlayerState: 없음");
	}

	// 로그 출력
	UE_LOG(LogCheat, Warning, TEXT("[치트] ===== 현재 플레이어 프레임워크 정보 ====="));
	UE_LOG(LogCheat, Warning, TEXT("%s"), *InfoString);

	// 화면 출력
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, InfoString);
	}
}

void ULCCheatManager::PrintAcquiredItems()
{
	APlayerController* PC = GetOuterAPlayerController();
	if (PC == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("PlayerController가 없습니다."));
		return;
	}

	ABasePlayerState* PS = PC->GetPlayerState<ABasePlayerState>();
	if (PS == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("PlayerState가 BasePlayerState가 아닙니다."));
		return;
	}

	ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance());
	if (GI == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("GameInstance가 유효하지 않습니다."));
		return;
	}

	ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
	if (Subsystem == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("GameInstanceSubsystem이 유효하지 않습니다."));
		return;
	}

	const UDataTable* ItemTable = Subsystem->GetItemDataTable();
	if (ItemTable == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("ItemDataTable이 없습니다."));
		return;
	}

	const TArray<int32>& ItemIDs = PS->AquiredItemIDs;
	if (ItemIDs.Num() == 0)
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] 획득한 아이템이 없습니다."));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("획득한 아이템 없음"));
		return;
	}

	FString Summary = TEXT("▶ 획득한 아이템 목록");

	for (int32 ItemID : ItemIDs)
	{
		FName RowName = *FString::Printf(TEXT("Item_%d"), ItemID);
		const FItemDataRow* Row = ItemTable->FindRow<FItemDataRow>(RowName, TEXT("Cheat: PrintAcquiredItems"));

		if (Row)
		{
			FString Line = FString::Printf(TEXT(" - %s (%s)"), *Row->ItemName.ToString(), Row->bIsResourceItem ? TEXT("자원") : TEXT("일반"));
			UE_LOG(LogCheat, Warning, TEXT("%s"), *Line);
			Summary += LINE_TERMINATOR + Line;
		}
		else
		{
			FString Line = FString::Printf(TEXT(" - ItemID %d: Row를 찾을 수 없습니다."), ItemID);
			UE_LOG(LogCheat, Warning, TEXT("%s"), *Line);
			Summary += LINE_TERMINATOR + Line;
		}
	}

	// 화면 출력
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, Summary);
	}
}

void ULCCheatManager::KillAllOthers()
{
	APlayerController* MyPC = GetOuterAPlayerController();
	if (MyPC == nullptr || MyPC->HasAuthority() == false)
	{
		return;
	}

	UWorld* World = MyPC->GetWorld();
	if (World == nullptr)
	{
		return;

	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* OtherPC = It->Get();
		if (OtherPC && OtherPC != MyPC)
		{
			APawn* Pawn = OtherPC->GetPawn();
			if (Pawn)
			{
				// 최대 체력만큼 데미지를 줘서 즉시 사망 유도
				UGameplayStatics::ApplyDamage(
					Pawn,
					10000.f, // 충분히 큰 값
					MyPC,
					nullptr,
					UDamageType::StaticClass()
				);
			}
		}
	}
}

void ULCCheatManager::TeleportAllPlayers(float X, float Y, float Z, float Radius)
{
	FVector CenterLocation(X, Y, Z);
	TeleportAllPlayers_Internal(CenterLocation, Radius);
}

void ULCCheatManager::TeleportAllPlayers_Internal(FVector CenterLocation, float OffsetRadius)
{
	APlayerController* MyPC = GetOuterAPlayerController();
	if (MyPC == nullptr || MyPC->HasAuthority() == false)
	{
		return;
	}

	UWorld* World = MyPC->GetWorld();
	if (World == nullptr)
	{
		return;

	}

	TArray<APlayerController*> AllPCs;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			AllPCs.Add(PC);
		}
	}

	const int32 PlayerCount = AllPCs.Num();
	const float AngleStep = 360.0f / FMath::Max(PlayerCount, 1);

	for (int32 i = 0; i < PlayerCount; ++i)
	{
		APlayerController* PC = AllPCs[i];
		if (PC == nullptr)
		{
			continue;
		}

		APawn* Pawn = PC->GetPawn();
		if (Pawn == nullptr)
		{
			continue;
		}

		// 오프셋 위치 계산 (원형 배치)
		float AngleDeg = i * AngleStep;
		float Radian = FMath::DegreesToRadians(AngleDeg);
		FVector Offset(FMath::Cos(Radian) * OffsetRadius, FMath::Sin(Radian) * OffsetRadius, 0.f);

		FVector TargetLocation = CenterLocation + Offset;
		Pawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);

		UE_LOG(LogCheat, Warning, TEXT("[치트] %s 이동: X=%.1f Y=%.1f Z=%.1f"),
			*PC->GetName(), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
	}
}

void ULCCheatManager::Lumos(float ForcedValue)
{
	APlayerController* PC = GetOuterAPlayerController();
	if (PC == nullptr)
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (Pawn == nullptr)
	{
		return;
	}

	if (ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn))
	{
		if (Character->DisplayComponent == nullptr)
		{
			UE_LOG(LogCheat, Warning, TEXT("[치트] CustomPostProcessComponent가 없음"));
			return;
		}

		if (FMath::IsNearlyZero(ForcedValue))
		{
			// 저장된 밝기 값 불러오기
			UWorld* World = Character->GetWorld();
			const float SavedBrightness = ULCLocalPlayerSaveGame::LoadBrightness(World);
			Character->SetBrightness(SavedBrightness);

			UE_LOG(LogCheat, Warning, TEXT("[치트] 밝기 원래대로 복구 (Saved = %.2f)"), SavedBrightness);
		}
		else
		{
			float Clamped = FMath::Clamp(ForcedValue, -5.0f, 20.0f);

			Character->SetBrightness(Clamped);

			UE_LOG(LogCheat, Warning, TEXT("[치트] 밝기 강제 설정: %.2f"), Clamped);
		}
	}
}

void ULCCheatManager::EscapeToBaseCamp()
{
	APlayerController* PC = GetOuterAPlayerController();
	if (PC == nullptr || PC->HasAuthority() == false)
	{
		return;
	}

	UWorld* World = PC->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 가장 가까운 ALCGateActor 중 ToBaseCamp 타입 찾기
	ALCGateActor* TargetGate = nullptr;
	float MinDistSq = FLT_MAX;

	for (TActorIterator<ALCGateActor> It(World); It; ++It)
	{
		ALCGateActor* Gate = *It;
		if (IsValid(Gate) == false)
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Gate->GetActorLocation(), PC->GetPawn()->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			TargetGate = Gate;
			MinDistSq = DistSq;
		}
	}

	if (IsValid(TargetGate))
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] 게이트로 탈출 시도: %s"), *TargetGate->GetName());
		TargetGate->ReturnToBaseCamp(PC);
	}
	else
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] ToBaseCamp 게이트를 찾을 수 없음."));
	}
}

void ULCCheatManager::PPP()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const bool bIsPaused = UGameplayStatics::IsGamePaused(World);
	UGameplayStatics::SetGamePaused(World, !bIsPaused);

	if (GEngine)
	{
		FString Status = bIsPaused ? TEXT("▶ 게임 재개") : TEXT("⏸ 게임 일시정지");
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, FString::Printf(TEXT("[치트] %s"), *Status));
	}

	UE_LOG(LogCheat, Warning, TEXT("[치트] 게임 일시정지 토글 → 현재 상태: %s"), bIsPaused ? TEXT("재생 중") : TEXT("정지됨"));
}

void ULCCheatManager::SpawnEnemy(FName EnemyRowName)
{
	APlayerController* PC = GetOuterAPlayerController();
	if (PC == nullptr || PC->HasAuthority() == false)
	{
		return;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (IsValid(PlayerPawn) == false)
	{
		UE_LOG(LogCheat, Warning, TEXT("플레이어 Pawn이 없습니다."));
		return;
	}

	ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance());
	if (GI == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("GameInstance가 유효하지 않습니다."));
		return;
	}

	ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
	if (Subsystem == nullptr)
	{
		UE_LOG(LogCheat, Warning, TEXT("GameInstanceSubsystem이 유효하지 않습니다."));
		return;
	}

	const FString Context = TEXT("SpawnEnemyCheat");

	const UDataTable* MonsterTable = Subsystem->GetMonsterDataTable();

	const FMonsterDataTable* Row = nullptr;

	if (Row == nullptr && MonsterTable)
	{
		Row = MonsterTable->FindRow<FMonsterDataTable>(EnemyRowName, Context);
	}

	if (!Row || !Row->MonsterActor)
	{
		return;
	}

	// 스폰
	const FVector SpawnLocation = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 300.f;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters Params;
	Params.Owner = PC;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedEnemy = PC->GetWorld()->SpawnActor<AActor>(Row->MonsterActor, SpawnLocation, SpawnRotation, Params);
	if (IsValid(SpawnedEnemy))
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] 적 스폰 완료: %s (%s)"), *Row->MonsterName.ToString(), *EnemyRowName.ToString());
	}
	else
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] 적 스폰 실패"));
	}
}

void ULCCheatManager::Superman(float NewSpeed)
{
	ToggleGodMode(); // GodMode 활성화
	SetPlayerSpeed(NewSpeed); // 속도 설정
	RunningMan(); // 무한 달리기 활성화
}

void ULCCheatManager::Kit1()
{
	SpawnItem(FName(TEXT("FlashV1"))); // Kit1 아이템 스폰
	SpawnItem(FName(TEXT("FlashV2"))); // Kit1 아이템 스폰
	SpawnItem(FName(TEXT("FlashV3"))); // Kit1 아이템 스폰
	SpawnItem(FName(TEXT("Rifle01"))); // Kit1 아이템 스폰
	SpawnItem(FName(TEXT("Rifle01_S"))); // Kit1 아이템 스폰
	SpawnItem(FName(TEXT("Shotgun"))); // Kit1 아이템 스폰
	SpawnItem(FName(TEXT("Shotgun_S"))); // Kit1 아이템 스폰
}

void ULCCheatManager::Kit2()
{
	SpawnItem(FName(TEXT("Scanner")));
	SpawnItem(FName(TEXT("Scanner")));
	SpawnItem(FName(TEXT("WalkieTalkie")));
	SpawnItem(FName(TEXT("WalkieTalkie")));
	SpawnItem(FName(TEXT("DRN01")));
	SpawnItem(FName(TEXT("DRN02")));
	SpawnItem(FName(TEXT("DRN03")));
	SpawnItem(FName(TEXT("DRN04")));
	SpawnItem(FName(TEXT("DRN05")));
}