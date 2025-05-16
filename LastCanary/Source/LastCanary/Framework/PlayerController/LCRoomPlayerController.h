#pragma once

#include "CoreMinimal.h"
#include "Character/BasePlayerController.h"
#include "LCRoomPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCRoomPlayerController : public ABasePlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
};
