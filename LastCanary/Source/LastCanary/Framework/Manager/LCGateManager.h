#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LCGateManager.generated.h"

class ALCGateActor;
UCLASS()
class LASTCANARY_API ALCGateManager : public AActor
{
	GENERATED_BODY()
	
public:
    ALCGateManager();

protected:
    virtual void BeginPlay() override;

    /** 일정 주기로 게이트 생성 조건 확인 */
    void CheckGateSpawnCondition();

    /** 게이트 생성 실행 (서버 전용) */
    void TrySpawnGate();

public:
    /** 게이트 스폰에 필요한 날짜 */
    UPROPERTY(EditAnywhere, Category = "Gate|Spawn")
    int32 RequiredDay = 3;

    /** 게이트 스폰 포인트 (디자이너가 배치한 TargetPoint들 참조) */
    UPROPERTY(EditAnywhere, Category = "Gate|Spawn")
    TArray<AActor*> GateSpawnPoints;

    /** 생성할 게이트 클래스 */
    UPROPERTY(EditAnywhere, Category = "Gate|Spawn")
    TSubclassOf<ALCGateActor> GateClass;

    /** 게이트 체크 주기 (초) */
    UPROPERTY(EditAnywhere, Category = "Gate|Spawn")
    float CheckInterval = 5.0f;

private:
    FTimerHandle GateCheckTimerHandle;

    /** 이미 스폰된 경우 중복 생성 방지 */
    bool bGateSpawned = false;
};
