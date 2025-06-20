#include "UI/Popup/PopupLevelInfo.h"
#include "Components/TextBlock.h"

void UPopupLevelInfo::InitLevelInfoWidget(FMapDataRow* MapDataRow)
{
	if (TextMapName)
	{
		TextMapName->SetText(FText::FromName(MapDataRow->MapInfo.MapName));
	}
	if (TextDescription)
	{
		TextDescription->SetText(FText::FromString(MapDataRow->MapInfo.Description));
	}
}

void UPopupLevelInfo::NativeConstruct()
{
	Super::NativeConstruct();

	if (LevelInfoAnim)
	{
		FWidgetAnimationDynamicEvent AnimFinishedDelegate;
		AnimFinishedDelegate.BindDynamic(this, &UPopupLevelInfo::OnInfoAnimationFinished);
		BindToAnimationFinished(LevelInfoAnim, AnimFinishedDelegate);
		PlayAnimation(LevelInfoAnim);
	}
}

void UPopupLevelInfo::NativeDestruct()
{
	Super::NativeDestruct();

}



void UPopupLevelInfo::OnInfoAnimationFinished()
{
	RemoveFromParent();
}
