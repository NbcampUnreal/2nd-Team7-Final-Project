#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationInvokerComponent.h"
#include "InvokerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UInvokerComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = Navigation, meta = (AllowPrivateAccess = "true"))
	UNavigationInvokerComponent* NavInvoker;

public:	
	UInvokerComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nav")
	float NavGenerationradius = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nav")
	float NavRemovalradius = 15.f;


protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE class UNavigationInvokerComponent* GetNavInvoker() const { return NavInvoker; }
		
};
