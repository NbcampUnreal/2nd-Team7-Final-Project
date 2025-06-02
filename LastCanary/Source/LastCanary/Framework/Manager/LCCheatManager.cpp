#include "Framework/Manager/LCCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Character/BasePlayerState.h"
#include "Character/BaseCharacter.h"

#include "AI/BaseAIController.h"
#include "AIController.h"

#include "Item/ItemBase.h"
#include "DataTable/ItemDataRow.h"

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
			PS->SetPlayerMovementSetting(NewSpeed*0.5f, NewSpeed * 0.5f, NewSpeed, NewSpeed, NewSpeed * 2);
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
		if (ABasePlayerState* PS = Cast<ABasePlayerState>(PC->PlayerState))
		{
			PS->AddTotalGold(Amount);
			UE_LOG(LogCheat, Warning, TEXT("[치트] 골드 지급: %d"), Amount);
		}
	}
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

void ULCCheatManager::SpawnItem(FName ItemRowName)
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
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

				const FString ContextString(TEXT("Cheat: SpawnItem"));
				const FItemDataRow* Row = ItemTable->FindRow<FItemDataRow>(ItemRowName, ContextString);

				if (Row && Row->ItemActorClass)
				{
					APawn* PlayerPawn = PC->GetPawn();
					FVector SpawnLoc = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 100.f;

					FActorSpawnParameters Params;
					Params.Owner = PlayerPawn;

					AItemBase* NewItem = PC->GetWorld()->SpawnActor<AItemBase>(Row->ItemActorClass, SpawnLoc, FRotator::ZeroRotator, Params);
					if (NewItem)
					{
						// NewItem->ApplyItemData(ItemRowName); // 내부에 정의된 데이터 적용 함수
						UE_LOG(LogCheat, Warning, TEXT("[치트] 아이템 생성: %s"), *ItemRowName.ToString());
					}
					else
					{
						UE_LOG(LogCheat, Warning, TEXT("아이템 생성 실패"));
					}
				}
				else
				{
					UE_LOG(LogCheat, Warning, TEXT("아이템 정보가 존재하지 않거나 ItemClass가 누락됨: %s"), *ItemRowName.ToString());
				}
			}
		}
	}
}

void ULCCheatManager::ToggleGodMode()
{
	bGodMode = !bGodMode;

	FString Status = bGodMode ? TEXT("ON") : TEXT("OFF");
	UE_LOG(LogCheat, Warning, TEXT("[치트] GodMode 전환됨: %s"), *Status);

	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (ACharacter* Character = PC->GetCharacter())
		{
			Character->SetCanBeDamaged(!bGodMode);
		}
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
		InfoString += FString::Printf(TEXT("\n   Gold       : %d"), PS->GetTotalGold());
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