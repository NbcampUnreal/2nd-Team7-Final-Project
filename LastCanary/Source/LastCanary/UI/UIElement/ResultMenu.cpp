#include "UI/UIElement/ResultMenu.h"
#include "UI/UIObject/RewardEntry.h"
#include "UI/UIObject/ResourceScoreEntry.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"

#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"

#include "LastCanary.h"

void UResultMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (AcceptButton)
	{
		AcceptButton->OnClicked.AddUniqueDynamic(this, &UResultMenu::OnAcceptClicked);
	}
	if (RankText)
	{
		RankText->SetRenderOpacity(0.0f);
	}
}

void UResultMenu::NativeDestruct()
{
	Super::NativeDestruct();
	if (AcceptButton)
	{
		AcceptButton->OnClicked.RemoveDynamic(this, &UResultMenu::OnAcceptClicked);
	}
}

void UResultMenu::SetRewardEntries(const TArray<FResultRewardEntry>& InEntries)
{
	CachedEntries = InEntries;
	CurrentEntryIndex = 0;
	CurrentResourceIndex = 0;
	bIsAddingResources = false;

	if (RewardScrollBox == nullptr)
	{
		return;
	}
	if (RewardEntryClass == nullptr)
	{
		return;
	}

	RewardScrollBox->ClearChildren();

	GetWorld()->GetTimerManager().SetTimer(
		EntryAddTimerHandle,
		this,
		&UResultMenu::AddNextEntry, // 이름 통일
		0.3f,
		true
	);
}

void UResultMenu::AddNextEntry()
{
	if (!RewardScrollBox)
	{
		GetWorld()->GetTimerManager().ClearTimer(EntryAddTimerHandle);
		return;
	}

	if (bIsAddingResources == false)
	{
		if (CachedEntries.IsValidIndex(CurrentEntryIndex))
		{
			const FResultRewardEntry& EntryData = CachedEntries[CurrentEntryIndex];
			URewardEntry* EntryWidget = CreateWidget<URewardEntry>(this, RewardEntryClass);
			if (EntryWidget)
			{
				EntryWidget->InitWithEntry(EntryData);
				RewardScrollBox->AddChild(EntryWidget);
			}

			CurrentEntryIndex++;
			return;
		}

		// 보상 항목이 끝난 후 → 자원 점수 단계로 전환
		bIsAddingResources = true;
		CurrentResourceIndex = 0;
		return;
	}

	// 자원 점수 항목 추가 단계
	if (CachedResourceDetails.IsValidIndex(CurrentResourceIndex))
	{
		if (ResourceScoreEntryClass)
		{
			const FResourceScoreInfo& Info = CachedResourceDetails[CurrentResourceIndex];

			UResourceScoreEntry* Entry = CreateWidget<UResourceScoreEntry>(this, ResourceScoreEntryClass);
			if (Entry)
			{
				const FString Breakdown = FString::Printf(
					TEXT("%d × %.2f × %d = %d"),
					Info.BaseScore,
					Info.Multiplier,
					Info.Amount,
					Info.TotalScore);

				Entry->InitResourceScoreEntry(FText::FromName(Info.ResourceID), FText::FromString(Breakdown));
				RewardScrollBox->AddChild(Entry);
			}
		}

		CurrentResourceIndex++;
		return;
	}

	// 모든 출력 완료 → 타이머 종료
	GetWorld()->GetTimerManager().ClearTimer(EntryAddTimerHandle);

	LOG_Frame_WARNING(TEXT("ResultMenu - ResourceScore Entries Displayed: %d"), CachedResourceDetails.Num());

	if (FadeInRankAnim && RankText)
	{
		PlayAnimation(FadeInRankAnim);
		LOG_Frame_WARNING(TEXT("ResultMenu - RankText FadeIn 애니메이션 실행"));
	}
}

void UResultMenu::SetResourceScoreDetails(const TArray<FResourceScoreInfo>& InDetails)
{
	CachedResourceDetails = InDetails;
}

void UResultMenu::SetTotalGold(int32 InTotalGold)
{
	if (TotalGoldText)
	{
		TotalGoldText->SetText(FText::AsNumber(InTotalGold));
	}
}

void UResultMenu::SetRankText(const FString& InRank)
{
	if (RankText)
	{
		RankText->SetText(FText::FromString(InRank));
	}
}

void UResultMenu::OnAcceptClicked()
{
	RemoveFromParent();

	LOG_Frame_WARNING(TEXT("ResultMenu - AcceptClicked → Returning to BaseCamp"));

	// TODO : 호스트가 클릭하면 레벨 전환
	ALCRoomPlayerController* PC = Cast<ALCRoomPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC && PC->GetNetMode() == NM_ListenServer)
	{
		if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			const FName BaseCampMapName = TEXT("BaseCamp");
			const int32 BaseCampID = FCrc::StrCrc32(*BaseCampMapName.ToString());
			GISubsystem->ChangeLevelByMapID(BaseCampID);
		}
	}
}

void UResultMenu::ActivateResultCamera()
{
	// 선택 사항: 카메라 ViewTarget 변경 또는 SceneCapture 등
}