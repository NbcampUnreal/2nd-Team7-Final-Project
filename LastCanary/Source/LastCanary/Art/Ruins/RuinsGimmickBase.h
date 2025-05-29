#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuinsGimmickBase.generated.h"

UCLASS()
class LASTCANARY_API ARuinsGimmickBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARuinsGimmickBase();

protected:
	virtual void BeginPlay() override;


};
