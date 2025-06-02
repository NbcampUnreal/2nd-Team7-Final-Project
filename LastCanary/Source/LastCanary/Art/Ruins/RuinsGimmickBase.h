#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "Sound/SoundCue.h"
#include "RuinsGimmickBase.generated.h"


UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ARuinsGimmickBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	ARuinsGimmickBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GimmickMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Sound")
	TObjectPtr<USoundCue> GimmickSound;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound();
	virtual void Multicast_PlaySound_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Interaction")
	FString InteractionMessage;

public:
	/** 기믹 활성화 함수 (블루프린트에서 오버라이드 가능) */
	UFUNCTION(BlueprintNativeEvent, Category = "Gimmick")
	void ActivateGimmick();
	virtual void ActivateGimmick_Implementation();

	/** 상호작용 메시지 반환 (인터페이스 구현) */
	virtual FString GetInteractMessage_Implementation() const override;

	/** 상호작용 처리 (기본은 서버에 Activate 요청) */
	virtual void Interact_Implementation(APlayerController* Interactor) override;

	// 변수 복제 등록 함수 (Replication용)
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};