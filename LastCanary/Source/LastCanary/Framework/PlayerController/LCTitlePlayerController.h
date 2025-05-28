#pragma once

#include "CoreMinimal.h"
#include "LCPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "LCTitlePlayerController.generated.h"

UCLASS()
class LASTCANARY_API ALCTitlePlayerController : public ALCPlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
};
