#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossSpawner.generated.h"

// 전방 선언
class ABaseBossMonsterCharacter;

UCLASS()
class LASTCANARY_API ABossSpawner : public AActor
{
	GENERATED_BODY()
	
public:
    ABossSpawner();

protected:
    virtual void BeginPlay() override;

    /** 에디터에서 지정할 보스 클래스 목록 */
    UPROPERTY(EditAnywhere, Category = "Spawner")
    TArray<TSubclassOf<ABaseBossMonsterCharacter>> BossTypes;

    /** 스폰 위치를 지정할 박스 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "Spawner")
    class UBoxComponent* SpawnArea;

    /** 실제 스폰 처리 */
    void SpawnRandomBoss();

};
