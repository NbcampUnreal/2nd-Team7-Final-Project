#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "Engine/DataTable.h"
#include "TutorialWidget.generated.h"

USTRUCT(BlueprintType)
struct FTutorialRow : public FTableRowBase
{
	GENERATED_BODY()

	// 섹션 구분 (정렬/그룹핑용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Section; // "Goal","Controls","QuickGuide","Monster","Items","Team","Spectator","Result","Help"

	// 표시 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// 표시 텍스트(직접 입력) — 또는 아래 StringTableKey 로도 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	// StringTable 사용 시 키 (예: "ST_Tutorial","Controls.Move")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StringTableId;   // ST_Tutorial
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StringKey;       // Controls.Move

	// 키치환(예: {Move},{Look})용 토큰 -> 액션 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FName> KeyTokens; // { "Move":"IA_Move", "Look":"IA_Look" }
};

class UTutorialRowWidget;
class UTutorialSectionHeaderWidget;
UCLASS()
class LASTCANARY_API UTutorialWidget : public UDesktopWindowBaseWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// DataTable을 지정 안 하면 코드의 기본 세트로 채움
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	UDataTable* TutorialData;

	// 섹션별 제목 블록(굵게)
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollBoxRoot;

	// 행 프리팹(UMG) — 아이콘+텍스트 한 줄짜리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TSubclassOf<UTutorialRowWidget> RowWidgetClass;

	// 섹션 제목 프리팹(UMG) — "🎯 목표" 같은 굵은 라벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TSubclassOf<UTutorialSectionHeaderWidget> SectionHeaderClass;

protected:
	// DataTable → 화면 그리기
	void BuildFromData();

	// StringTable + 키치환 → 최종 FText
	FText ResolveText(const FTutorialRow& Row) const;

	// 액션 이름 → FKey 표시명 (Enhanced Input 기준, 프로젝트에 맞게 바꿔 끼우면 됨)
	FText GetKeyDisplayName(const FName& InputActionName) const;

	// 섹션 그리기 편의를 위한 헬퍼
	void AddSectionHeader(const FText& Title);
	void AddRow(const FTutorialRow& Row);
};
