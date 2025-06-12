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
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Mesh")
	TObjectPtr<UStaticMeshComponent> GimmickMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Rotation")
	float RotationStep;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Rotation")
	float RotateSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Gimmick|Rotation")
	bool bIsRotating;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Gimmick|Rotation")
	float TargetYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Sound")
	TObjectPtr<USoundCue> GimmickSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Interaction")
	FString InteractionMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Cooldown")
	float CooldownTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Gimmick|Cooldown")
	float LastActivatedTime;

public:
	// 인터페이스 구현
	virtual void Interact_Implementation(APlayerController* Interactor) override;
	virtual FString GetInteractMessage_Implementation() const override;

	// 서버에서 기믹 실행
	UFUNCTION(Server, Reliable, Category = "Gimmick")
	void ActivateGimmick();
	virtual void ActivateGimmick_Implementation();

	// 클라이언트가 호출하는 서버 RPC
	UFUNCTION(Server, Reliable)
	void Server_Interact(APlayerController* Interactor);
	void Server_Interact_Implementation(APlayerController* Interactor);

	// 사운드 재생
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound();
	virtual void Multicast_PlaySound_Implementation();

	// 회전 실행
	virtual void StartRotation(float Step);
	virtual void FinishRotation();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
