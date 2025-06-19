#include "Framework/Manager/LCCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Character/BasePlayerState.h"
#include "Character/BaseCharacter.h"


#include "AI/BaseAIController.h"
#include "AI/MonsterSpawnComponent.h"
#include "AIController.h"

#include "Item/ItemBase.h"
#include "Item/ItemSpawner.h"
#include "DataTable/ItemDataRow.h"

#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameInstance.h"

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
				LCGM->AddGold(Amount);
			}
		}
	}

	//if (APlayerController* PC = GetOuterAPlayerController())
	//{
	//	if (ABasePlayerState* PS = Cast<ABasePlayerState>(PC->PlayerState))
	//	{
	//		PS->AddTotalGold(Amount);
	//		UE_LOG(LogCheat, Warning, TEXT("[치트] 골드 지급: %d"), Amount);
	//	}
	//}
}

void ULCCheatManager::KillAllEnemies()
{
	UWorld* World = GetWorld();
	if (!World) return;

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
	if (!World)
	{
		UE_LOG(LogCheat, Warning, TEXT("World가 유효하지 않습니다."));
		return;
	}

	int32 FoundCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor))
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
	if (!World)
	{
		UE_LOG(LogCheat, Warning, TEXT("World가 유효하지 않습니다."));
		return;
	}

	int32 FoundCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor))
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
		if (!IsValid(PlayerPawn))
		{
			UE_LOG(LogCheat, Warning, TEXT("플레이어가 존재하지 않음"));
			return;
		}

		if (ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance()))
		{
			if (ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				const UDataTable* ItemTable = Subsystem->GetItemDataTable();
				if (!ItemTable)
				{
					UE_LOG(LogCheat, Warning, TEXT("ItemDataTable이 존재하지 않습니다."));
					return;
				}

				const FItemDataRow* Row = ItemTable->FindRow<FItemDataRow>(ItemRowName, TEXT("Cheat SpawnItem"));
				if (!Row || !Row->ItemActorClass)
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
				if (!TempSpawner)
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
	if (!ItemRowHandle.DataTable || !ItemRowHandle.RowName.IsValid())
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
			UE_LOG(LogCheat, Warning, TEXT("[치트] 현재 위치: X=%.1f Y=%.1f Z=%.1f"), Loc.X, Loc.Y, Loc.Z);
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
	if (!PC)
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] PlayerController 없음"));
		return;
	}
	ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogCheat, Warning, TEXT("[치트] GameInstance 없음"));
		return;
	}
	ULCGameManager* LCGM = GI->GetSubsystem<ULCGameManager>();
	if (!LCGM)
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
	if (!PC)
	{
		UE_LOG(LogCheat, Warning, TEXT("PlayerController가 없습니다."));
		return;
	}

	ABasePlayerState* PS = PC->GetPlayerState<ABasePlayerState>();
	if (!PS)
	{
		UE_LOG(LogCheat, Warning, TEXT("PlayerState가 BasePlayerState가 아닙니다."));
		return;
	}

	ULCGameInstance* GI = Cast<ULCGameInstance>(PC->GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogCheat, Warning, TEXT("GameInstance가 유효하지 않습니다."));
		return;
	}

	ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogCheat, Warning, TEXT("GameInstanceSubsystem이 유효하지 않습니다."));
		return;
	}

	const UDataTable* ItemTable = Subsystem->GetItemDataTable();
	if (!ItemTable)
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
	if (!MyPC || !MyPC->HasAuthority()) return;

	UWorld* World = MyPC->GetWorld();
	if (!World) return;

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