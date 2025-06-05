#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/LCGimmickInterface.h"
#include "Interface/InteractableInterface.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "LCBaseGimmick.generated.h"

UCLASS(Abstract)
class LASTCANARY_API ALCBaseGimmick : public AActor, public ILCGimmickInterface, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ALCBaseGimmick();

public:
	virtual void BeginPlay() override;

	/** ===== 시각 및 사운드 설정 ===== */

	/** 시각적 메쉬 (회전 / 이동 대상) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Visual")
	UStaticMeshComponent* VisualMesh;

	/** 상호작용 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Sound")
	USoundBase* InteractSound;

	/** 상호작용 시 출력되는 메시지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Interaction")
	FString InteractMessage;

	/** 연결된 효과 액터들 (IGimmickEffectInterface 구현체) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Target")
	TArray<AActor*> LinkedTargets;

	/** ==== 감지 영역 ==== */

	// 감지 기능 활성화 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Detection")
	bool bEnableActorDetection;

	/** 감지 영역 컴포넌트  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Detection")
	UBoxComponent* DetectionArea;

	/** 감지된 액터들 (회전/이동 대상) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Detection")
	TArray<AActor*> AttachedActors;

	/** 감지 영역 진입 이벤트 */
	UFUNCTION()
	virtual void OnActorEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 감지 영역 이탈 이벤트 */
	UFUNCTION()
	virtual void OnActorExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** ===== 쿨타임 설정 ===== */

	/** 현재 활성화된 상태 여부 */
	UPROPERTY(VisibleInstanceOnly, Category = "Gimmick|Cooldown")
	bool bActivated;

	/** 마지막으로 활성화된 시점 */
	UPROPERTY(VisibleInstanceOnly, Category = "Gimmick|Cooldown")
	float LastActivatedTime;

	/** 쿨타임 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Cooldown")
	float CooldownTime;

	/** ===== 상태 토글 설정 ===== */

	/** 상태 유지 여부 (true: 유지, false: 일정 시간 후 복귀) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Toggle")
	bool bToggleState;

	/** 상태 복귀까지 대기 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Toggle", meta = (EditCondition = "!bToggleState"))
	float ReturnDelay;

public:
	/** ===== 인터페이스 구현 ===== */

	/** 상호작용 처리 (클라이언트/서버 구분) */
	virtual void Interact_Implementation(APlayerController* Interactor) override;

	/** 상호작용 메시지 반환 */
	virtual FString GetInteractMessage_Implementation() const override;

	/** 기믹 활성화 */
	virtual void ActivateGimmick_Implementation() override;

	/** 기믹 비활성화 */
	virtual void DeactivateGimmick_Implementation() override;

	/** 기믹 사용 가능 여부 판단 */
	virtual bool CanActivate_Implementation() override;

	/** 동작 중 여부 판단 */
	virtual bool IsGimmickBusy_Implementation() override;

	/** ===== 네트워크 함수 ===== */

	/** 기믹 활성화 요청 (서버 전용) */
	UFUNCTION(Server, Reliable)
	void Server_ActivateGimmick();
	void Server_ActivateGimmick_Implementation();

	/** 상호작용 사운드 재생 (멀티캐스트) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound();
	void Multicast_PlaySound_Implementation();

public:
	FORCEINLINE const TArray<AActor*>& GetAttachedActors() const { return AttachedActors; }


};
