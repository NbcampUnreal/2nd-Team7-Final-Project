#include "UI/UIElement/TutorialWidget.h"
#include "Components/ScrollBox.h"
#include "Kismet/KismetTextLibrary.h"
#include "Internationalization/Internationalization.h"
#include "Blueprint/UserWidget.h"
#include "UI/UIObject/TutorialRowWidget.h"
#include "UI/UIObject/TutorialSectionHeaderWidget.h"

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 앱 등록(작업표시줄): 이 위젯도 데스크탑 앱으로 보이게
	SetAppInfo(TEXT("Tutorial"), NSLOCTEXT("App", "TutorialName", "Tutorial"), nullptr);

	BuildFromData();
}

void UTutorialWidget::BuildFromData()
{
	if (ScrollBoxRoot == nullptr)
	{
		return;
	}

	ScrollBoxRoot->ClearChildren();

	// 1) 섹션 제목 먼저 추가
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "GoalHeader", "🎯 목표"));
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "ControlsHeader", "🎮 기본 조작"));
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "QuickHeader", "⚡ 진행 순서 (퀵 가이드)"));
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "MonsterHeader", "👹 엘리트 몬스터 특징 (파훼법)"));
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "ItemHeader", "🎒 아이템·소모품"));
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "TeamHeader", "🤝 팀플레이 팁 (멀티플레이)"));
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "SpecHeader", "☠️ 사망 & 관전자"));
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "ResultHeader", "📊 결과·정산"));
	AddSectionHeader(NSLOCTEXT("TLCTutorial", "HelpHeader", "❗ 문제가 생기면"));

	// 2) DataTable 있으면 행 채우기, 없으면 기본 세트로
	TArray<FTutorialRow> Rows;
	if (TutorialData)
	{
		static const FString Ctx(TEXT("TutorialDT"));
		const TArray<FName> Names = TutorialData->GetRowNames();
		for (const FName& N : Names)
		{
			if (const FTutorialRow* R = TutorialData->FindRow<FTutorialRow>(N, Ctx))
			{
				Rows.Add(*R);
			}
		}
	}
	else
	{
		// 기본 세트(구체화 버전 그대로)
		auto Add = [&](FName Section, const TCHAR* Text)
			{
				FTutorialRow R; R.Section = Section; R.Text = FText::FromString(Text);
				Rows.Add(R);
			};

		Add("Goal", TEXT("- 코어를 확보하고 게이트로 탈출하세요."));
		Add("Controls", TEXT("▸ 이동: [WASD]"));
		Add("Controls", TEXT("▸ 카메라: [마우스 이동]"));
		Add("Controls", TEXT("▸ 상호작용: [F]"));
		Add("Controls", TEXT("▸ 손전등: [T]"));
		Add("Controls", TEXT("▸ 달리기: [Shift] / 걷기 해제"));
		Add("Controls", TEXT("▸ 웅크리기: [Ctrl]"));
		Add("Controls", TEXT("▸ 보이스 채팅: [V]"));
		Add("QuickGuide", TEXT("1. 둘러보기: 조작을 익히며 주위를 탐색"));
		Add("QuickGuide", TEXT("2. 코어 확보: 목표 지점에서 [F]로 획득"));
		Add("QuickGuide", TEXT("3. 게이트 탈출: 활성화된 게이트에 상호작용"));
		Add("Monster", TEXT("▸ [추후 업데이트: 몬스터별 특징 아이콘]"));
		Add("Items", TEXT("▸ [F]로 줍기 → 퀵슬롯에서 사용 가능"));
		Add("Items", TEXT("▸ 소모품은 1회성, 아이템은 지속성"));
		Add("Team", TEXT("▸ [V]로 팀원과 보이스 소통"));
		Add("Team", TEXT("▸ 한 명이 어그로 → 다른 팀원 목표 수행"));
		Add("Team", TEXT("▸ 아이템 공유 및 역할 분담 권장"));
		Add("Spectator", TEXT("▸ 사망 시 관전자 모드 전환"));
		Add("Spectator", TEXT("▸ 팀원 시점 확인 및 힌트 제공 가능"));
		Add("Result", TEXT("▸ 라운드 종료 후 결과 화면 표시"));
		Add("Result", TEXT("▸ 생존, 증거 기록, 클리어 시간 = 보상 반영"));
		Add("Help", TEXT("▸ [추후 업데이트: FAQ / 힌트창 연결]"));
	}

	// 3) 섹션 순서 보장하여 추가
	const TArray<FName> Order = { "Goal","Controls","QuickGuide","Monster","Items","Team","Spectator","Result","Help" };

	for (const FName& Sec : Order)
	{
		// 섹션 헤더 다음에 해당 섹션 행을 붙인다
		for (const FTutorialRow& R : Rows)
		{
			if (R.Section == Sec)
			{
				AddRow(R);
			}
		}
	}
}

void UTutorialWidget::AddSectionHeader(const FText& Title)
{
	if (!SectionHeaderClass || !ScrollBoxRoot) return;

	if (auto* Header = CreateWidget<UTutorialSectionHeaderWidget>(GetWorld(), SectionHeaderClass))
	{
		Header->SetTitle(Title);
		ScrollBoxRoot->AddChild(Header);
	}
}

void UTutorialWidget::AddRow(const FTutorialRow& Row)
{
	if (!RowWidgetClass || !ScrollBoxRoot) return;

	if (UTutorialRowWidget* W = CreateWidget<UTutorialRowWidget>(GetWorld(), RowWidgetClass))
	{
		// 아이콘 로드
		UTexture2D* Icon = Row.Icon.LoadSynchronous();
		FText FinalText = ResolveText(Row);

		W->Setup(Icon, FinalText); // Row 위젯에 함수 하나 만들어서 세팅

		ScrollBoxRoot->AddChild(W);
	}
}

FText UTutorialWidget::ResolveText(const FTutorialRow& Row) const
{
	FText Base = Row.Text;

	// StringTable 키가 있으면 우선
	if (!Row.StringTableId.IsNone() && !Row.StringKey.IsNone())
	{
		Base = FText::FromStringTable(Row.StringTableId, FTextKey(Row.StringKey.ToString()));
	}

	// 키치환
	if (Row.KeyTokens.Num() == 0)
	{
		return Base;
	}

	FFormatNamedArguments Args;
	for (const auto& P : Row.KeyTokens)
	{
		const FText KeyLabel = GetKeyDisplayName(P.Value); // FText
		Args.Add(P.Key.ToString(), FFormatArgumentValue(KeyLabel));
	}
	return FText::Format(Base, Args);
}

FText UTutorialWidget::GetKeyDisplayName(const FName& InputActionName) const
{
	// 프로젝트의 인풋 시스템에 맞춰 구현하세요.
	// 예시) Enhanced Input: 액션에서 첫 바인딩 FKey 뽑아 DisplayName 반환
	// 여기선 임시로 액션 이름 그대로 노출
	return FText::FromName(InputActionName);
}
