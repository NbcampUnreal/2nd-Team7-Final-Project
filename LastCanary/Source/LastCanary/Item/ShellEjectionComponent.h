#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShellEjectionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UShellEjectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UShellEjectionComponent();

    /** 탄피 배출 실행 */
    UFUNCTION(BlueprintCallable, Category = "Shell Ejection")
    void EjectShell();

    /** 탄피 이펙트 시스템 설정 */
    UFUNCTION(BlueprintCallable, Category = "Shell Ejection")
    void SetShellParticleSystem(UNiagaraSystem* NewSystem);

protected:
    virtual void BeginPlay() override;

    /** 탄피 배출 소켓 이름 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Socket")
    FName ShellEjectionSocketName = TEXT("ShellEjection");

    /** 대체 소켓 이름들 (우선순위 순) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Socket")
    TArray<FName> FallbackSocketNames = { TEXT("shell_eject"), TEXT("ejection_port"), TEXT("Muzzle") };

    /** 탄피 파티클 시스템 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Effects")
    UNiagaraSystem* ShellParticleSystem;

    /** 탄피 배출 사운드(필요여부 불확실) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Effects")
    USoundBase* ShellEjectionSound;

    /** 배출 방향 오프셋 (소켓 기준 로컬 좌표) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Physics")
    FVector EjectionDirectionOffset = FVector(0, 1, 0.3);

    /** 배출 힘의 세기 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Physics")
    float EjectionForce = 250.0f;

    /** 소켓이 없을 때 사용할 기본 오프셋 위치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Fallback")
    FVector DefaultEjectionOffset = FVector(0, 20, 10);

    /** 최대 효과 거리 (성능 최적화를 위해) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Optimization")
    float MaxEffectDistance = 1000.0f;

    /** 사운드 볼륨 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shell|Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ShellSoundVolume = 0.3f;

private:
    /** 성능 최적화를 위한 쿨다운 */
    float LastEjectionTime = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Shell|Optimization")
    float MinEjectionInterval = 0.05f;

    /** 캐시된 스켈레탈 메시 컴포넌트 */
    UPROPERTY()
    USkeletalMeshComponent* CachedSkeletalMesh;

    /** 유효한 소켓 이름 (캐시됨) */
    FName ValidSocketName;

    /** 소켓 유효성 확인 및 캐싱 */
    void CacheSkeletalMeshAndSocket();

    /** 탄피 배출 가능 여부 확인 */
    bool ShouldEjectShell() const;

    /** 탄피 배출 위치와 회전 계산 */
    bool GetEjectionTransform(FVector& OutLocation, FRotator& OutRotation) const;

    /** 플레이어와의 거리 확인 */
    bool IsWithinEffectDistance() const;

    /** 실제 파티클 이펙트 스폰 */
    void SpawnShellParticleEffect(const FVector& Location, const FRotator& Rotation);

    /** 탄피 사운드 재생 */
    void PlayShellEjectionSound(const FVector& Location);
};
