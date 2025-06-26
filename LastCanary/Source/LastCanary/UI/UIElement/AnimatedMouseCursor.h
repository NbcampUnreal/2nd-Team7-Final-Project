#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "AnimatedMouseCursor.generated.h"

UCLASS()
class LASTCANARY_API UAnimatedMouseCursor : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
	UAnimatedMouseCursor(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 이미지 변경 처리 */
	void SwitchToNextFrame();

protected:
	/** 커서 프레임 이미지들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animated Cursor")
	TArray<UTexture2D*> CursorFrames;

	/** 프레임 간 간격 (초 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animated Cursor", meta = (ClampMin = 0.01))
	float FrameInterval = 0.1f;

	/** 바인딩된 이미지 위젯 */
	UPROPERTY(meta = (BindWidget))
	class UImage* CursorImage;

private:
	int32 CurrentFrameIndex = 0;
	FTimerHandle FrameSwitchTimerHandle;
};
