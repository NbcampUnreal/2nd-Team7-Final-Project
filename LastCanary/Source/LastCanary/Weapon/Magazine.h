#pragma once

#include "CoreMinimal.h"
#include "Weapon/Parts.h"
#include "Magazine.generated.h"

class AFirearm;

UCLASS()
class LASTCANARY_API AMagazine : public AParts
{
	GENERATED_BODY()
	
public:
	AMagazine();

	UFUNCTION(BlueprintPure, Category = "Magazine")
	virtual int32 GetMagazineCapacity() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	int32 MagazineCapacity;
};
