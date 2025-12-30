#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataType/TrainingType.h"
#include "TrainingRoomManager.generated.h"

class ABaseCharacter;
class UTrainingTargetComponent;
class UWeaponStatsComponent;

UENUM(BlueprintType)
enum class ETargetMeshType : uint8
{
    Skeletal    UMETA(DisplayName = "스켈레탈 (캐릭터형)"),
    Static      UMETA(DisplayName = "스태틱 (과녁형)")
};

UCLASS()
class LASTCANARY_API ATrainingRoomManager : public AActor
{
    GENERATED_BODY()

public:
    ATrainingRoomManager();

    //-----------------------------------------------------
    // 훈련 세션 관리
    //-----------------------------------------------------

    /** 현재 훈련 세션이 활성화되어 있는지 */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SessionActive, Category = "Training")
    bool bSessionActive = false;

    /** 훈련 시작 시간 */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Training")
    float SessionStartTime = 0.0f;

    /** 훈련 제한 시간 (0이면 무제한) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    float SessionDuration = 60.0f;

    /** 현재 훈련 모드 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Training")
    ETrainingMode CurrentTrainingMode = ETrainingMode::Practice;

    //-----------------------------------------------------
    // 더미 행동 설정
    //-----------------------------------------------------

    /** 생성할 더미의 기본 타입 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Training|Dummy")
    EDummyType DefaultDummyType = EDummyType::Static;

    /** 더미 이동 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Training|Dummy")
    float DummyMovementSpeed = 100.0f;

    /** 더미 랜덤 이동 반경 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Training|Dummy")
    float DummyRandomRadius = 500.0f;

    //-----------------------------------------------------
    // 더미 관리
    //-----------------------------------------------------

    /** 활성화된 훈련 타겟들 */
    UPROPERTY(BlueprintReadOnly, Category = "Training")
    TArray<AActor*> ActiveTargets;

    /** 더미 스폰 위치들 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    TArray<FTransform> DummySpawnPoints;

    /** 타겟 메시 타입 (스켈레탈/스태틱 선택) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Training|Dummy")
    ETargetMeshType TargetMeshType = ETargetMeshType::Skeletal;

    /** 스켈레탈 더미 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Classes")
    TSubclassOf<class ASkeletalTrainingDummy> SkeletalDummyClass;

    /** 스태틱 더미 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Classes")
    TSubclassOf<class AStaticTrainingDummy> StaticDummyClass;

    /** 최대 더미 개수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxDummies = 20;

    /** 소환될 더미 개수 */
    UPROPERTY(BlueprintReadWrite, Category = "Training")
    int32 CurrentDummyCount = 5;

    /** 스폰 패턴 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Spawn")
    ESpawnPattern SpawnPattern = ESpawnPattern::FreeRandom;

    /** 스폰 영역 중심 위치 (액터 기준 상대 위치) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Spawn")
    FVector SpawnAreaCenter = FVector::ZeroVector;

    /** 스폰 영역 크기 (X: 가로, Y: 세로) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Spawn", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    FVector2D SpawnAreaSize = FVector2D(2000.0f, 1000.0f);

    /** 그리드 행 개수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Spawn", meta = (EditCondition = "SpawnPattern != ESpawnPattern::FreeRandom", ClampMin = "1", ClampMax = "20"))
    int32 GridRows = 5;

    /** 그리드 열 개수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Spawn", meta = (EditCondition = "SpawnPattern == ESpawnPattern::GridRandom", ClampMin = "1", ClampMax = "20"))
    int32 GridColumns = 10;

    /** 특정 거리 (GridByDistance 모드용, 센티미터 단위) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Spawn", meta = (EditCondition = "SpawnPattern == ESpawnPattern::GridByDistance", ClampMin = "0.0", ClampMax = "10000.0"))
    float TargetDistance = 1000.0f;

private:
    /** 스폰 포인트 생성 */
    void GenerateSpawnPoints();

    /** 자유 랜덤 패턴 생성 */
    void GenerateFreeRandomPattern();

    /** 그리드 랜덤 패턴 생성 */
    void GenerateGridRandomPattern();

    /** 거리별 그리드 패턴 생성 */
    void GenerateGridByDistancePattern();

    /** 랜덤 스폰을 위한 랜덤 시드 */
    FRandomStream RandomStream;

    //-----------------------------------------------------
    // 통계 시스템
    //-----------------------------------------------------
public:
    /** 현재 세션 통계 */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stats, Category = "Training")
    FTrainingStats CurrentStats;

    /** WeaponStatsComponent 이벤트 구독 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void SubscribeToWeaponStats(UWeaponStatsComponent* WeaponStatsComp);

    /** WeaponStatsComponent 이벤트 구독 해제 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void UnsubscribeFromWeaponStats(UWeaponStatsComponent* WeaponStatsComp);

private:
    /** 통계 업데이트 이벤트 콜백 */
    UFUNCTION()
    void OnWeaponStatsUpdated();

    //-----------------------------------------------------
    // 델리게이트
    //-----------------------------------------------------
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatsUpdated, const FTrainingStats&, Stats);
    UPROPERTY(BlueprintAssignable, Category = "Training")
    FOnStatsUpdated OnStatsUpdated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStateChanged, bool, bActive);
    UPROPERTY(BlueprintAssignable, Category = "Training")
    FOnSessionStateChanged OnSessionStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionComplete);
    UPROPERTY(BlueprintAssignable, Category = "Training")
    FOnSessionComplete OnSessionComplete;

    //-----------------------------------------------------
    // 공개 함수
    //-----------------------------------------------------

    /** 훈련 세션 시작 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void StartTrainingSession(ABaseCharacter* TrainingPlayer = nullptr);

    UFUNCTION(Server, Reliable)
    void ServerStartTraining(ABaseCharacter* TrainingPlayer);
    void ServerStartTraining_Implementation(ABaseCharacter* TrainingPlayer);

    /** 훈련 세션 종료 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void EndTrainingSession();

    /** 더미 스폰 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void SpawnDummies(int32 Count = 5);

    /** 모든 더미 제거 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void ClearAllDummies();

    /** 모든 더미 리셋 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void ResetAllDummies();

    /** 더미 파괴 이벤트 처리 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void OnDummyDestroyed(AActor* DestroyedDummy, float DamageDealt);

    /** 더미 피격 이벤트 처리 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void OnDummyHit(AActor* HitDummy, float DamageAmount, const FHitResult& HitInfo);

    /** 총기 발사 이벤트 처리 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void OnWeaponFired(bool bHit);

    /** 현재 통계 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Training")
    FTrainingStats GetCurrentStats() const { return CurrentStats; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_SessionActive();

    UFUNCTION()
    void OnRep_Stats();

    /** 통계 업데이트 */
    void UpdateStats();

    /** 세션 시간 체크 */
    void CheckSessionTime();

    /** 스폰 영역 디버그 가시화 */
    void DrawDebugGrid();

    /** 훈련 중인 플레이어 참조 */
    UPROPERTY()
    ABaseCharacter* CurrentTrainingPlayer = nullptr;

    /** 훈련 중인 플레이어의 WeaponStatsComponent */
    UPROPERTY()
    UWeaponStatsComponent* PlayerWeaponStats = nullptr;

    /** 타겟의 TrainingTargetComponent 가져오기 헬퍼 */
    UTrainingTargetComponent* GetTargetComponent(AActor* Target) const;

private:
    /** 세션 타이머 */
    FTimerHandle SessionTimerHandle;
};
