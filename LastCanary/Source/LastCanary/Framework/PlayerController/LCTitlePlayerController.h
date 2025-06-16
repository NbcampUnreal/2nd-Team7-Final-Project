#pragma once

#include "CoreMinimal.h"
#include "LCPlayerController.h"
#include "Framework/PlayerController/LCPlayerInputController.h"
#include "LCTitlePlayerController.generated.h"

UCLASS()
class LASTCANARY_API ALCTitlePlayerController : public ALCPlayerInputController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
};
