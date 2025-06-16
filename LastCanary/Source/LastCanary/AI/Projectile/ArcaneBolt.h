#pragma once

#include "CoreMinimal.h"
#include "AI/Projectile/BaseProjectile.h"
#include "ArcaneBolt.generated.h"


UCLASS()
class LASTCANARY_API AArcaneBolt : public ABaseProjectile
{
	GENERATED_BODY()

public:
	AArcaneBolt();

	/** 투사체 초기화 (방향, 속도, 데미지, 인스티게이터 지정) */
	void InitProjectile(const FVector& Direction, float Speed, float Damage, AController* InstigatorController);
	
};
