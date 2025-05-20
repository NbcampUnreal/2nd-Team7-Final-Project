#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_NearbyPlayerDensity.generated.h"

UCLASS()
class LASTCANARY_API UEnvQueryTest_NearbyPlayerDensity : public UEnvQueryTest
{
	GENERATED_BODY()

public:
    UEnvQueryTest_NearbyPlayerDensity();

protected:
    // Configurable radius (디폴트 600)
    UPROPERTY(EditDefaultsOnly, Category = "Test")
    float Radius;

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
};
