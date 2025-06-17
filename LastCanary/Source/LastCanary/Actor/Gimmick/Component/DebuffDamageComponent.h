#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameplayTagAssetInterface.h"
#include "DebuffDamageComponent.generated.h"



UENUM(BlueprintType)
enum class EGimmickDamageType : uint8
{
	None            UMETA(DisplayName = "None"),
	InstantDamage   UMETA(DisplayName = "Instant Damage"),
	DamageOverTime  UMETA(DisplayName = "Damage Over Time")
};

UENUM(BlueprintType)
enum class EGimmickDebuffType : uint8
{
	None           UMETA(DisplayName = "None"),
	SlowMovement   UMETA(DisplayName = "Slow Movement")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UDebuffDamageComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UDebuffDamageComponent();

protected:
	virtual void BeginPlay() override;

	/** 효과를 적용할 대상 목록 */
	UPROPERTY()
	TSet<AActor*> AffectedActors;

	/** 데미지 타이머 핸들 (DoT용) */
	UPROPERTY()
	TMap<AActor*, FTimerHandle> DamageTimers;

	/** ===== Damage 관련 설정 ===== */

	/** 데미지 타입 (None / Instant / OverTime) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	EGimmickDamageType DamageType;

	/** 데미지 수치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "DamageType != EGimmickDamageType::None"))
	float DamageValue;

	/** DamageOverTime 시 적용 주기 (초 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "DamageType == EGimmickDamageType::DamageOverTime"))
	float DamageInterval;

	/** 오버랩 종료 후에도 데미지를 딜레이 후 제거할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	bool bDelayRemoveDamage;

	/** 데미지 제거 지연 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "bDelayRemoveDamage"))
	float DamageRemoveDelay;

	/** ===== Debuff 관련 설정 ===== */

	/** 디버프 타입 (None / SlowMovement 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debuff")
	EGimmickDebuffType DebuffType;

	/** 이동속도 감소율 (0.5 = 50%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debuff", meta = (EditCondition = "DebuffType == EGimmickDebuffType::SlowMovement"))
	float DebuffSlowRate;

	/** 오버랩 종료 후에도 디버프를 딜레이 후 제거할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debuff")
	bool bDelayRemoveDebuff;

	/** 디버프 제거 지연 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debuff", meta = (EditCondition = "bDelayRemoveDebuff"))
	float DebuffRemoveDelay;

	/** ===== 내부 처리 함수 ===== */

	/** 데미지 조건 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
	FGameplayTag RequiredDamageTag;

	/** 디버프 조건 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
	FGameplayTag RequiredDebuffTag;

	/** 데미지 및 디버프 적용 */
	void ApplyEffectToActor(AActor* OtherActor);

	/** 데미지 및 디버프 제거 */
	void RemoveEffectFromActor(AActor* OtherActor);

	/** DOT용 데미지 틱 */
	UFUNCTION()
	void ApplyOverTimeDamage(AActor* Target);

	/** 데미지 타이머 제거 */
	UFUNCTION()
	void StopDamageTimer(AActor* Target);

	/** 디버프 해제 */
	UFUNCTION()
	void RemoveDebuff(AActor* Target);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};