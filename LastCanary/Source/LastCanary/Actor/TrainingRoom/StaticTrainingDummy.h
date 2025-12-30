#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "DataType/TrainingType.h"
#include "StaticTrainingDummy.generated.h"

class UDamageReceiverComponent;
class UTrainingTargetComponent;

USTRUCT(BlueprintType)
struct FScoreZoneMultiplier
{
    GENERATED_BODY()

    /** 구간 반지름 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    float Radius = 10.0f;

    /** 데미지 배율 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DamageMultiplier = 1.0f;

    /** 디버그 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    FLinearColor ZoneColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FTargetScoreMarker
{
    GENERATED_BODY()

    /** 마커 이름 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    FString MarkerLabel = TEXT("Center");

    /** 마커 위치 (TargetMesh 로컬 좌표) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    FVector LocalPosition = FVector::ZeroVector;

    /** 점수 구간들 (안쪽부터 바깥쪽 순서) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    TArray<FScoreZoneMultiplier> ScoreZones;

    /** 마커 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    FLinearColor MarkerColor = FLinearColor::Red;

    /** 이 마커 활성화 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    bool bEnabled = true;
};

UCLASS()
class LASTCANARY_API AStaticTrainingDummy : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:	
	AStaticTrainingDummy();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TargetMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDamageReceiverComponent* DamageReceiverComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTrainingTargetComponent* TrainingTargetComponent;


    //-----------------------------------------------------
    // 점수 시스템
    //-----------------------------------------------------

    /** 점수 마커 배열 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Scoring")
    TArray<FTargetScoreMarker> ScoreMarkers;

    /** 기본 데미지 배율 (모든 마커 범위 밖일 때) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Scoring")
    float DefaultDamageMultiplier = 1.0f;

    /** 마커 범위 밖 히트는 점수를 주지 않음 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Scoring")
    bool bIgnoreOutOfRangeHits = false;

    /** 디버그 시각화 활성화 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Scoring")
    bool bShowDebugZones = true;

    /** 히트 위치 기반으로 최종 점수 계산 */
    UFUNCTION(BlueprintCallable, Category = "Target Scoring")
    float CalculateFinalScore(const FVector& HitLocation, float BaseDamage, FString& OutMarkerLabel, FLinearColor& OutZoneColor);

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

    /** 데미지 받을 때 호출 */
    UFUNCTION()
    void OnDamageReceived(AActor* DamagedActor, float DamageAmount, const FHitResult& HitInfo, AActor* DamageCauser);

    /** 특정 마커의 월드 좌표 반환 */
    FVector GetMarkerWorldPosition(const FTargetScoreMarker& Marker) const;

    /** 특정 마커에서 데미지 배율 계산 */
    bool CalculateMarkerMultiplier(const FTargetScoreMarker& Marker, const FVector& HitLocation, float& OutMultiplier, FLinearColor& OutZoneColor) const;

    /** 디버그 구간 그리기 */
    void DrawDebugZones();

    /** 마커 초기화 및 정렬 */
    void InitializeMarkers();

private:
    FGameplayTagContainer GameplayTags;
};
