#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataTable/MapDataRow.h"
#include "MapSelectWidget.generated.h"

/**
 * 
 */
class UImage;
class UTextBlock;
class UButton;
class UDataTable;
class ALCGateActor;
UCLASS()
class LASTCANARY_API UMapSelectWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadWrite, Category = "Gate")
    ALCGateActor* GateActorInstance;

protected:
    /** 데이터테이블에서 맵 정보 로드 */
    void LoadMapData();

    /** UI 표시 갱신 */
    void UpdateMapDisplay();

    /** 선택된 맵 ID 반환 */
    int32 GetCurrentMapID() const;

    /** 버튼 바인딩 함수 */
    UFUNCTION()
    void OnLeftButtonClicked();

    UFUNCTION()
    void OnRightButtonClicked();

    UFUNCTION()
    void OnConfirmButtonClicked();

protected:
    /** 현재 선택된 인덱스 */
    int32 CurrentIndex = 0;

    /** 전체 맵 목록 */
    TArray<FMapDataRow*> AllMapRows;

    /** MapDataTable (디자이너에서 할당) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (ExposeOnSpawn = true))
    UDataTable* MapDataTable;

    /** 썸네일 이미지 위젯 */
    UPROPERTY(meta = (BindWidget))
    UImage* ThumbnailImage;

    /** 맵 이름 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* MapNameText;

    ///** 맵 난이도 텍스트 */
    //UPROPERTY(meta = (BindWidget))
    //UTextBlock* MapDifficultyText;

    ///** 맵 설명 or 목표 텍스트 */
    //UPROPERTY(meta = (BindWidget))
    //UTextBlock* MapDescriptionText;

    /** 좌우 버튼 + 확인 버튼 */
    UPROPERTY(meta = (BindWidget))
    UButton* LeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* RightButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ConfirmButton;
};
