#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCRotationGimmick.h"
#include "LCRotationHandle.generated.h"

UCLASS()
class LASTCANARY_API ALCRotationHandle : public ALCRotationGimmick
{
	GENERATED_BODY()

public:
	ALCRotationHandle();

protected:
	virtual void StartRotation() override;
};
