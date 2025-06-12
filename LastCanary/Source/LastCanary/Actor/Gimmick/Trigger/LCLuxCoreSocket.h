#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCTransformGimmick.h"
#include "LCLuxCoreSocket.generated.h"

class ALCRotationLuxStatue;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCLuxCoreSocket : public ALCTransformGimmick
{
	GENERATED_BODY()

public:
	ALCLuxCoreSocket();

protected:
	virtual void BeginPlay() override;

	/** 최초 장착 여부 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	bool bIsCoreInstalled;

public:
	/** 연결된 회전형 Lux 동상 */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Lux")
	ALCRotationLuxStatue* TargetStatue;

	/** 상호작용 처리 */
	virtual void Interact_Implementation(APlayerController* Interactor) override;

	/** 서버: 코어 장착 처리 및 위치 상승 */
	UFUNCTION(Server, Reliable)
	void Server_InstallCore();
	void Server_InstallCore_Implementation();

	/** 클라이언트: 코어 장착 시 비주얼 연출 (움직임 등) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayInstallEffect();
	void Multicast_PlayInstallEffect_Implementation();

	/** 코어 장착 처리 (서버에서만 호출됨) */
	void InstallCore();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

