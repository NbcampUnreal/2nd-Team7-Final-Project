#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/LCGimmickInterface.h"
#include "Interface/InteractableInterface.h"
#include "DataType/GimmickActivationType.h"
#include "Actor/Gimmick/Component/DebuffDamageComponent.h"
#include "LCBaseGimmick.generated.h"

class USoundBase;
class UBoxComponent;


UCLASS(Abstract)
class LASTCANARY_API ALCBaseGimmick : public AActor, public ILCGimmickInterface, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ALCBaseGimmick();

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** ===== 시각 및 사운드 설정 ===== */

	/** 시각적 메쉬 (회전 / 이동 대상) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Visual")
	USceneComponent* VisualMesh;

	/** 상호작용 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Sound")
	USoundBase* InteractSound;

	/** 기믹 작동시 포커스 카메라 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	class ACameraActor* CutsceneCamera;

	/** 카메라까지 가는 시간 (0이면 순간이동) */
	UPROPERTY(EditAnywhere, Category = "Cutscene")
	float CameraBlendTime;

	/** 컷신 실행 시간 */
	UPROPERTY(EditAnywhere, Category = "Cutscene")
	float CutsceneDuration;

	/** 컷신 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutscene")
	bool bEnableCutscene;

	/** 모든 플레이어에게 컷신 보여줄지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutscene")
	bool bCutsceneForAllPlayers; 

	/** 모든 플레이어에게 컷신 시작 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartCutscene();
	void Multicast_StartCutscene_Implementation();

	/** 특정 플레이어에게만 컷신 시작 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartCutsceneForSpecificPlayer(APlayerController* PC);
	void Multicast_StartCutsceneForSpecificPlayer_Implementation(APlayerController* PC);

	/** 컷신 중인지 확인 */
	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	bool IsPlayingCutscene() const;

	/** 컷신 강제 중단 */
	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void StopCutscene();

	/** 블루프린트 이벤트 - 컷신 시작 시 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Cutscene")
	void OnCutsceneStarted(APlayerController* PC);

	/** 블루프린트 이벤트 - 컷신 종료 시 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Cutscene")
	void OnCutsceneEnded(APlayerController* PC);

	/** 상호작용 시 출력되는 메시지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Interaction")
	FString InteractMessage;

	/** 클래스 기반 연결 (블루프린트 클래스에서 연결) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Target")
	TArray<TSubclassOf<AActor>> LinkedTargetClasses;

	/** 인스턴스 기반 연결 (레벨 인스턴스에서 연결) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Target")
	TArray<AActor*> LinkedTargets;

	/** ===== 캐릭터 상호작용 애니메이션 설정 ===== */

	/** 기믹에 상호작용 하는 캐릭터의 애니메이션 제어 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|CharacterAnimation")
	bool bPlayCharacterAnimation = false;

	/** 기믹에 상호작용 하는 캐릭터가 취할 애니메이션 (자신에게만) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|CharacterAnimation")
	UAnimMontage* LocalAnimation;

	/** 기믹에 상호작용 하는 캐릭터가 취할 애니메이션 (타인에게만) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|CharacterAnimation")
	UAnimMontage* RemoteAnimation;

	/** ===== 작동 방식 설정 ===== */

	/** 기믹 작동 방식 (자식 클래스가 고유 조건 처리 시 이 옵션은 비활성화됩니다) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Activation",
		meta = (EditCondition = "bEnableBaseActivationType"))
	EGimmickActivationType ActivationType;

	/** BaseGimmick ActivationType 옵션 활성화 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Activation")
	bool bEnableBaseActivationType;

protected:
	/** ActivateOnStep 타입 처리 */
	void HandleActivateOnStep();

	/** ActivateWhileStepping 타입 처리 */
	void HandleActivateWhileStepping();

	/** ActivateAfterDelay 타입 처리 */
	void HandleActivateAfterDelay();

public:
	/** ==== 오버랩 방식 ==== */

	/** 오버랩 기반 기믹 작동용 트리거 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Trigger")
	UBoxComponent* ActivationTrigger;

	/** 기믹 작동 트리거 영역 진입 */
	UFUNCTION()
	virtual void OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 기믹 작동 트리거 영역 이탈 */
	UFUNCTION()
	virtual void OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 오버랩 감지 캐릭터 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Detection")
	TSet<AActor*> OverlappingActors;

	/** 오버랩으로 기믹을 작동시킬 수 있는 액터 태그들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Detection")
	TArray<FName> ValidActivatorTags;

	/** 유효한 액터인지 확인 (Player, Actor 태그 기반) */
	UFUNCTION(BlueprintCallable, Category = "Gimmick|Detection")
	bool IsValidActivator(AActor* OtherActor) const;

	/**ActivateAfterDelay 전용 타이머 */
	FTimerHandle ActivationDelayHandle;

	/** 활성화까지 요구되는 오버랩 캐릭터 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Activation")
	int32 RequiredCount;

	/** ActivateAfterDelay 사용 시 지연 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Activation")
	float ActivationDelay;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|State", meta = (EditCondition = "!bToggleState"))
	float ReturnDelay;

	/** 복귀 호출여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Interaction")
	bool bCallReturnToInitialStateInsteadOfActivate;

	/** 지정한 지연 시간 후 ReturnToInitialState 호출 */
	//void ScheduleReturn(float Delay);

	/** 상태 복귀 타이머 */
	FTimerHandle ReturnTimerHandle;

private:
	/** 컷신 재생 중인지 여부 */
	bool bIsPlayingCutscene;

	/** 플레이어 입력이 비활성화되었는지 여부 */
	bool bPlayerInputDisabled;

	/** 원래 뷰 타겟 (컷신 종료 시 복원용) */
	AActor* OriginalViewTarget;

	/** 컷신 타이머 */
	FTimerHandle CutsceneTimer;

	/** 개별 플레이어용 컷신 시작 함수 */
	void StartCutsceneForPlayer(APlayerController* PC);

	/** 개별 플레이어용 컷신 종료 함수 */
	void EndCutsceneForPlayer(APlayerController* PC);

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

	/** 즉시 복귀 활성화 */
	virtual void ReturnToInitialState_Implementation() override;

	/** ===== 함정 파괴 관련 ===== */

	/** 총기에 의해 파괴될 수 있는지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Damage")
	bool bDestructibleByGun;

	/** 총기에 맞았을 때 파괴까지 필요한 체력 (1발 = 1 데미지) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Damage", meta = (EditCondition = "bDestructibleByGun"))
	float DestructibleHealth;

	/** 현재 남은 체력 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Damage")
	float CurrentHealth;

	/** 총기 피격 처리 */
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** 총기에 의해 파괴되었을 때 호출 (이펙트 확장용) */
	UFUNCTION(BlueprintNativeEvent, Category = "Gimmick|Damage")
	void OnDestroyedByBullet();
	virtual void OnDestroyedByBullet_Implementation();

	/** 파괴 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Sound")
	USoundBase* DestroySound;

	/** ===== 네트워크 함수 ===== */

	/** 기믹 활성화 요청 (서버 전용) */
	UFUNCTION(Server, Reliable)
	void Server_ActivateGimmick();
	void Server_ActivateGimmick_Implementation();

	/** 상호작용 사운드 재생 (멀티캐스트) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound();
	void Multicast_PlaySound_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDestroySound();
	void Multicast_PlayDestroySound_Implementation();
};
