#pragma once

#include "CoreMinimal.h"
#include "LCPlayerController.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "LCTitlePlayerController.generated.h"

UCLASS()
class LASTCANARY_API ALCTitlePlayerController : public ALCRoomPlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
};
