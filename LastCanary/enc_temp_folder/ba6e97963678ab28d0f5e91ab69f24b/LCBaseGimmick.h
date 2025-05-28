#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/LCGimmickInterface.h"
#include "LCBaseGimmick.generated.h"

UCLASS(Abstract)
class LASTCANARY_API ALCBaseGimmick : public AActor, public ILCGimmickInterface
{
	GENERATED_BODY()
	
public:
    ALCBaseGimmick();

protected:
    /** 기믹이 다시 되돌릴 수 있는지 여부 */
    UPROPERTY(EditAnywhere, Category = "Gimmick")
    bool bRevertible = true;

    /** 연결된 효과 액터들 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick")
    TArray<AActor*> LinkedTargets;

    /** 이미 발동되었는지 여부 */
    UPROPERTY(VisibleInstanceOnly, Category = "Gimmick")
    bool bActivated = false;

    /** 서버에서만 실행되도록 보호된 RPC */
    UFUNCTION(Server, Reliable)
    void Server_ActivateGimmick();
    void Server_ActivateGimmick_Implementation();
    
public:
    virtual void ActivateGimmick_Implementation() override;
    virtual bool CanActivate_Implementation() override;
	virtual void DeactivateGimmick_Implementation() override;
};
