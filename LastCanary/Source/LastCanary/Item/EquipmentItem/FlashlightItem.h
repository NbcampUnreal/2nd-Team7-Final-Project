#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Components/SpotLightComponent.h"
#include "FlashlightItem.generated.h"

/**
 * 손전등 장비 아이템 클래스
 * 빛 컴포넌트 제어와 배터리 소모 기능을 구현합니다.
 */
UCLASS()
class LASTCANARY_API AFlashlightItem : public AEquipmentItemBase
{
    GENERATED_BODY()
    
public:
    AFlashlightItem();
    
protected:
    virtual void BeginPlay() override;
    
    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------
public:
    /** 손전등의 빛을 생성하는 스팟 라이트 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight|Components")
    USpotLightComponent* SpotLightComponent;
    
    //-----------------------------------------------------
    // 손전등 상태 및 속성
    //-----------------------------------------------------
    
    /** 손전등 빛이 켜져 있는지 여부 */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Flashlight|State")
    bool bIsLightOn;
    
    /** 손전등 빛의 강도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Properties")
    float LightIntensity;
    
    /** 손전등 빛의 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Properties")
    FLinearColor LightColor;
    
    /** 손전등 빛의 내부 원뿔 각도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Properties")
    float InnerConeAngle;
    
    /** 손전등 빛의 외부 원뿔 각도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Properties")
    float OuterConeAngle;
    
    /** 초당 배터리 소모율 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Battery")
    float BatteryConsumptionRate;
    
    /** 배터리 소모 타이머 핸들 */
    FTimerHandle BatteryTimerHandle;
    
    //-----------------------------------------------------
    // 아이템 기본 기능 (오버라이드)
    //-----------------------------------------------------
    
    /** 아이템 사용 함수 (손전등 켜기/끄기) */
    virtual void UseItem() override;
    
    /** 아이템 장착 상태 설정 */
    virtual void SetEquipped(bool bNewEquipped) override;
    
    //-----------------------------------------------------
    // 손전등 고유 기능
    //-----------------------------------------------------
    
    /** 손전등 빛 상태 토글 */
    UFUNCTION(BlueprintCallable, Category = "Flashlight|Actions")
    void ToggleLight(bool bTurnOn);
    
    /** 배터리 소모 처리 */
    UFUNCTION(BlueprintCallable, Category = "Flashlight|Battery")
    void ConsumeBattery();

    //-----------------------------------------------------
    // 사운드 효과
    //-----------------------------------------------------

    /** 손전등 켤 때 재생할 사운드 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Sound")
    USoundBase* TurnOnSound;

    /** 손전등 끌 때 재생할 사운드 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Sound")
    USoundBase* TurnOffSound;

    /** 손전등 사운드 볼륨 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SoundVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Sound")
    USoundAttenuation* FlashlightSoundAttenuation;

    /** 모든 클라이언트에서 사운드 재생 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayFlashlightSound(bool bTurnOn);
    
    //-----------------------------------------------------
    // 네트워크 함수
    //-----------------------------------------------------
    
    /** 서버에서 손전등 상태 변경 요청 */
    UFUNCTION(Server, Reliable)
    void Server_ToggleLight(bool bNewState);
    
    /** 모든 클라이언트에게 손전등 상태 업데이트 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UpdateLightState(bool bNewState);

    /** 리플리케이션 속성 설정 */
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
